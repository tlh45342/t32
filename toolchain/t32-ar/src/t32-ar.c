#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T32AR_VERSION "0.0.1"

#define AR_MAGIC "T32AR\0\0\0"
#define AR_MAGIC_SIZE 8
#define AR_VERSION_MAJOR 1
#define AR_VERSION_MINOR 0

#define OBJ_MAGIC "T32OBJ\0\0"
#define OBJ_MAGIC_SIZE 8

#define MAX_MEMBERS 4096
#define MAX_SYMBOLS 65536
#define MAX_NAME 260

typedef struct {
    char *name;
    uint8_t *data;
    uint32_t size;
} member_t;

typedef struct {
    char *name;
    uint32_t member_index;
} symbol_ref_t;

typedef struct {
    member_t *members;
    uint32_t member_count;
    symbol_ref_t *symbols;
    uint32_t symbol_count;
} archive_t;

static uint16_t rd16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static void wr16(FILE *f, uint16_t v) {
    fputc((int)(v & 0xffu), f);
    fputc((int)((v >> 8) & 0xffu), f);
}

static void wr32(FILE *f, uint32_t v) {
    fputc((int)(v & 0xffu), f);
    fputc((int)((v >> 8) & 0xffu), f);
    fputc((int)((v >> 16) & 0xffu), f);
    fputc((int)((v >> 24) & 0xffu), f);
}

static void die(const char *msg) {
    fprintf(stderr, "t32-ar: error: %s\n", msg);
    exit(1);
}

static void die2(const char *a, const char *b) {
    fprintf(stderr, "t32-ar: error: %s%s\n", a, b);
    exit(1);
}

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (!p) die("out of memory");
    memcpy(p, s, n);
    return p;
}

static uint8_t *read_file(const char *path, uint32_t *size_out) {
    FILE *f = fopen(path, "rb");
    long size;
    uint8_t *buf;
    if (!f) die2("cannot open ", path);
    if (fseek(f, 0, SEEK_END) != 0) die2("cannot seek ", path);
    size = ftell(f);
    if (size < 0 || (unsigned long)size > 0xffffffffUL) die2("invalid size for ", path);
    if (fseek(f, 0, SEEK_SET) != 0) die2("cannot seek ", path);
    buf = (uint8_t *)malloc((size_t)size ? (size_t)size : 1u);
    if (!buf) die("out of memory");
    if (size && fread(buf, 1, (size_t)size, f) != (size_t)size) die2("cannot read ", path);
    fclose(f);
    *size_out = (uint32_t)size;
    return buf;
}

static const char *basename_portable(const char *path) {
    const char *a = strrchr(path, '/');
    const char *b = strrchr(path, '\\');
    const char *p = a;
    if (!p || (b && b > p)) p = b;
    return p ? p + 1 : path;
}

static void free_archive(archive_t *ar) {
    uint32_t i;
    for (i = 0; i < ar->member_count; ++i) {
        free(ar->members[i].name);
        free(ar->members[i].data);
    }
    for (i = 0; i < ar->symbol_count; ++i) {
        free(ar->symbols[i].name);
    }
    free(ar->members);
    free(ar->symbols);
    memset(ar, 0, sizeof(*ar));
}

static const char *obj_string(const uint8_t *obj, uint32_t size,
                              uint32_t str_off, uint32_t str_size,
                              uint32_t name_off) {
    uint32_t i;
    if (name_off >= str_size) return NULL;
    if (str_off > size || str_size > size - str_off) return NULL;
    for (i = name_off; i < str_size; ++i) {
        if (obj[str_off + i] == 0) return (const char *)(obj + str_off + name_off);
    }
    return NULL;
}

static void collect_symbols_from_object(archive_t *ar, uint32_t member_index) {
    const uint8_t *obj = ar->members[member_index].data;
    uint32_t size = ar->members[member_index].size;
    uint32_t sym_count, sym_off, str_off, str_size;
    uint32_t i;

    if (size < 48 || memcmp(obj, OBJ_MAGIC, OBJ_MAGIC_SIZE) != 0) {
        die2("archive member is not T32OBJ: ", ar->members[member_index].name);
    }
    if (rd16(obj + 8) != 1) {
        die2("unsupported T32OBJ version in ", ar->members[member_index].name);
    }

    sym_count = rd32(obj + 20);
    sym_off = rd32(obj + 32);
    str_off = rd32(obj + 40);
    str_size = rd32(obj + 44);

    if (sym_off > size || sym_count > (size - sym_off) / 24u) {
        die2("invalid symbol table in ", ar->members[member_index].name);
    }

    for (i = 1; i < sym_count; ++i) {
        const uint8_t *s = obj + sym_off + i * 24u;
        uint32_t name_off = rd32(s + 0);
        uint32_t section_index = rd32(s + 4);
        uint8_t binding = s[16];
        const char *name;

        if (binding != 1 || section_index == 0) continue;

        name = obj_string(obj, size, str_off, str_size, name_off);
        if (!name || !*name) die2("invalid symbol name in ", ar->members[member_index].name);

        ar->symbols = (symbol_ref_t *)realloc(
            ar->symbols, (size_t)(ar->symbol_count + 1) * sizeof(symbol_ref_t));
        if (!ar->symbols) die("out of memory");

        ar->symbols[ar->symbol_count].name = xstrdup(name);
        ar->symbols[ar->symbol_count].member_index = member_index;
        ar->symbol_count++;
        if (ar->symbol_count > MAX_SYMBOLS) die("too many archive symbols");
    }
}

static void rebuild_symbol_index(archive_t *ar) {
    uint32_t i;
    for (i = 0; i < ar->symbol_count; ++i) free(ar->symbols[i].name);
    free(ar->symbols);
    ar->symbols = NULL;
    ar->symbol_count = 0;

    for (i = 0; i < ar->member_count; ++i) collect_symbols_from_object(ar, i);
}

static void load_archive(const char *path, archive_t *ar, int allow_missing) {
    uint32_t size, member_count, symbol_count;
    uint32_t member_table_off, symbol_table_off, strings_off, strings_size, data_off;
    uint8_t *buf;
    uint32_t i;

    memset(ar, 0, sizeof(*ar));
    buf = read_file(path, &size);

    if (size < 40) {
        free(buf);
        if (allow_missing) return;
        die2("invalid archive: ", path);
    }
    if (memcmp(buf, AR_MAGIC, AR_MAGIC_SIZE) != 0) {
        free(buf);
        die2("invalid archive magic: ", path);
    }
    if (rd16(buf + 8) != AR_VERSION_MAJOR) {
        free(buf);
        die2("unsupported archive version: ", path);
    }

    member_count = rd32(buf + 16);
    symbol_count = rd32(buf + 20);
    member_table_off = rd32(buf + 24);
    symbol_table_off = rd32(buf + 28);
    strings_off = rd32(buf + 32);
    strings_size = rd32(buf + 36);
    data_off = rd32(buf + 12);

    if (member_count > MAX_MEMBERS || symbol_count > MAX_SYMBOLS) die("archive count too large");
    if (member_table_off > size || member_count > (size - member_table_off) / 16u)
        die("invalid archive member table");
    if (symbol_table_off > size || symbol_count > (size - symbol_table_off) / 8u)
        die("invalid archive symbol table");
    if (strings_off > size || strings_size > size - strings_off)
        die("invalid archive string table");
    if (data_off > size) die("invalid archive data offset");

    ar->members = (member_t *)calloc(member_count ? member_count : 1u, sizeof(member_t));
    if (!ar->members) die("out of memory");
    ar->member_count = member_count;

    for (i = 0; i < member_count; ++i) {
        const uint8_t *m = buf + member_table_off + i * 16u;
        uint32_t name_off = rd32(m + 0);
        uint32_t off = rd32(m + 4);
        uint32_t msize = rd32(m + 8);
        const char *name = obj_string(buf, size, strings_off, strings_size, name_off);
        if (!name || !*name) die("invalid archive member name");
        if (off > size || msize > size - off) die("invalid archive member bounds");

        ar->members[i].name = xstrdup(name);
        ar->members[i].size = msize;
        ar->members[i].data = (uint8_t *)malloc(msize ? msize : 1u);
        if (!ar->members[i].data) die("out of memory");
        if (msize) memcpy(ar->members[i].data, buf + off, msize);
    }

    free(buf);
    rebuild_symbol_index(ar);
}

static uint32_t add_string(uint8_t **table, uint32_t *size, const char *s) {
    uint32_t off = *size;
    size_t n = strlen(s) + 1;
    *table = (uint8_t *)realloc(*table, (size_t)(*size) + n);
    if (!*table) die("out of memory");
    memcpy(*table + *size, s, n);
    *size += (uint32_t)n;
    return off;
}

static void write_archive(const char *path, archive_t *ar) {
    FILE *f;
    uint8_t *strings = NULL;
    uint32_t strings_size = 0;
    uint32_t *member_name_offs;
    uint32_t *symbol_name_offs;
    uint32_t header_size = 40;
    uint32_t member_table_off = header_size;
    uint32_t symbol_table_off;
    uint32_t strings_off;
    uint32_t data_off;
    uint32_t running;
    uint32_t i;

    rebuild_symbol_index(ar);

    member_name_offs = (uint32_t *)calloc(ar->member_count ? ar->member_count : 1u, sizeof(uint32_t));
    symbol_name_offs = (uint32_t *)calloc(ar->symbol_count ? ar->symbol_count : 1u, sizeof(uint32_t));
    if (!member_name_offs || !symbol_name_offs) die("out of memory");

    add_string(&strings, &strings_size, "");
    for (i = 0; i < ar->member_count; ++i)
        member_name_offs[i] = add_string(&strings, &strings_size, ar->members[i].name);
    for (i = 0; i < ar->symbol_count; ++i)
        symbol_name_offs[i] = add_string(&strings, &strings_size, ar->symbols[i].name);

    symbol_table_off = member_table_off + ar->member_count * 16u;
    strings_off = symbol_table_off + ar->symbol_count * 8u;
    data_off = strings_off + strings_size;
    running = data_off;

    f = fopen(path, "wb");
    if (!f) die2("cannot create ", path);

    fwrite(AR_MAGIC, 1, AR_MAGIC_SIZE, f);
    wr16(f, AR_VERSION_MAJOR);
    wr16(f, AR_VERSION_MINOR);
    wr32(f, data_off);
    wr32(f, ar->member_count);
    wr32(f, ar->symbol_count);
    wr32(f, member_table_off);
    wr32(f, symbol_table_off);
    wr32(f, strings_off);
    wr32(f, strings_size);

    for (i = 0; i < ar->member_count; ++i) {
        wr32(f, member_name_offs[i]);
        wr32(f, running);
        wr32(f, ar->members[i].size);
        wr32(f, 0);
        running += ar->members[i].size;
    }

    for (i = 0; i < ar->symbol_count; ++i) {
        wr32(f, symbol_name_offs[i]);
        wr32(f, ar->symbols[i].member_index);
    }

    fwrite(strings, 1, strings_size, f);
    for (i = 0; i < ar->member_count; ++i) {
        fwrite(ar->members[i].data, 1, ar->members[i].size, f);
    }

    if (fclose(f) != 0) die2("cannot finalize ", path);

    free(strings);
    free(member_name_offs);
    free(symbol_name_offs);
}

static int find_member(const archive_t *ar, const char *name) {
    uint32_t i;
    for (i = 0; i < ar->member_count; ++i) {
        if (strcmp(ar->members[i].name, name) == 0) return (int)i;
    }
    return -1;
}

static void replace_member(archive_t *ar, const char *path) {
    const char *base = basename_portable(path);
    uint32_t size;
    uint8_t *data = read_file(path, &size);
    int idx = find_member(ar, base);

    if (size < 8 || memcmp(data, OBJ_MAGIC, OBJ_MAGIC_SIZE) != 0) {
        free(data);
        die2("member is not a T32OBJ file: ", path);
    }

    if (idx >= 0) {
        free(ar->members[idx].data);
        ar->members[idx].data = data;
        ar->members[idx].size = size;
        return;
    }

    ar->members = (member_t *)realloc(
        ar->members, (size_t)(ar->member_count + 1) * sizeof(member_t));
    if (!ar->members) die("out of memory");

    ar->members[ar->member_count].name = xstrdup(base);
    ar->members[ar->member_count].data = data;
    ar->members[ar->member_count].size = size;
    ar->member_count++;

    if (ar->member_count > MAX_MEMBERS) die("too many archive members");
}

static void delete_member(archive_t *ar, const char *name) {
    int idx = find_member(ar, name);
    uint32_t i;
    if (idx < 0) die2("member not found: ", name);

    free(ar->members[idx].name);
    free(ar->members[idx].data);

    for (i = (uint32_t)idx + 1; i < ar->member_count; ++i)
        ar->members[i - 1] = ar->members[i];
    ar->member_count--;
}

static void extract_member(const member_t *m) {
    FILE *f = fopen(m->name, "wb");
    if (!f) die2("cannot create ", m->name);
    if (m->size && fwrite(m->data, 1, m->size, f) != m->size)
        die2("cannot write ", m->name);
    fclose(f);
}

static void usage(void) {
    puts("Usage:");
    puts("  t32-ar rcs ARCHIVE FILE...");
    puts("  t32-ar t ARCHIVE");
    puts("  t32-ar x ARCHIVE [MEMBER...]");
    puts("  t32-ar d ARCHIVE MEMBER...");
    puts("  t32-ar --version");
    puts("  t32-ar --help");
}

int main(int argc, char **argv) {
    archive_t ar;
    const char *op;
    const char *archive_path;
    int i;

    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        printf("t32-ar %s\n", T32AR_VERSION);
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        usage();
        return 0;
    }
    if (argc < 3) {
        usage();
        return 2;
    }

    op = argv[1];
    archive_path = argv[2];

    if (strcmp(op, "rcs") == 0 || strcmp(op, "r") == 0) {
        FILE *probe = fopen(archive_path, "rb");
        if (probe) {
            fclose(probe);
            load_archive(archive_path, &ar, 0);
        } else {
            memset(&ar, 0, sizeof(ar));
        }

        if (argc < 4) die("no object members supplied");
        for (i = 3; i < argc; ++i) replace_member(&ar, argv[i]);
        write_archive(archive_path, &ar);
        free_archive(&ar);
        return 0;
    }

    load_archive(archive_path, &ar, 0);

    if (strcmp(op, "t") == 0) {
        uint32_t n;
        for (n = 0; n < ar.member_count; ++n) puts(ar.members[n].name);
    } else if (strcmp(op, "x") == 0) {
        if (argc == 3) {
            uint32_t n;
            for (n = 0; n < ar.member_count; ++n) extract_member(&ar.members[n]);
        } else {
            for (i = 3; i < argc; ++i) {
                int idx = find_member(&ar, argv[i]);
                if (idx < 0) die2("member not found: ", argv[i]);
                extract_member(&ar.members[idx]);
            }
        }
    } else if (strcmp(op, "d") == 0) {
        if (argc < 4) die("no members supplied for deletion");
        for (i = 3; i < argc; ++i) delete_member(&ar, argv[i]);
        write_archive(archive_path, &ar);
    } else {
        free_archive(&ar);
        die2("unknown operation: ", op);
    }

    free_archive(&ar);
    return 0;
}

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/version.h"

#define T32OBJ_HEADER_SIZE 48u
#define T32OBJ_SECTION_SIZE 32u
#define T32OBJ_SYMBOL_SIZE 24u
#define T32OBJ_RELOC_SIZE 20u
#define T32OBJ_SECTION_NULL 0u
#define T32OBJ_SECTION_PROGBITS 1u
#define T32OBJ_SECTION_NOBITS 2u
#define T32OBJ_SHF_ALLOC 0x1u
#define T32OBJ_SHF_EXEC 0x2u
#define T32OBJ_SHF_WRITE 0x4u
#define T32OBJ_BIND_LOCAL 0u
#define T32OBJ_BIND_GLOBAL 1u
#define T32OBJ_SHN_UNDEF 0u
#define T32OBJ_SHN_ABS 0xffffffffu
#define R_T32_ABS32 1u
#define R_T32_TARGET32 2u
#define R_T32_ADDR32 3u

typedef struct {
    uint16_t major, minor;
    uint32_t flags, section_count, symbol_count, relocation_count;
    uint32_t section_off, symbol_off, relocation_off, string_off, string_size;
} header_t;

typedef struct {
    uint32_t name_off, type, flags, alignment, size, data_off;
} section_t;

typedef struct {
    uint32_t name_off, section_index, value, size;
    uint8_t binding, type;
} symbol_t;

typedef struct {
    uint32_t section_index, offset, symbol_index, type;
    int32_t addend;
} relocation_t;

typedef struct {
    uint8_t *data;
    size_t size;
    header_t header;
    section_t *sections;
    symbol_t *symbols;
    relocation_t *relocations;
    const char *strings;
} object_t;

typedef struct {
    int show_sections;
    int show_symbols;
    int show_relocations;
    const char *filename;
} options_t;

static uint16_t rd16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int range_ok(size_t size, uint32_t off, uint64_t amount) {
    return (uint64_t)off + amount <= (uint64_t)size;
}

static void usage(FILE *out) {
    fprintf(out,
        "Usage: t32-nm [options] FILE.o\n"
        "\n"
        "Inspect a T32OBJ v1 relocatable object file.\n"
        "\n"
        "Options:\n"
        "  --sections       show section table only\n"
        "  --symbols        show symbol table only\n"
        "  --relocs         show relocation table only\n"
        "  -h, --help       show this help\n"
        "  --version        show version\n");
}

static void die(const char *file, const char *message) {
    if (file)
        fprintf(stderr, "%s: %s: %s\n", T32_NM_PRODUCT, file, message);
    else
        fprintf(stderr, "%s: %s\n", T32_NM_PRODUCT, message);
    exit(1);
}

static options_t parse_options(int argc, char **argv) {
    options_t o = {0, 0, 0, NULL};
    int explicit_view = 0;
    int i;
    for (i = 1; i < argc; ++i) {
        const char *a = argv[i];
        if (!strcmp(a, "-h") || !strcmp(a, "--help")) {
            usage(stdout);
            exit(0);
        } else if (!strcmp(a, "--version")) {
            printf("%s %s\n", T32_NM_PRODUCT, T32_NM_VERSION);
            exit(0);
        } else if (!strcmp(a, "--sections")) {
            o.show_sections = 1; explicit_view = 1;
        } else if (!strcmp(a, "--symbols")) {
            o.show_symbols = 1; explicit_view = 1;
        } else if (!strcmp(a, "--relocs")) {
            o.show_relocations = 1; explicit_view = 1;
        } else if (a[0] == '-') {
            usage(stderr);
            die(NULL, "unknown option");
        } else if (o.filename) {
            usage(stderr);
            die(NULL, "exactly one object file is required");
        } else {
            o.filename = a;
        }
    }
    if (!o.filename) {
        usage(stderr);
        die(NULL, "missing object file");
    }
    if (!explicit_view)
        o.show_sections = o.show_symbols = o.show_relocations = 1;
    return o;
}

static uint8_t *read_file(const char *name, size_t *size_out) {
    FILE *f = fopen(name, "rb");
    long n;
    uint8_t *data;
    if (!f) {
        fprintf(stderr, "%s: %s: %s\n", T32_NM_PRODUCT, name, strerror(errno));
        exit(1);
    }
    if (fseek(f, 0, SEEK_END) != 0 || (n = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f); die(name, "cannot determine file size");
    }
    data = (uint8_t *)malloc(n ? (size_t)n : 1u);
    if (!data) { fclose(f); die(name, "out of memory"); }
    if (n && fread(data, 1, (size_t)n, f) != (size_t)n) {
        free(data); fclose(f); die(name, "cannot read file");
    }
    fclose(f); *size_out = (size_t)n; return data;
}

static const char *string_at(const object_t *o, uint32_t off) {
    const char *s;
    size_t remaining;
    if (off >= o->header.string_size)
        die(NULL, "string-table offset is outside the table");
    s = o->strings + off;
    remaining = o->header.string_size - off;
    if (!memchr(s, '\0', remaining))
        die(NULL, "unterminated string-table entry");
    return s;
}

static void parse_object(object_t *o, const char *filename) {
    const uint8_t *p;
    uint32_t i;
    memset(o, 0, sizeof(*o));
    o->data = read_file(filename, &o->size);
    if (o->size < T32OBJ_HEADER_SIZE) die(filename, "file is smaller than T32OBJ header");
    if (memcmp(o->data, "T32OBJ\0\0", 8) != 0) die(filename, "invalid T32OBJ magic");
    p = o->data + 8;
    o->header.major = rd16(p); o->header.minor = rd16(p + 2);
    o->header.flags = rd32(p + 4);
    o->header.section_count = rd32(p + 8);
    o->header.symbol_count = rd32(p + 12);
    o->header.relocation_count = rd32(p + 16);
    o->header.section_off = rd32(p + 20);
    o->header.symbol_off = rd32(p + 24);
    o->header.relocation_off = rd32(p + 28);
    o->header.string_off = rd32(p + 32);
    o->header.string_size = rd32(p + 36);
    if (o->header.major != 1) die(filename, "unsupported T32OBJ major version");
    if (o->header.flags != 1u) die(filename, "unsupported T32OBJ flags");
    if (!o->header.section_count || !o->header.symbol_count)
        die(filename, "missing null section or null symbol");
    if (!range_ok(o->size, o->header.section_off, (uint64_t)o->header.section_count * T32OBJ_SECTION_SIZE))
        die(filename, "section table extends beyond file");
    if (!range_ok(o->size, o->header.symbol_off, (uint64_t)o->header.symbol_count * T32OBJ_SYMBOL_SIZE))
        die(filename, "symbol table extends beyond file");
    if (!range_ok(o->size, o->header.relocation_off, (uint64_t)o->header.relocation_count * T32OBJ_RELOC_SIZE))
        die(filename, "relocation table extends beyond file");
    if (!range_ok(o->size, o->header.string_off, o->header.string_size))
        die(filename, "string table extends beyond file");
    o->sections = (section_t *)calloc(o->header.section_count, sizeof(section_t));
    o->symbols = (symbol_t *)calloc(o->header.symbol_count, sizeof(symbol_t));
    o->relocations = (relocation_t *)calloc(o->header.relocation_count ? o->header.relocation_count : 1u, sizeof(relocation_t));
    if (!o->sections || !o->symbols || !o->relocations) die(filename, "out of memory");
    o->strings = (const char *)(o->data + o->header.string_off);
    if (!o->header.string_size || o->strings[0] != '\0') die(filename, "invalid string table");

    for (i = 0; i < o->header.section_count; ++i) {
        section_t *s = &o->sections[i];
        p = o->data + o->header.section_off + i * T32OBJ_SECTION_SIZE;
        s->name_off = rd32(p); s->type = rd32(p + 4); s->flags = rd32(p + 8);
        s->alignment = rd32(p + 12); s->size = rd32(p + 16); s->data_off = rd32(p + 20);
        if (rd32(p + 24) || rd32(p + 28)) die(filename, "nonzero reserved section field");
        (void)string_at(o, s->name_off);
        if (i == 0) {
            if (s->name_off || s->type || s->flags || s->alignment || s->size || s->data_off)
                die(filename, "section zero is not null");
            continue;
        }
        if (!s->alignment || (s->alignment & (s->alignment - 1u))) die(filename, "invalid section alignment");
        if (s->type != T32OBJ_SECTION_PROGBITS && s->type != T32OBJ_SECTION_NOBITS)
            die(filename, "unknown section type");
        if (s->type == T32OBJ_SECTION_PROGBITS && !range_ok(o->size, s->data_off, s->size))
            die(filename, "section data extends beyond file");
        if (s->type == T32OBJ_SECTION_NOBITS && s->data_off != 0)
            die(filename, "NOBITS section has file data offset");
    }

    for (i = 0; i < o->header.symbol_count; ++i) {
        symbol_t *s = &o->symbols[i];
        p = o->data + o->header.symbol_off + i * T32OBJ_SYMBOL_SIZE;
        s->name_off = rd32(p); s->section_index = rd32(p + 4); s->value = rd32(p + 8); s->size = rd32(p + 12);
        s->binding = p[16]; s->type = p[17];
        if (rd16(p + 18) || rd32(p + 20)) die(filename, "nonzero reserved symbol field");
        (void)string_at(o, s->name_off);
        if (s->binding > T32OBJ_BIND_GLOBAL) die(filename, "unknown symbol binding");
        if (s->section_index != T32OBJ_SHN_UNDEF && s->section_index != T32OBJ_SHN_ABS && s->section_index >= o->header.section_count)
            die(filename, "symbol section index is invalid");
    }

    for (i = 0; i < o->header.relocation_count; ++i) {
        relocation_t *r = &o->relocations[i];
        p = o->data + o->header.relocation_off + i * T32OBJ_RELOC_SIZE;
        r->section_index = rd32(p); r->offset = rd32(p + 4); r->symbol_index = rd32(p + 8); r->type = rd32(p + 12); r->addend = (int32_t)rd32(p + 16);
        if (!r->section_index || r->section_index >= o->header.section_count) die(filename, "relocation section index is invalid");
        if (r->symbol_index >= o->header.symbol_count) die(filename, "relocation symbol index is invalid");
        if (r->offset > o->sections[r->section_index].size || o->sections[r->section_index].size - r->offset < 4u)
            die(filename, "relocation patch lies outside section");
        if (r->type < R_T32_ABS32 || r->type > R_T32_ADDR32) die(filename, "unknown relocation type");
    }
}

static void free_object(object_t *o) {
    free(o->relocations); free(o->symbols); free(o->sections); free(o->data);
}

static const char *section_type_name(uint32_t type) {
    return type == T32OBJ_SECTION_PROGBITS ? "PROGBITS" : type == T32OBJ_SECTION_NOBITS ? "NOBITS" : "NULL";
}

static void section_flags(char out[4], uint32_t flags) {
    out[0] = (flags & T32OBJ_SHF_ALLOC) ? 'A' : '-';
    out[1] = (flags & T32OBJ_SHF_EXEC) ? 'X' : '-';
    out[2] = (flags & T32OBJ_SHF_WRITE) ? 'W' : '-';
    out[3] = '\0';
}

static char symbol_letter(const object_t *o, const symbol_t *s) {
    char c = '?';
    if (s->section_index == T32OBJ_SHN_UNDEF) return 'U';
    if (s->section_index == T32OBJ_SHN_ABS) c = 'A';
    else if (s->section_index < o->header.section_count) {
        const section_t *sec = &o->sections[s->section_index];
        if (sec->type == T32OBJ_SECTION_NOBITS) c = 'B';
        else if (sec->flags & T32OBJ_SHF_EXEC) c = 'T';
        else if (sec->flags & T32OBJ_SHF_WRITE) c = 'D';
        else c = 'R';
    }
    if (s->binding == T32OBJ_BIND_LOCAL && c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
    return c;
}

static const char *reloc_name(uint32_t type) {
    switch (type) {
    case R_T32_ABS32: return "R_T32_ABS32";
    case R_T32_TARGET32: return "R_T32_TARGET32";
    case R_T32_ADDR32: return "R_T32_ADDR32";
    default: return "R_T32_UNKNOWN";
    }
}

static void print_sections(const object_t *o) {
    uint32_t i;
    printf("Sections\n--------\n");
    printf("Idx Name             Type      Flags Align       Size   FileOff\n");
    for (i = 0; i < o->header.section_count; ++i) {
        const section_t *s = &o->sections[i];
        char flags[4]; section_flags(flags, s->flags);
        printf("%3u %-16s %-9s %-5s %5u 0x%08X 0x%08X\n",
               i, string_at(o, s->name_off), section_type_name(s->type), flags,
               s->alignment, s->size, s->data_off);
    }
}

static void print_symbols(const object_t *o) {
    uint32_t i;
    printf("Symbols\n-------\n");
    for (i = 1; i < o->header.symbol_count; ++i) {
        const symbol_t *s = &o->symbols[i];
        const char *name = string_at(o, s->name_off);
        char letter = symbol_letter(o, s);
        if (letter == 'U') printf("         %c %s\n", letter, name);
        else printf("%08X %c %s\n", s->value, letter, name);
    }
    if (o->header.symbol_count == 1) printf("(none)\n");
}

static void print_relocations(const object_t *o) {
    uint32_t i;
    printf("Relocations\n-----------\n");
    for (i = 0; i < o->header.relocation_count; ++i) {
        const relocation_t *r = &o->relocations[i];
        const char *sec = string_at(o, o->sections[r->section_index].name_off);
        const char *sym = string_at(o, o->symbols[r->symbol_index].name_off);
        printf("%-10s+0x%08X %-16s %-20s addend=%d\n",
               sec, r->offset, reloc_name(r->type), sym, r->addend);
    }
    if (!o->header.relocation_count) printf("(none)\n");
}

int main(int argc, char **argv) {
    options_t options = parse_options(argc, argv);
    object_t object;
    int first = 1;
    parse_object(&object, options.filename);
    printf("T32OBJ %u.%u  %s\n\n", object.header.major, object.header.minor, options.filename);
    if (options.show_sections) { print_sections(&object); first = 0; }
    if (options.show_symbols) { if (!first) putchar('\n'); print_symbols(&object); first = 0; }
    if (options.show_relocations) { if (!first) putchar('\n'); print_relocations(&object); }
    free_object(&object);
    return 0;
}

/*
 * t32-disk 0.0.1
 *
 * Host-side T32D disk image utility.
 *
 * Deliberately small first format:
 *   sector size       512
 *   LBA 0             T32D header
 *   LBA 1..4          32 fixed directory entries, 64 bytes each
 *   LBA 8..           contiguous file data
 *
 * This is not the long-term filesystem. It is the bootstrap media format
 * needed to prove BIOS -> disk -> BOOT.BIN chain loading.
 */

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define T32_STDIN_ISATTY() _isatty(_fileno(stdin))
#else
#include <unistd.h>
#define T32_STDIN_ISATTY() isatty(STDIN_FILENO)
#endif

#define T32_DISK_VERSION "0.0.1"

#define T32D_SECTOR_SIZE       512u
#define T32D_HEADER_LBA        0u
#define T32D_DIRECTORY_LBA     1u
#define T32D_DIRECTORY_ENTRIES 32u
#define T32D_DIR_ENTRY_SIZE    64u
#define T32D_DIRECTORY_SECTORS 4u
#define T32D_DATA_START_LBA    8u

#define T32D_MAGIC0 'T'
#define T32D_MAGIC1 '3'
#define T32D_MAGIC2 '2'
#define T32D_MAGIC3 'D'

#define T32D_VERSION_MAJOR 0u
#define T32D_VERSION_MINOR 1u

#define T32D_FILE_FLAG_BOOT 0x00000001u
#define T32D_NAME_BYTES 32u

#define MAX_LINE 2048
#define MAX_ARGS 32
#define MAX_SCRIPT_DEPTH 8

typedef struct {
    char name[T32D_NAME_BYTES];
    uint32_t start_lba;
    uint32_t byte_length;
    uint32_t flags;
    uint8_t reserved[20];
} t32d_dirent_t;

typedef struct {
    uint32_t sector_size;
    uint32_t total_sectors;
    uint32_t directory_lba;
    uint32_t directory_entries;
    uint32_t directory_entry_size;
    uint32_t data_start_lba;
    char boot_name[T32D_NAME_BYTES];
} t32d_info_t;

static uint32_t load_le32(const uint8_t *p)
{
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void store_le32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

static int seek_lba(FILE *fp, uint32_t lba)
{
    uint64_t offset = (uint64_t)lba * T32D_SECTOR_SIZE;
#if defined(_WIN32)
    return _fseeki64(fp, (__int64)offset, SEEK_SET) == 0;
#else
    if (offset > (uint64_t)LONG_MAX)
        return 0;
    return fseek(fp, (long)offset, SEEK_SET) == 0;
#endif
}

static int file_size(FILE *fp, uint64_t *size_out)
{
#if defined(_WIN32)
    __int64 here = _ftelli64(fp);
    __int64 end;
    if (here < 0 || _fseeki64(fp, 0, SEEK_END) != 0)
        return 0;
    end = _ftelli64(fp);
    if (end < 0 || _fseeki64(fp, here, SEEK_SET) != 0)
        return 0;
    *size_out = (uint64_t)end;
#else
    long here = ftell(fp);
    long end;
    if (here < 0 || fseek(fp, 0, SEEK_END) != 0)
        return 0;
    end = ftell(fp);
    if (end < 0 || fseek(fp, here, SEEK_SET) != 0)
        return 0;
    *size_out = (uint64_t)(unsigned long)end;
#endif
    return 1;
}

static int parse_size(const char *text, uint64_t *bytes_out)
{
    char *end;
    unsigned long long value;
    uint64_t multiplier = 1;

    errno = 0;
    value = strtoull(text, &end, 0);
    if (errno || end == text)
        return 0;

    if (*end) {
        if (end[1] != '\0')
            return 0;
        switch (toupper((unsigned char)*end)) {
        case 'K': multiplier = 1024ull; break;
        case 'M': multiplier = 1024ull * 1024ull; break;
        case 'G': multiplier = 1024ull * 1024ull * 1024ull; break;
        default: return 0;
        }
    }

    if (value == 0 || value > UINT64_MAX / multiplier)
        return 0;

    *bytes_out = (uint64_t)value * multiplier;
    return 1;
}

static int valid_guest_name(const char *name)
{
    size_t i, n = strlen(name);
    if (n == 0 || n >= T32D_NAME_BYTES)
        return 0;
    for (i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-'))
            return 0;
    }
    return 1;
}

static void uppercase_name(char *dst, size_t dst_size, const char *src)
{
    size_t i;
    if (!dst_size)
        return;
    for (i = 0; i + 1 < dst_size && src[i]; ++i)
        dst[i] = (char)toupper((unsigned char)src[i]);
    dst[i] = '\0';
}

static int create_raw_image(const char *path, uint64_t requested_bytes)
{
    FILE *fp;
    uint64_t bytes;
    uint8_t zero = 0;

    bytes = (requested_bytes + T32D_SECTOR_SIZE - 1) /
            T32D_SECTOR_SIZE * T32D_SECTOR_SIZE;

    if (bytes < (uint64_t)(T32D_DATA_START_LBA + 1u) * T32D_SECTOR_SIZE) {
        fprintf(stderr, "error: image is too small for T32D metadata\n");
        return 0;
    }

    fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "error: cannot create %s: %s\n", path, strerror(errno));
        return 0;
    }

#if defined(_WIN32)
    if (_fseeki64(fp, (__int64)(bytes - 1), SEEK_SET) != 0)
#else
    if (bytes - 1 > (uint64_t)LONG_MAX ||
        fseek(fp, (long)(bytes - 1), SEEK_SET) != 0)
#endif
    {
        fprintf(stderr, "error: cannot size %s\n", path);
        fclose(fp);
        return 0;
    }

    if (fwrite(&zero, 1, 1, fp) != 1) {
        fprintf(stderr, "error: cannot finish %s\n", path);
        fclose(fp);
        return 0;
    }

    fclose(fp);
    printf("created %s (%" PRIu64 " bytes, %" PRIu64 " sectors)\n",
           path, bytes, bytes / T32D_SECTOR_SIZE);
    return 1;
}

static int read_header(FILE *fp, t32d_info_t *info)
{
    uint8_t sector[T32D_SECTOR_SIZE];

    if (!seek_lba(fp, T32D_HEADER_LBA) ||
        fread(sector, 1, sizeof(sector), fp) != sizeof(sector))
        return 0;

    if (sector[0] != T32D_MAGIC0 || sector[1] != T32D_MAGIC1 ||
        sector[2] != T32D_MAGIC2 || sector[3] != T32D_MAGIC3)
        return 0;

    if (sector[4] != T32D_VERSION_MAJOR)
        return 0;

    memset(info, 0, sizeof(*info));
    info->sector_size = load_le32(sector + 8);
    info->total_sectors = load_le32(sector + 12);
    info->directory_lba = load_le32(sector + 16);
    info->directory_entries = load_le32(sector + 20);
    info->directory_entry_size = load_le32(sector + 24);
    info->data_start_lba = load_le32(sector + 28);
    memcpy(info->boot_name, sector + 32, T32D_NAME_BYTES);
    info->boot_name[T32D_NAME_BYTES - 1] = '\0';

    if (info->sector_size != T32D_SECTOR_SIZE ||
        info->directory_entry_size != T32D_DIR_ENTRY_SIZE ||
        info->directory_entries != T32D_DIRECTORY_ENTRIES)
        return 0;

    return 1;
}

static int format_image(const char *path)
{
    FILE *fp;
    uint64_t bytes;
    uint64_t sectors64;
    uint8_t header[T32D_SECTOR_SIZE];
    uint8_t zero[T32D_SECTOR_SIZE];
    unsigned i;

    fp = fopen(path, "r+b");
    if (!fp) {
        fprintf(stderr, "error: cannot open %s: %s\n", path, strerror(errno));
        return 0;
    }

    if (!file_size(fp, &bytes) || bytes % T32D_SECTOR_SIZE != 0) {
        fprintf(stderr, "error: image size must be a multiple of 512 bytes\n");
        fclose(fp);
        return 0;
    }

    sectors64 = bytes / T32D_SECTOR_SIZE;
    if (sectors64 <= T32D_DATA_START_LBA || sectors64 > UINT32_MAX) {
        fprintf(stderr, "error: unsupported image size\n");
        fclose(fp);
        return 0;
    }

    memset(header, 0, sizeof(header));
    header[0] = T32D_MAGIC0;
    header[1] = T32D_MAGIC1;
    header[2] = T32D_MAGIC2;
    header[3] = T32D_MAGIC3;
    header[4] = T32D_VERSION_MAJOR;
    header[5] = T32D_VERSION_MINOR;
    store_le32(header + 8, T32D_SECTOR_SIZE);
    store_le32(header + 12, (uint32_t)sectors64);
    store_le32(header + 16, T32D_DIRECTORY_LBA);
    store_le32(header + 20, T32D_DIRECTORY_ENTRIES);
    store_le32(header + 24, T32D_DIR_ENTRY_SIZE);
    store_le32(header + 28, T32D_DATA_START_LBA);
    memcpy(header + 32, "BOOT.BIN", 8);

    if (!seek_lba(fp, 0) || fwrite(header, 1, sizeof(header), fp) != sizeof(header)) {
        fprintf(stderr, "error: cannot write T32D header\n");
        fclose(fp);
        return 0;
    }

    memset(zero, 0, sizeof(zero));
    for (i = 0; i < T32D_DIRECTORY_SECTORS; ++i) {
        if (!seek_lba(fp, T32D_DIRECTORY_LBA + i) ||
            fwrite(zero, 1, sizeof(zero), fp) != sizeof(zero)) {
            fprintf(stderr, "error: cannot clear T32D directory\n");
            fclose(fp);
            return 0;
        }
    }

    fflush(fp);
    fclose(fp);
    printf("formatted %s as T32D v0.1\n", path);
    return 1;
}

static int read_dirent(FILE *fp, unsigned index, t32d_dirent_t *entry)
{
    uint8_t raw[T32D_DIR_ENTRY_SIZE];
    uint64_t byte_offset =
        (uint64_t)T32D_DIRECTORY_LBA * T32D_SECTOR_SIZE +
        (uint64_t)index * T32D_DIR_ENTRY_SIZE;

#if defined(_WIN32)
    if (_fseeki64(fp, (__int64)byte_offset, SEEK_SET) != 0)
#else
    if (byte_offset > (uint64_t)LONG_MAX ||
        fseek(fp, (long)byte_offset, SEEK_SET) != 0)
#endif
        return 0;

    if (fread(raw, 1, sizeof(raw), fp) != sizeof(raw))
        return 0;

    memset(entry, 0, sizeof(*entry));
    memcpy(entry->name, raw, T32D_NAME_BYTES);
    entry->name[T32D_NAME_BYTES - 1] = '\0';
    entry->start_lba = load_le32(raw + 32);
    entry->byte_length = load_le32(raw + 36);
    entry->flags = load_le32(raw + 40);
    return 1;
}

static int write_dirent(FILE *fp, unsigned index, const t32d_dirent_t *entry)
{
    uint8_t raw[T32D_DIR_ENTRY_SIZE];
    uint64_t byte_offset =
        (uint64_t)T32D_DIRECTORY_LBA * T32D_SECTOR_SIZE +
        (uint64_t)index * T32D_DIR_ENTRY_SIZE;

    memset(raw, 0, sizeof(raw));
    memcpy(raw, entry->name, T32D_NAME_BYTES);
    store_le32(raw + 32, entry->start_lba);
    store_le32(raw + 36, entry->byte_length);
    store_le32(raw + 40, entry->flags);

#if defined(_WIN32)
    if (_fseeki64(fp, (__int64)byte_offset, SEEK_SET) != 0)
#else
    if (byte_offset > (uint64_t)LONG_MAX ||
        fseek(fp, (long)byte_offset, SEEK_SET) != 0)
#endif
        return 0;

    return fwrite(raw, 1, sizeof(raw), fp) == sizeof(raw);
}

static int find_entry(FILE *fp, const char *name, t32d_dirent_t *entry_out,
                      unsigned *index_out)
{
    char wanted[T32D_NAME_BYTES];
    unsigned i;
    t32d_dirent_t entry;

    uppercase_name(wanted, sizeof(wanted), name);

    for (i = 0; i < T32D_DIRECTORY_ENTRIES; ++i) {
        if (!read_dirent(fp, i, &entry))
            return 0;
        if (entry.name[0] && strcmp(entry.name, wanted) == 0) {
            if (entry_out) *entry_out = entry;
            if (index_out) *index_out = i;
            return 1;
        }
    }
    return 0;
}

static uint32_t next_free_lba(FILE *fp)
{
    uint32_t next = T32D_DATA_START_LBA;
    unsigned i;
    t32d_dirent_t entry;

    for (i = 0; i < T32D_DIRECTORY_ENTRIES; ++i) {
        uint32_t end;
        if (!read_dirent(fp, i, &entry))
            return 0;
        if (!entry.name[0])
            continue;
        end = entry.start_lba +
              (entry.byte_length + T32D_SECTOR_SIZE - 1) / T32D_SECTOR_SIZE;
        if (end > next)
            next = end;
    }
    return next;
}

static int first_free_dirent(FILE *fp, unsigned *index_out)
{
    unsigned i;
    t32d_dirent_t entry;
    for (i = 0; i < T32D_DIRECTORY_ENTRIES; ++i) {
        if (!read_dirent(fp, i, &entry))
            return 0;
        if (!entry.name[0]) {
            *index_out = i;
            return 1;
        }
    }
    return 0;
}

static int info_image(const char *path)
{
    FILE *fp = fopen(path, "rb");
    t32d_info_t info;
    if (!fp) {
        fprintf(stderr, "error: cannot open %s\n", path);
        return 0;
    }
    if (!read_header(fp, &info)) {
        fprintf(stderr, "error: %s is not a recognized T32D v0.x image\n", path);
        fclose(fp);
        return 0;
    }
    fclose(fp);

    printf("T32D image: %s\n", path);
    printf("  format            T32D v0.1\n");
    printf("  sector size       %" PRIu32 "\n", info.sector_size);
    printf("  sectors           %" PRIu32 "\n", info.total_sectors);
    printf("  directory LBA     %" PRIu32 "\n", info.directory_lba);
    printf("  directory entries %" PRIu32 "\n", info.directory_entries);
    printf("  data start LBA    %" PRIu32 "\n", info.data_start_lba);
    printf("  boot file         %s\n", info.boot_name);
    return 1;
}

static int list_image(const char *path)
{
    FILE *fp = fopen(path, "rb");
    t32d_info_t info;
    unsigned i;
    unsigned count = 0;
    t32d_dirent_t entry;

    if (!fp) {
        fprintf(stderr, "error: cannot open %s\n", path);
        return 0;
    }
    if (!read_header(fp, &info)) {
        fprintf(stderr, "error: %s is not a recognized T32D image\n", path);
        fclose(fp);
        return 0;
    }

    printf("NAME                             START-LBA   BYTES      FLAGS\n");
    for (i = 0; i < T32D_DIRECTORY_ENTRIES; ++i) {
        if (!read_dirent(fp, i, &entry)) {
            fclose(fp);
            return 0;
        }
        if (!entry.name[0])
            continue;
        printf("%-32s %-11" PRIu32 " %-10" PRIu32 " 0x%08" PRIX32 "\n",
               entry.name, entry.start_lba, entry.byte_length, entry.flags);
        count++;
    }

    if (!count)
        printf("(empty)\n");

    fclose(fp);
    return 1;
}

static int put_file(const char *image_path, const char *host_path,
                    const char *guest_name)
{
    FILE *image = NULL;
    FILE *input = NULL;
    t32d_info_t info;
    t32d_dirent_t entry;
    uint64_t host_size64;
    uint32_t start_lba;
    uint32_t sectors_needed;
    unsigned index;
    uint8_t buffer[T32D_SECTOR_SIZE];
    uint64_t remaining;
    char normalized[T32D_NAME_BYTES];

    if (!valid_guest_name(guest_name)) {
        fprintf(stderr, "error: invalid T32D filename: %s\n", guest_name);
        return 0;
    }
    uppercase_name(normalized, sizeof(normalized), guest_name);

    image = fopen(image_path, "r+b");
    if (!image) {
        fprintf(stderr, "error: cannot open image %s\n", image_path);
        return 0;
    }
    if (!read_header(image, &info)) {
        fprintf(stderr, "error: image is not formatted T32D\n");
        fclose(image);
        return 0;
    }
    if (find_entry(image, normalized, NULL, NULL)) {
        fprintf(stderr, "error: %s already exists in image\n", normalized);
        fclose(image);
        return 0;
    }
    if (!first_free_dirent(image, &index)) {
        fprintf(stderr, "error: T32D directory is full\n");
        fclose(image);
        return 0;
    }

    input = fopen(host_path, "rb");
    if (!input) {
        fprintf(stderr, "error: cannot open input %s\n", host_path);
        fclose(image);
        return 0;
    }
    if (!file_size(input, &host_size64) || host_size64 > UINT32_MAX) {
        fprintf(stderr, "error: input file too large\n");
        fclose(input);
        fclose(image);
        return 0;
    }

    start_lba = next_free_lba(image);
    sectors_needed = ((uint32_t)host_size64 + T32D_SECTOR_SIZE - 1) /
                     T32D_SECTOR_SIZE;
    if ((uint64_t)start_lba + sectors_needed > info.total_sectors) {
        fprintf(stderr, "error: not enough free space in image\n");
        fclose(input);
        fclose(image);
        return 0;
    }

    if (!seek_lba(image, start_lba)) {
        fclose(input);
        fclose(image);
        return 0;
    }

    remaining = host_size64;
    while (remaining) {
        size_t want = remaining > sizeof(buffer) ? sizeof(buffer) : (size_t)remaining;
        size_t got;
        memset(buffer, 0, sizeof(buffer));
        got = fread(buffer, 1, want, input);
        if (got != want || fwrite(buffer, 1, sizeof(buffer), image) != sizeof(buffer)) {
            fprintf(stderr, "error: I/O while copying file into image\n");
            fclose(input);
            fclose(image);
            return 0;
        }
        remaining -= got;
    }

    memset(&entry, 0, sizeof(entry));
    snprintf(entry.name, sizeof(entry.name), "%s", normalized);
    entry.start_lba = start_lba;
    entry.byte_length = (uint32_t)host_size64;
    if (strcmp(normalized, "BOOT.BIN") == 0)
        entry.flags |= T32D_FILE_FLAG_BOOT;

    if (!write_dirent(image, index, &entry)) {
        fprintf(stderr, "error: cannot write directory entry\n");
        fclose(input);
        fclose(image);
        return 0;
    }

    fflush(image);
    fclose(input);
    fclose(image);
    printf("put %s -> %s:%s (%" PRIu32 " bytes at LBA %" PRIu32 ")\n",
           host_path, image_path, normalized, entry.byte_length, entry.start_lba);
    return 1;
}

static int get_file(const char *image_path, const char *guest_name,
                    const char *host_path)
{
    FILE *image = fopen(image_path, "rb");
    FILE *output = NULL;
    t32d_info_t info;
    t32d_dirent_t entry;
    uint8_t buffer[T32D_SECTOR_SIZE];
    uint32_t remaining;

    if (!image) {
        fprintf(stderr, "error: cannot open image %s\n", image_path);
        return 0;
    }
    if (!read_header(image, &info)) {
        fprintf(stderr, "error: image is not formatted T32D\n");
        fclose(image);
        return 0;
    }
    if (!find_entry(image, guest_name, &entry, NULL)) {
        fprintf(stderr, "error: %s not found\n", guest_name);
        fclose(image);
        return 0;
    }

    output = fopen(host_path, "wb");
    if (!output) {
        fprintf(stderr, "error: cannot create %s\n", host_path);
        fclose(image);
        return 0;
    }

    if (!seek_lba(image, entry.start_lba)) {
        fclose(output);
        fclose(image);
        return 0;
    }

    remaining = entry.byte_length;
    while (remaining) {
        size_t count = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
        if (fread(buffer, 1, sizeof(buffer), image) != sizeof(buffer) ||
            fwrite(buffer, 1, count, output) != count) {
            fprintf(stderr, "error: I/O while extracting file\n");
            fclose(output);
            fclose(image);
            return 0;
        }
        remaining -= (uint32_t)count;
    }

    fclose(output);
    fclose(image);
    printf("get %s:%s -> %s\n", image_path, guest_name, host_path);
    return 1;
}

static int tokenize(char *line, char **argv, int max_args)
{
    int argc = 0;
    char *p = line;

    while (*p) {
        char *start;

        while (isspace((unsigned char)*p))
            ++p;

        if (!*p || *p == '#')
            break;

        if (argc >= max_args)
            return -1;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            start = p;

            while (*p && *p != quote)
                ++p;

            if (*p != quote) {
                fprintf(stderr, "error: unterminated quoted argument\n");
                return -1;
            }

            *p++ = '\0';
            argv[argc++] = start;
        } else {
            start = p;

            while (*p && !isspace((unsigned char)*p) && *p != '#')
                ++p;

            if (*p == '#') {
                *p = '\0';
                argv[argc++] = start;
                break;
            }

            if (*p)
                *p++ = '\0';

            argv[argc++] = start;
        }
    }

    return argc;
}

static void print_help(void)
{
    puts("t32-disk commands:");
    puts("  create <image> <size>             create raw image (e.g. 16M)");
    puts("  format <image>                    write T32D v0.1 metadata");
    puts("  info <image>                      display T32D header");
    puts("  list <image>                      list files");
    puts("  put <image> <hostfile> <name>     copy host file into image");
    puts("  get <image> <name> <hostfile>     extract file from image");
    puts("  do <script>                       execute commands from script");
    puts("  help                              show this help");
    puts("  quit | exit                       leave interactive mode");
}

static int execute_line(char *line, int depth, int *quit_requested);

static int execute_script(const char *path, int depth)
{
    FILE *fp;
    char line[MAX_LINE];
    unsigned line_no = 0;
    int quit_requested = 0;

    if (depth >= MAX_SCRIPT_DEPTH) {
        fprintf(stderr, "error: script nesting too deep\n");
        return 0;
    }

    fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "error: cannot open script %s\n", path);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        ++line_no;
        if (!execute_line(line, depth + 1, &quit_requested)) {
            fprintf(stderr, "error: %s:%u failed\n", path, line_no);
            fclose(fp);
            return 0;
        }
        if (quit_requested)
            break;
    }

    fclose(fp);
    return 1;
}

static int execute_line(char *line, int depth, int *quit_requested)
{
    char *argv[MAX_ARGS];
    int argc = tokenize(line, argv, MAX_ARGS);
    uint64_t bytes;

    if (argc < 0) {
        fprintf(stderr, "error: too many command arguments\n");
        return 0;
    }
    if (argc == 0)
        return 1;

    if (strcmp(argv[0], "create") == 0) {
        if (argc != 3 || !parse_size(argv[2], &bytes)) {
            fprintf(stderr, "usage: create <image> <size>\n");
            return 0;
        }
        return create_raw_image(argv[1], bytes);
    }
    if (strcmp(argv[0], "format") == 0) {
        if (argc != 2) { fprintf(stderr, "usage: format <image>\n"); return 0; }
        return format_image(argv[1]);
    }
    if (strcmp(argv[0], "info") == 0) {
        if (argc != 2) { fprintf(stderr, "usage: info <image>\n"); return 0; }
        return info_image(argv[1]);
    }
    if (strcmp(argv[0], "list") == 0) {
        if (argc != 2) { fprintf(stderr, "usage: list <image>\n"); return 0; }
        return list_image(argv[1]);
    }
    if (strcmp(argv[0], "put") == 0) {
        if (argc != 4) { fprintf(stderr, "usage: put <image> <hostfile> <name>\n"); return 0; }
        return put_file(argv[1], argv[2], argv[3]);
    }
    if (strcmp(argv[0], "get") == 0) {
        if (argc != 4) { fprintf(stderr, "usage: get <image> <name> <hostfile>\n"); return 0; }
        return get_file(argv[1], argv[2], argv[3]);
    }
    if (strcmp(argv[0], "do") == 0) {
        if (argc != 2) { fprintf(stderr, "usage: do <script>\n"); return 0; }
        return execute_script(argv[1], depth);
    }
    if (strcmp(argv[0], "help") == 0 || strcmp(argv[0], "?") == 0) {
        print_help();
        return 1;
    }
    if (strcmp(argv[0], "quit") == 0 || strcmp(argv[0], "exit") == 0) {
        *quit_requested = 1;
        return 1;
    }

    fprintf(stderr, "error: unknown command: %s\n", argv[0]);
    return 0;
}

int main(int argc, char **argv)
{
    char line[MAX_LINE];
    int interactive = T32_STDIN_ISATTY();
    int quit_requested = 0;
    int overall_ok = 1;

    if (argc == 2 &&
        (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)) {
        printf("t32-disk %s\n", T32_DISK_VERSION);
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "do") == 0)
        return execute_script(argv[2], 0) ? 0 : 1;

    if (argc != 1) {
        fprintf(stderr, "usage: t32-disk [do script]\n");
        return 2;
    }

    if (interactive) {
        printf("t32-disk %s\n", T32_DISK_VERSION);
        printf("type 'help' for commands\n");
    }

    while (!quit_requested) {
        if (interactive) {
            fputs("t32-disk> ", stdout);
            fflush(stdout);
        }
        if (!fgets(line, sizeof(line), stdin))
            break;

        if (!execute_line(line, 0, &quit_requested)) {
            overall_ok = 0;
            /*
             * Redirected stdin is an automation/script path and is fail-fast.
             * Interactive mode reports the error and allows correction.
             */
            if (!interactive)
                break;
        }
    }

    return overall_ok ? 0 : 1;
}

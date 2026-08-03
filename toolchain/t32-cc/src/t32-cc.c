#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef T32CC_VERSION
#define T32CC_VERSION "0.2.0"
#endif

#ifdef _WIN32
#define PATH_SEP '\\'
#define NULL_REDIRECT " >NUL"
#else
#define PATH_SEP '/'
#define NULL_REDIRECT " >/dev/null"
#endif

typedef enum {
    TOK_EOF = 0,
    TOK_IDENTIFIER,
    TOK_INTEGER,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMICOLON,
    TOK_MINUS,
    TOK_PLUS,
    TOK_ASSIGN,
    TOK_INVALID
} token_kind_t;

typedef struct {
    token_kind_t kind;
    const char *start;
    size_t length;
    uint32_t integer_value;
    int line;
    int column;
} token_t;

typedef struct {
    const char *path;
    const char *source;
    size_t length;
    size_t offset;
    int line;
    int column;
} lexer_t;

typedef struct {
    lexer_t lexer;
    token_t current;
    int failed;
} parser_t;

typedef enum { MODE_LINK = 0, MODE_ASSEMBLY, MODE_OBJECT } output_mode_t;

typedef struct {
    const char *input_path;
    const char *output_path;
    output_mode_t mode;
    int verbose;
} options_t;

static void print_usage(FILE *stream, const char *program_name)
{
    fprintf(stream,
        "Usage: %s [options] <input.c>\n\n"
        "T32 C compiler driver - Stage 3\n\n"
        "Supported source form:\n"
        "  int main(void) { return <integer>; }\n"
        "  int main(void) { int <name> = <integer>; return <name>; }\n\n"
        "Options:\n"
        "  -S                 Compile to assembly and stop\n"
        "  -c                 Compile to relocatable object and stop\n"
        "  -o FILE            Write output to FILE\n"
        "  -v, --verbose      Show compiler phases and invoked tools\n"
        "      --version      Show version\n"
        "  -h, --help         Show this help\n",
        program_name);
}

static int make_default_output_path(const char *input, const char *suffix,
                                    char *output, size_t output_size)
{
    const char *slash = strrchr(input, '/');
    const char *backslash = strrchr(input, '\\');
    const char *base = input;
    const char *dot;
    size_t prefix_length;
    size_t suffix_length = strlen(suffix);

    if (slash && (!backslash || slash > backslash)) base = slash + 1;
    else if (backslash) base = backslash + 1;

    dot = strrchr(base, '.');
    if (!dot || dot == base) dot = input + strlen(input);
    prefix_length = (size_t)(dot - input);
    if (prefix_length + suffix_length + 1 > output_size) return 0;
    memcpy(output, input, prefix_length);
    memcpy(output + prefix_length, suffix, suffix_length + 1);
    return 1;
}

static int parse_options(int argc, char **argv, options_t *options,
                         char *default_output, size_t default_output_size)
{
    int index;
    int saw_S = 0;
    int saw_c = 0;
    const char *suffix;

    memset(options, 0, sizeof(*options));
    options->mode = MODE_LINK;

    for (index = 1; index < argc; ++index) {
        const char *argument = argv[index];
        if (strcmp(argument, "-h") == 0 || strcmp(argument, "--help") == 0) {
            print_usage(stdout, argv[0]); exit(EXIT_SUCCESS);
        } else if (strcmp(argument, "--version") == 0) {
            printf("t32-cc %s\n", T32CC_VERSION); exit(EXIT_SUCCESS);
        } else if (strcmp(argument, "-v") == 0 || strcmp(argument, "--verbose") == 0) {
            options->verbose = 1;
        } else if (strcmp(argument, "-S") == 0) {
            saw_S = 1; options->mode = MODE_ASSEMBLY;
        } else if (strcmp(argument, "-c") == 0) {
            saw_c = 1; options->mode = MODE_OBJECT;
        } else if (strcmp(argument, "-o") == 0 || strcmp(argument, "--output") == 0) {
            if (++index >= argc) { fprintf(stderr, "t32-cc: missing argument for %s\n", argument); return 0; }
            options->output_path = argv[index];
        } else if (argument[0] == '-') {
            fprintf(stderr, "t32-cc: unknown option: %s\n", argument); return 0;
        } else if (options->input_path) {
            fprintf(stderr, "t32-cc: too many input files\n"); return 0;
        } else options->input_path = argument;
    }

    if (saw_S && saw_c) { fprintf(stderr, "t32-cc: -S and -c cannot be used together\n"); return 0; }
    if (!options->input_path) { fprintf(stderr, "t32-cc: an input C file is required\n"); return 0; }

    suffix = options->mode == MODE_ASSEMBLY ? ".s" :
             options->mode == MODE_OBJECT ? ".o" : ".bin";
    if (!options->output_path) {
        if (!make_default_output_path(options->input_path, suffix, default_output, default_output_size)) {
            fprintf(stderr, "t32-cc: output path is too long\n"); return 0;
        }
        options->output_path = default_output;
    }
    return 1;
}

static char *read_entire_file(const char *path, size_t *length_out)
{
    FILE *input;
    long file_size;
    size_t bytes_read;
    char *buffer;

    input = fopen(path, "rb");
    if (!input) {
        fprintf(stderr, "t32-cc: cannot open %s: %s\n", path, strerror(errno));
        return NULL;
    }

    if (fseek(input, 0, SEEK_END) != 0) {
        fprintf(stderr, "t32-cc: cannot seek %s\n", path);
        fclose(input);
        return NULL;
    }

    file_size = ftell(input);
    if (file_size < 0) {
        fprintf(stderr, "t32-cc: cannot determine size of %s\n", path);
        fclose(input);
        return NULL;
    }

    if (fseek(input, 0, SEEK_SET) != 0) {
        fprintf(stderr, "t32-cc: cannot rewind %s\n", path);
        fclose(input);
        return NULL;
    }

    buffer = malloc((size_t)file_size + 1);
    if (!buffer) {
        fprintf(stderr, "t32-cc: out of memory reading %s\n", path);
        fclose(input);
        return NULL;
    }

    bytes_read = fread(buffer, 1, (size_t)file_size, input);
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "t32-cc: error reading %s\n", path);
        free(buffer);
        fclose(input);
        return NULL;
    }

    buffer[bytes_read] = '\0';
    fclose(input);

    *length_out = bytes_read;
    return buffer;
}

static int lexer_at_end(const lexer_t *lexer)
{
    return lexer->offset >= lexer->length;
}

static char lexer_peek(const lexer_t *lexer)
{
    if (lexer_at_end(lexer))
        return '\0';
    return lexer->source[lexer->offset];
}

static char lexer_peek_next(const lexer_t *lexer)
{
    if (lexer->offset + 1 >= lexer->length)
        return '\0';
    return lexer->source[lexer->offset + 1];
}

static char lexer_advance(lexer_t *lexer)
{
    char character;

    if (lexer_at_end(lexer))
        return '\0';

    character = lexer->source[lexer->offset++];
    if (character == '\n') {
        ++lexer->line;
        lexer->column = 1;
    } else {
        ++lexer->column;
    }

    return character;
}

static int lexer_skip_layout(lexer_t *lexer)
{
    for (;;) {
        char current = lexer_peek(lexer);
        char next = lexer_peek_next(lexer);

        if (isspace((unsigned char)current)) {
            lexer_advance(lexer);
            continue;
        }

        if (current == '/' && next == '/') {
            while (!lexer_at_end(lexer) && lexer_peek(lexer) != '\n')
                lexer_advance(lexer);
            continue;
        }

        if (current == '/' && next == '*') {
            int start_line = lexer->line;
            int start_column = lexer->column;

            lexer_advance(lexer);
            lexer_advance(lexer);

            while (!lexer_at_end(lexer)) {
                if (lexer_peek(lexer) == '*' && lexer_peek_next(lexer) == '/') {
                    lexer_advance(lexer);
                    lexer_advance(lexer);
                    break;
                }
                lexer_advance(lexer);
            }

            if (lexer_at_end(lexer) &&
                !(lexer->offset >= 2 &&
                  lexer->source[lexer->offset - 2] == '*' &&
                  lexer->source[lexer->offset - 1] == '/')) {
                fprintf(stderr,
                    "%s:%d:%d: error: unterminated block comment\n",
                    lexer->path,
                    start_line,
                    start_column);
                return 0;
            }
            continue;
        }

        return 1;
    }
}

static token_t make_token(
    token_kind_t kind,
    const char *start,
    size_t length,
    int line,
    int column)
{
    token_t token;

    memset(&token, 0, sizeof(token));
    token.kind = kind;
    token.start = start;
    token.length = length;
    token.line = line;
    token.column = column;
    return token;
}

static token_t lexer_next_token(lexer_t *lexer)
{
    const char *start;
    size_t start_offset;
    int start_line;
    int start_column;
    char character;

    if (!lexer_skip_layout(lexer))
        return make_token(TOK_INVALID, NULL, 0, lexer->line, lexer->column);

    if (lexer_at_end(lexer))
        return make_token(TOK_EOF, lexer->source + lexer->offset, 0,
                          lexer->line, lexer->column);

    start_offset = lexer->offset;
    start = lexer->source + start_offset;
    start_line = lexer->line;
    start_column = lexer->column;
    character = lexer_advance(lexer);

    if (isalpha((unsigned char)character) || character == '_') {
        while (isalnum((unsigned char)lexer_peek(lexer)) ||
               lexer_peek(lexer) == '_') {
            lexer_advance(lexer);
        }

        return make_token(
            TOK_IDENTIFIER,
            start,
            lexer->offset - start_offset,
            start_line,
            start_column);
    }

    if (isdigit((unsigned char)character)) {
        token_t token;
        char temporary[64];
        char *end = NULL;
        unsigned long value;
        size_t length;

        if (character == '0' &&
            (lexer_peek(lexer) == 'x' || lexer_peek(lexer) == 'X')) {
            lexer_advance(lexer);
            while (isxdigit((unsigned char)lexer_peek(lexer)))
                lexer_advance(lexer);
        } else {
            while (isdigit((unsigned char)lexer_peek(lexer)))
                lexer_advance(lexer);
        }

        length = lexer->offset - start_offset;
        token = make_token(TOK_INTEGER, start, length, start_line, start_column);

        if (length >= sizeof(temporary)) {
            token.kind = TOK_INVALID;
            return token;
        }

        memcpy(temporary, start, length);
        temporary[length] = '\0';

        errno = 0;
        value = strtoul(temporary, &end, 0);
        if (errno != 0 || !end || *end != '\0' || value > UINT32_MAX) {
            token.kind = TOK_INVALID;
            return token;
        }

        token.integer_value = (uint32_t)value;
        return token;
    }

    switch (character) {
    case '(':
        return make_token(TOK_LPAREN, start, 1, start_line, start_column);
    case ')':
        return make_token(TOK_RPAREN, start, 1, start_line, start_column);
    case '{':
        return make_token(TOK_LBRACE, start, 1, start_line, start_column);
    case '}':
        return make_token(TOK_RBRACE, start, 1, start_line, start_column);
    case ';':
        return make_token(TOK_SEMICOLON, start, 1, start_line, start_column);
    case '-':
        return make_token(TOK_MINUS, start, 1, start_line, start_column);
    case '+':
        return make_token(TOK_PLUS, start, 1, start_line, start_column);
    case '=':
        return make_token(TOK_ASSIGN, start, 1, start_line, start_column);
    default:
        return make_token(TOK_INVALID, start, 1, start_line, start_column);
    }
}

static const char *token_kind_name(token_kind_t kind)
{
    switch (kind) {
    case TOK_EOF: return "end of file";
    case TOK_IDENTIFIER: return "identifier";
    case TOK_INTEGER: return "integer literal";
    case TOK_LPAREN: return "'('";
    case TOK_RPAREN: return "')'";
    case TOK_LBRACE: return "'{'";
    case TOK_RBRACE: return "'}'";
    case TOK_SEMICOLON: return "';'";
    case TOK_MINUS: return "'-'";
    case TOK_PLUS: return "'+'";
    case TOK_ASSIGN: return "'='";
    default: return "invalid token";
    }
}

static int token_is_identifier(const token_t *token, const char *text)
{
    size_t length = strlen(text);

    return token->kind == TOK_IDENTIFIER &&
           token->length == length &&
           memcmp(token->start, text, length) == 0;
}

static void parser_error(parser_t *parser, const char *message)
{
    const token_t *token = &parser->current;

    if (parser->failed)
        return;

    fprintf(stderr,
        "%s:%d:%d: error: %s",
        parser->lexer.path,
        token->line,
        token->column,
        message);

    if (token->kind == TOK_EOF) {
        fprintf(stderr, " at end of file\n");
    } else if (token->start && token->length > 0) {
        fprintf(stderr, " near '%.*s'\n", (int)token->length, token->start);
    } else {
        fputc('\n', stderr);
    }

    parser->failed = 1;
}

static void parser_advance(parser_t *parser)
{
    if (parser->failed)
        return;

    parser->current = lexer_next_token(&parser->lexer);
    if (parser->current.kind == TOK_INVALID)
        parser_error(parser, "invalid token");
}

static int parser_expect_kind(
    parser_t *parser,
    token_kind_t kind,
    const char *description)
{
    char message[160];

    if (parser->current.kind != kind) {
        snprintf(message, sizeof(message),
                 "expected %s, found %s",
                 description,
                 token_kind_name(parser->current.kind));
        parser_error(parser, message);
        return 0;
    }

    parser_advance(parser);
    return !parser->failed;
}

static int parser_expect_word(parser_t *parser, const char *word)
{
    char message[160];

    if (!token_is_identifier(&parser->current, word)) {
        snprintf(message, sizeof(message), "expected '%s'", word);
        parser_error(parser, message);
        return 0;
    }

    parser_advance(parser);
    return !parser->failed;
}

typedef enum {
    RETURN_INTEGER = 0,
    RETURN_LOCAL
} return_kind_t;

typedef struct {
    int has_local;
    char local_name[64];
    uint32_t local_initializer;
    return_kind_t return_kind;
    uint32_t return_value;
} program_t;

static int copy_identifier(const token_t *token, char *buffer, size_t size)
{
    if (token->kind != TOK_IDENTIFIER || token->length + 1 > size)
        return 0;
    memcpy(buffer, token->start, token->length);
    buffer[token->length] = '\0';
    return 1;
}

static int token_matches_name(const token_t *token, const char *name)
{
    size_t length = strlen(name);
    return token->kind == TOK_IDENTIFIER &&
           token->length == length &&
           memcmp(token->start, name, length) == 0;
}

static int parse_integer_literal(parser_t *parser, uint32_t *value_out)
{
    int negative = 0;
    uint32_t magnitude;

    if (parser->current.kind == TOK_MINUS) {
        negative = 1;
        parser_advance(parser);
    }

    if (parser->current.kind != TOK_INTEGER) {
        parser_error(parser, "expected integer literal");
        return 0;
    }

    magnitude = parser->current.integer_value;
    parser_advance(parser);

    if (negative) {
        if (magnitude > UINT32_C(2147483648)) {
            parser_error(parser, "negative integer literal is out of range");
            return 0;
        }
        *value_out = (uint32_t)(0u - magnitude);
    } else {
        *value_out = magnitude;
    }

    return 1;
}

static int parse_program(parser_t *parser, program_t *program)
{
    memset(program, 0, sizeof(*program));
    parser_advance(parser);

    if (!parser_expect_word(parser, "int")) return 0;
    if (!parser_expect_word(parser, "main")) return 0;
    if (!parser_expect_kind(parser, TOK_LPAREN, "'('") ) return 0;

    if (token_is_identifier(&parser->current, "void"))
        parser_advance(parser);

    if (!parser_expect_kind(parser, TOK_RPAREN, "')'")) return 0;
    if (!parser_expect_kind(parser, TOK_LBRACE, "'{'")) return 0;

    if (token_is_identifier(&parser->current, "int")) {
        token_t name_token;

        parser_advance(parser);
        if (parser->current.kind != TOK_IDENTIFIER) {
            parser_error(parser, "expected local variable name");
            return 0;
        }

        name_token = parser->current;
        if (!copy_identifier(&name_token, program->local_name,
                             sizeof(program->local_name))) {
            parser_error(parser, "local variable name is too long");
            return 0;
        }
        program->has_local = 1;
        parser_advance(parser);

        if (!parser_expect_kind(parser, TOK_ASSIGN, "'='")) return 0;
        if (!parse_integer_literal(parser, &program->local_initializer)) return 0;
        if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'")) return 0;

        if (token_is_identifier(&parser->current, "int")) {
            parser_error(parser, "Stage 3 supports exactly one local variable");
            return 0;
        }
    }

    if (!parser_expect_word(parser, "return")) return 0;

    if (parser->current.kind == TOK_INTEGER || parser->current.kind == TOK_MINUS) {
        program->return_kind = RETURN_INTEGER;
        if (!parse_integer_literal(parser, &program->return_value)) return 0;
    } else if (parser->current.kind == TOK_IDENTIFIER) {
        if (!program->has_local ||
            !token_matches_name(&parser->current, program->local_name)) {
            parser_error(parser, "use of undeclared local variable");
            return 0;
        }
        program->return_kind = RETURN_LOCAL;
        parser_advance(parser);
    } else {
        parser_error(parser, "return expression must be an integer literal or local variable");
        return 0;
    }

    if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'")) return 0;
    if (!parser_expect_kind(parser, TOK_RBRACE, "'}'")) return 0;

    if (parser->current.kind != TOK_EOF) {
        parser_error(parser, "Stage 3 supports exactly one function named main");
        return 0;
    }

    return !parser->failed;
}

static int emit_assembly(const char *path, const program_t *program)
{
    FILE *output = fopen(path, "w");
    if (!output) {
        fprintf(stderr, "t32-cc: cannot create %s: %s\n", path, strerror(errno));
        return 0;
    }

    fprintf(output,
        "; generated by t32-cc %s\n"
        "; Stage 3 ABI 0.1 translation unit\n\n"
        ".section .text\n"
        ".global main\n\n"
        "main:\n",
        T32CC_VERSION);

    if (program->has_local) {
        fprintf(output,
            "    ; local int %s occupies 4 bytes at [r15]\n"
            "    subi r15, r15, 4\n"
            "    movi r1, 0x%08X\n"
            "    stw  r1, [r15]\n",
            program->local_name,
            program->local_initializer);
    }

    if (program->return_kind == RETURN_LOCAL) {
        fprintf(output, "    ldw  r0, [r15]\n");
    } else {
        fprintf(output, "    movi r0, 0x%08X\n", program->return_value);
    }

    if (program->has_local)
        fprintf(output, "    addi r15, r15, 4\n");

    fprintf(output, "    ret\n");

    if (fclose(output) != 0) {
        fprintf(stderr, "t32-cc: error closing %s\n", path);
        return 0;
    }
    return 1;
}

static void remove_file_quietly(const char *path) { if (path) remove(path); }

static int run_command(const char *command, int verbose)
{
    char full[4096];
    int rc;
    if (verbose) printf("invoke: %s\n", command);
    if (verbose) snprintf(full, sizeof(full), "%s", command);
    else snprintf(full, sizeof(full), "%s%s", command, NULL_REDIRECT);
    rc = system(full);
    return rc == 0;
}

static int get_prefix(char *buffer, size_t size)
{
    const char *explicit_prefix = getenv("T32_PREFIX");
    const char *home;
    if (explicit_prefix && *explicit_prefix) {
        return snprintf(buffer, size, "%s", explicit_prefix) > 0 && strlen(explicit_prefix) < size;
    }
#ifdef _WIN32
    home = getenv("USERPROFILE");
#else
    home = getenv("HOME");
#endif
    if (!home || !*home) return 0;
    return snprintf(buffer, size, "%s%c.local", home, PATH_SEP) > 0;
}

static void make_temp_path(const char *output, const char *suffix, char *buffer, size_t size)
{
    snprintf(buffer, size, "%s%s", output, suffix);
}

static int assemble_object(const char *assembly, const char *object, int verbose)
{
    char command[8192];
    snprintf(command, sizeof(command), "t32-as -f obj \"%s\" -o \"%s\"", assembly, object);
    if (!run_command(command, verbose)) {
        fprintf(stderr, "t32-cc: assembler failed\n"); remove_file_quietly(object); return 0;
    }
    return 1;
}

static int link_program(const char *object, const char *output, int verbose)
{
    char prefix[1024], crt0[1400], library[1400], command[8192];
    FILE *probe;
    if (!get_prefix(prefix, sizeof(prefix))) {
        fprintf(stderr, "t32-cc: cannot determine T32 installation prefix\n"); return 0;
    }
    snprintf(crt0, sizeof(crt0), "%s%clib%ct32%ccrt0.o", prefix, PATH_SEP, PATH_SEP, PATH_SEP);
    snprintf(library, sizeof(library), "%s%clib%ct32%clibt32.a", prefix, PATH_SEP, PATH_SEP, PATH_SEP);
    probe = fopen(crt0, "rb");
    if (!probe) { fprintf(stderr, "t32-cc: missing startup object: %s\n", crt0); return 0; }
    fclose(probe);
    probe = fopen(library, "rb");
    if (!probe) { fprintf(stderr, "t32-cc: missing runtime library: %s\n", library); return 0; }
    fclose(probe);
    snprintf(command, sizeof(command), "t32-ld \"%s\" \"%s\" \"%s\" -o \"%s\"",
             crt0, object, library, output);
    if (!run_command(command, verbose)) {
        fprintf(stderr, "t32-cc: linker failed\n"); remove_file_quietly(output); return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    options_t options;
    char default_output[1024];
    char temp_assembly[1200];
    char temp_object[1200];
    const char *assembly_path = NULL;
    const char *object_path = NULL;
    char *source;
    size_t source_length;
    parser_t parser;
    program_t program;
    int success = 0;

    if (argc == 1) { print_usage(stderr, argv[0]); return EXIT_FAILURE; }
    if (!parse_options(argc, argv, &options, default_output, sizeof(default_output))) return EXIT_FAILURE;

    source = read_entire_file(options.input_path, &source_length);
    if (!source) return EXIT_FAILURE;
    memset(&parser, 0, sizeof(parser));
    parser.lexer.path = options.input_path;
    parser.lexer.source = source;
    parser.lexer.length = source_length;
    parser.lexer.line = 1;
    parser.lexer.column = 1;
    if (!parse_program(&parser, &program)) { free(source); return EXIT_FAILURE; }

    if (options.mode == MODE_ASSEMBLY) assembly_path = options.output_path;
    else { make_temp_path(options.output_path, ".t32cc.s", temp_assembly, sizeof(temp_assembly)); assembly_path = temp_assembly; }

    if (options.verbose) printf("compile: %s -> %s\n", options.input_path, assembly_path);
    if (!emit_assembly(assembly_path, &program)) goto done;

    if (options.mode == MODE_ASSEMBLY) { success = 1; goto done; }

    if (options.mode == MODE_OBJECT) object_path = options.output_path;
    else { make_temp_path(options.output_path, ".t32cc.o", temp_object, sizeof(temp_object)); object_path = temp_object; }

    if (!assemble_object(assembly_path, object_path, options.verbose)) goto done;
    if (options.mode == MODE_OBJECT) { success = 1; goto done; }
    if (!link_program(object_path, options.output_path, options.verbose)) goto done;
    success = 1;

done:
    if (options.mode != MODE_ASSEMBLY) remove_file_quietly(assembly_path);
    if (options.mode == MODE_LINK) remove_file_quietly(object_path);
    if (!success) remove_file_quietly(options.output_path);
    free(source);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

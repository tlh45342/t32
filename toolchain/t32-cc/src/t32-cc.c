#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef T32CC_VERSION
#define T32CC_VERSION "0.0.3"
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

typedef struct {
    const char *input_path;
    const char *output_path;
    uint32_t origin;
    int verbose;
} options_t;

static void print_usage(FILE *stream, const char *program_name)
{
    fprintf(stream,
        "Usage: %s [options] <input.c>\n"
        "\n"
        "T32 C compiler - Stage 1\n"
        "\n"
        "Supported source form:\n"
        "  int main(void) { return <integer>; }\n"
        "\n"
        "Options:\n"
        "  -o, --output <file>  Write assembly to <file>\n"
        "      --origin <addr>   Set logical code origin (default: 0x8000)\n"
        "  -v, --verbose        Show selected input and output\n"
        "      --version        Show version\n"
        "  -h, --help           Show this help\n",
        program_name);
}

static int make_default_output_path(
    const char *input_path,
    char *output_path,
    size_t output_size)
{
    const char *slash;
    const char *backslash;
    const char *base;
    const char *dot;
    size_t prefix_length;

    slash = strrchr(input_path, '/');
    backslash = strrchr(input_path, '\\');
    base = input_path;

    if (slash && (!backslash || slash > backslash))
        base = slash + 1;
    else if (backslash)
        base = backslash + 1;

    dot = strrchr(base, '.');
    if (!dot || dot == base)
        dot = input_path + strlen(input_path);

    prefix_length = (size_t)(dot - input_path);
    if (prefix_length + 3 > output_size)
        return 0;

    memcpy(output_path, input_path, prefix_length);
    memcpy(output_path + prefix_length, ".s", 3);
    return 1;
}

static int parse_options(
    int argc,
    char **argv,
    options_t *options,
    char *default_output,
    size_t default_output_size)
{
    int index;

    memset(options, 0, sizeof(*options));
    options->origin = UINT32_C(0x00008000);

    for (index = 1; index < argc; ++index) {
        const char *argument = argv[index];

        if (strcmp(argument, "-h") == 0 ||
            strcmp(argument, "--help") == 0) {
            print_usage(stdout, argv[0]);
            exit(EXIT_SUCCESS);
        } else if (strcmp(argument, "--version") == 0) {
            printf("t32-cc %s\n", T32CC_VERSION);
            exit(EXIT_SUCCESS);
        } else if (strcmp(argument, "-v") == 0 ||
                   strcmp(argument, "--verbose") == 0) {
            options->verbose = 1;
        } else if (strcmp(argument, "--origin") == 0) {
            char *end = NULL;
            unsigned long value;

            if (++index >= argc) {
                fprintf(stderr, "t32-cc: missing argument for --origin\n");
                return 0;
            }

            errno = 0;
            value = strtoul(argv[index], &end, 0);
            if (errno != 0 || !end || *end != '\0' || value > UINT32_MAX) {
                fprintf(stderr, "t32-cc: invalid origin: %s\n", argv[index]);
                return 0;
            }
            options->origin = (uint32_t)value;
        } else if (strcmp(argument, "-o") == 0 ||
                   strcmp(argument, "--output") == 0) {
            if (++index >= argc) {
                fprintf(stderr, "t32-cc: missing argument for %s\n", argument);
                return 0;
            }
            options->output_path = argv[index];
        } else if (argument[0] == '-') {
            fprintf(stderr, "t32-cc: unknown option: %s\n", argument);
            return 0;
        } else if (options->input_path) {
            fprintf(stderr, "t32-cc: too many input files\n");
            return 0;
        } else {
            options->input_path = argument;
        }
    }

    if (!options->input_path) {
        fprintf(stderr, "t32-cc: an input C file is required\n");
        return 0;
    }

    if (!options->output_path) {
        if (!make_default_output_path(
                options->input_path,
                default_output,
                default_output_size)) {
            fprintf(stderr, "t32-cc: output path is too long\n");
            return 0;
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

static int parse_return_value(parser_t *parser, uint32_t *value_out)
{
    int negative = 0;
    uint32_t magnitude;

    if (parser->current.kind == TOK_MINUS) {
        negative = 1;
        parser_advance(parser);
    }

    if (parser->current.kind != TOK_INTEGER) {
        parser_error(parser,
            "Stage 1 return expression must be an integer literal");
        return 0;
    }

    magnitude = parser->current.integer_value;
    parser_advance(parser);

    if (parser->current.kind == TOK_PLUS) {
        parser_error(parser,
            "Stage 1 return expression must be a single integer literal");
        return 0;
    }

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

static int parse_program(parser_t *parser, uint32_t *return_value)
{
    parser_advance(parser);

    if (!parser_expect_word(parser, "int"))
        return 0;
    if (!parser_expect_word(parser, "main"))
        return 0;
    if (!parser_expect_kind(parser, TOK_LPAREN, "'('") )
        return 0;

    if (token_is_identifier(&parser->current, "void"))
        parser_advance(parser);

    if (!parser_expect_kind(parser, TOK_RPAREN, "')'"))
        return 0;
    if (!parser_expect_kind(parser, TOK_LBRACE, "'{'"))
        return 0;
    if (!parser_expect_word(parser, "return"))
        return 0;
    if (!parse_return_value(parser, return_value))
        return 0;
    if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'"))
        return 0;
    if (!parser_expect_kind(parser, TOK_RBRACE, "'}'"))
        return 0;

    if (parser->current.kind != TOK_EOF) {
        parser_error(parser,
            "Stage 1 supports exactly one function named main");
        return 0;
    }

    return !parser->failed;
}

static int emit_assembly(
    const char *path,
    uint32_t origin,
    uint32_t return_value)
{
    FILE *output = fopen(path, "w");

    if (!output) {
        fprintf(stderr, "t32-cc: cannot create %s: %s\n",
                path, strerror(errno));
        return 0;
    }

    fprintf(output,
        "; generated by t32-cc %s\n"
        "; Stage 1: int main(void) { return integer; }\n"
        "; logical origin: 0x%08X\n"
        "\n"
        ".org 0x%08X\n"
        "\n"
        "main:\n"
        "    movi r0, 0x%08X\n"
        "    halt\n",
        T32CC_VERSION,
        origin,
        origin,
        return_value);

    if (fclose(output) != 0) {
        fprintf(stderr, "t32-cc: error closing %s\n", path);
        return 0;
    }

    return 1;
}

int main(int argc, char **argv)
{
    options_t options;
    char default_output[1024];
    char *source;
    size_t source_length;
    parser_t parser;
    uint32_t return_value;

    if (argc == 1) {
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    if (!parse_options(
            argc,
            argv,
            &options,
            default_output,
            sizeof(default_output))) {
        return EXIT_FAILURE;
    }

    source = read_entire_file(options.input_path, &source_length);
    if (!source)
        return EXIT_FAILURE;

    memset(&parser, 0, sizeof(parser));
    parser.lexer.path = options.input_path;
    parser.lexer.source = source;
    parser.lexer.length = source_length;
    parser.lexer.line = 1;
    parser.lexer.column = 1;

    if (!parse_program(&parser, &return_value)) {
        free(source);
        return EXIT_FAILURE;
    }

    if (!emit_assembly(options.output_path, options.origin, return_value)) {
        free(source);
        return EXIT_FAILURE;
    }

    if (options.verbose) {
        printf("t32-cc %s\n", T32CC_VERSION);
        printf("input:  %s\n", options.input_path);
        printf("output: %s\n", options.output_path);
        printf("origin: 0x%08X\n", options.origin);
        printf("main returns: 0x%08X (%u)\n",
               return_value,
               return_value);
    } else {
        printf("compiled %s -> %s\n",
               options.input_path,
               options.output_path);
    }

    free(source);
    return EXIT_SUCCESS;
}

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef T32CC_VERSION
#define T32CC_VERSION "0.9.0"
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
    TOK_STRING,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMICOLON,
    TOK_COMMA,
    TOK_MINUS,
    TOK_PLUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_ASSIGN,
    TOK_EQ,
    TOK_NE,
    TOK_LT,
    TOK_LE,
    TOK_GT,
    TOK_GE,
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
    int loop_depth;
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
        "T32 C compiler driver - Stage 17\n\n"
        "Supported source form:\n"
        "  int main(void) { return <integer>; }\n"
        "  int main(void) { int <name> = <integer>; return <name>; }\n"
        "  int main(void) { int <name> = <integer>; <name> = <expr>; return <expr>; }\n\n"
        "Expressions support chained arithmetic, parentheses, and comparisons.\n"
        "Comparison operators are ==, !=, <, <=, >, and >= and produce integer 0 or 1.\n"
        "Stage 17 adds for loops, break, and continue.\n\n"
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

    if (character == '"') {
        int escaped = 0;
        while (!lexer_at_end(lexer)) {
            char c = lexer_advance(lexer);
            if (!escaped && c == '"') {
                return make_token(
                    TOK_STRING,
                    start + 1,
                    (lexer->offset - start_offset) - 2,
                    start_line,
                    start_column);
            }
            if (!escaped && c == '\\')
                escaped = 1;
            else
                escaped = 0;
        }
        return make_token(TOK_INVALID, start,
                          lexer->offset - start_offset,
                          start_line, start_column);
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
    case ',':
        return make_token(TOK_COMMA, start, 1, start_line, start_column);
    case '-':
        return make_token(TOK_MINUS, start, 1, start_line, start_column);
    case '+':
        return make_token(TOK_PLUS, start, 1, start_line, start_column);
    case '*':
        return make_token(TOK_STAR, start, 1, start_line, start_column);
    case '/':
        return make_token(TOK_SLASH, start, 1, start_line, start_column);
    case '%':
        return make_token(TOK_PERCENT, start, 1, start_line, start_column);
    case '=':
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TOK_EQ, start, 2, start_line, start_column);
        }
        return make_token(TOK_ASSIGN, start, 1, start_line, start_column);
    case '!':
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TOK_NE, start, 2, start_line, start_column);
        }
        return make_token(TOK_INVALID, start, 1, start_line, start_column);
    case '<':
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TOK_LE, start, 2, start_line, start_column);
        }
        return make_token(TOK_LT, start, 1, start_line, start_column);
    case '>':
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TOK_GE, start, 2, start_line, start_column);
        }
        return make_token(TOK_GT, start, 1, start_line, start_column);
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
    case TOK_STRING: return "string literal";
    case TOK_LPAREN: return "'('";
    case TOK_RPAREN: return "')'";
    case TOK_LBRACE: return "'{'";
    case TOK_RBRACE: return "'}'";
    case TOK_SEMICOLON: return "';'";
    case TOK_MINUS: return "'-'";
    case TOK_PLUS: return "'+'";
    case TOK_STAR: return "'*'";
    case TOK_SLASH: return "'/'";
    case TOK_PERCENT: return "'%'";
    case TOK_ASSIGN: return "'='";
    case TOK_EQ: return "'=='";
    case TOK_NE: return "'!='";
    case TOK_LT: return "'<'";
    case TOK_LE: return "'<='";
    case TOK_GT: return "'>'";
    case TOK_GE: return "'>='";
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
    OPERAND_INTEGER = 0,
    OPERAND_LOCAL
} operand_kind_t;

typedef struct {
    operand_kind_t kind;
    uint32_t value;
} operand_t;

typedef enum {
    EXPR_OPERAND = 0,
    EXPR_BINARY,
    EXPR_CALL,
    EXPR_STRING
} expression_kind_t;

#define MAX_EXPRESSION_NODES 64

typedef struct {
    expression_kind_t kind;
    operand_t operand;
    token_kind_t operator_kind;
    int left;
    int right;
    char call_name[64];
    int call_args[4];
    size_t call_arg_count;
    unsigned string_id;
    char string_value[160];
} expression_node_t;

typedef struct {
    expression_node_t nodes[MAX_EXPRESSION_NODES];
    size_t node_count;
    int root;
} expression_t;

#define MAX_STATEMENTS 64

typedef enum {
    STMT_ASSIGN = 0,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_RETURN
} statement_kind_t;

typedef struct {
    statement_kind_t kind;
    expression_t expression;   /* assignment RHS or loop/if condition */
    expression_t for_init;     /* STMT_FOR initializer RHS */
    expression_t for_update;   /* STMT_FOR update RHS */
    int target_local;          /* STMT_ASSIGN: local-symbol index */
    int for_init_local;        /* STMT_FOR initializer target */
    int for_update_local;      /* STMT_FOR update target */
    int then_first;            /* STMT_IF/STMT_WHILE/STMT_FOR body */
    int else_first;            /* STMT_IF: first statement in false arm, or -1 */
    int next;                  /* next statement in the same sequence, or -1 */
} statement_t;

#define MAX_LOCALS 16

typedef struct {
    char name[64];
    int has_initializer;
    int is_parameter;
    unsigned parameter_index;
    expression_t initializer;
} local_t;

typedef struct {
    char function_name[64];
    int is_main;
    size_t parameter_count;
    local_t locals[MAX_LOCALS];
    size_t local_count;
    statement_t statements[MAX_STATEMENTS];
    size_t statement_count;
    int first_statement;
    int has_explicit_return;
    expression_t return_expression;
} program_t;

#define MAX_FUNCTIONS 16

typedef struct {
    program_t functions[MAX_FUNCTIONS];
    size_t function_count;
    int main_index;
} translation_unit_t;

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

static int find_local(const program_t *program, const token_t *token)
{
    size_t index;

    for (index = 0; index < program->local_count; ++index) {
        if (token_matches_name(token, program->locals[index].name))
            return (int)index;
    }

    return -1;
}

static token_t parser_peek_token(const parser_t *parser)
{
    lexer_t copy = parser->lexer;
    return lexer_next_token(&copy);
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

static int parse_operand(
    parser_t *parser,
    const program_t *program,
    operand_t *operand)
{
    if (parser->current.kind == TOK_INTEGER ||
        parser->current.kind == TOK_MINUS) {
        operand->kind = OPERAND_INTEGER;
        return parse_integer_literal(parser, &operand->value);
    }

    if (parser->current.kind == TOK_IDENTIFIER) {
        int local_index = find_local(program, &parser->current);

        if (local_index < 0) {
            parser_error(parser, "use of undeclared local variable");
            return 0;
        }

        operand->kind = OPERAND_LOCAL;
        operand->value = (uint32_t)local_index;
        parser_advance(parser);
        return !parser->failed;
    }

    parser_error(parser, "expected integer literal or local variable");
    return 0;
}

static int expression_add_node(
    parser_t *parser,
    expression_t *expression,
    const expression_node_t *node)
{
    if (expression->node_count >= MAX_EXPRESSION_NODES) {
        parser_error(parser, "expression is too complex");
        return -1;
    }

    expression->nodes[expression->node_count] = *node;
    return (int)expression->node_count++;
}

static int parse_equality(
    parser_t *parser,
    const program_t *program,
    expression_t *expression);

static unsigned next_string_id = 0;

static int parse_primary(
    parser_t *parser,
    const program_t *program,
    expression_t *expression)
{
    expression_node_t node;
    memset(&node, 0, sizeof(node));

    if (parser->current.kind == TOK_LPAREN) {
        int root;
        parser_advance(parser);
        root = parse_equality(parser, program, expression);
        if (root < 0)
            return -1;
        if (!parser_expect_kind(parser, TOK_RPAREN, "')'"))
            return -1;
        return root;
    }

    if (parser->current.kind == TOK_STRING) {
        size_t i = 0;
        size_t o = 0;
        const token_t token = parser->current;

        node.kind = EXPR_STRING;
        node.string_id = next_string_id++;

        while (i < token.length) {
            unsigned char c = (unsigned char)token.start[i++];
            if (c == '\\' && i < token.length) {
                unsigned char e = (unsigned char)token.start[i++];
                if (e == 'n') c = '\n';
                else if (e == 'r') c = '\r';
                else if (e == 't') c = '\t';
                else if (e == '\\') c = '\\';
                else if (e == '"') c = '"';
                else {
                    parser_error(parser, "unsupported string escape");
                    return -1;
                }
            }
            if (o + 1 >= sizeof(node.string_value)) {
                parser_error(parser, "string literal is too long");
                return -1;
            }
            node.string_value[o++] = (char)c;
        }
        node.string_value[o] = '\0';
        parser_advance(parser);
        return expression_add_node(parser, expression, &node);
    }

    if (parser->current.kind == TOK_IDENTIFIER &&
        parser_peek_token(parser).kind == TOK_LPAREN) {
        token_t name = parser->current;
        size_t arg_count = 0;

        node.kind = EXPR_CALL;
        if (!copy_identifier(&name, node.call_name, sizeof(node.call_name))) {
            parser_error(parser, "function name is too long");
            return -1;
        }

        parser_advance(parser);
        if (!parser_expect_kind(parser, TOK_LPAREN, "'('"))
            return -1;

        if (parser->current.kind != TOK_RPAREN) {
            for (;;) {
                int arg_root;
                if (arg_count >= 4) {
                    parser_error(parser, "Stage 17 supports at most four function arguments");
                    return -1;
                }
                arg_root = parse_equality(parser, program, expression);
                if (arg_root < 0)
                    return -1;
                node.call_args[arg_count++] = arg_root;
                if (parser->current.kind != TOK_COMMA)
                    break;
                parser_advance(parser);
            }
        }

        if (!parser_expect_kind(parser, TOK_RPAREN, "')'"))
            return -1;
        node.call_arg_count = arg_count;
        return expression_add_node(parser, expression, &node);
    }

    node.kind = EXPR_OPERAND;
    if (!parse_operand(parser, program, &node.operand))
        return -1;
    return expression_add_node(parser, expression, &node);
}

static int parse_multiplicative(
    parser_t *parser,
    const program_t *program,
    expression_t *expression)
{
    int left = parse_primary(parser, program, expression);

    if (left < 0)
        return -1;

    while (parser->current.kind == TOK_STAR ||
           parser->current.kind == TOK_SLASH ||
           parser->current.kind == TOK_PERCENT) {
        token_kind_t operator_kind = parser->current.kind;
        expression_node_t node;
        int right;

        parser_advance(parser);
        right = parse_primary(parser, program, expression);
        if (right < 0)
            return -1;

        memset(&node, 0, sizeof(node));
        node.kind = EXPR_BINARY;
        node.operator_kind = operator_kind;
        node.left = left;
        node.right = right;
        left = expression_add_node(parser, expression, &node);
        if (left < 0)
            return -1;
    }

    return left;
}

static int parse_additive(
    parser_t *parser,
    const program_t *program,
    expression_t *expression)
{
    int left = parse_multiplicative(parser, program, expression);

    if (left < 0)
        return -1;

    while (parser->current.kind == TOK_PLUS ||
           parser->current.kind == TOK_MINUS) {
        token_kind_t operator_kind = parser->current.kind;
        expression_node_t node;
        int right;

        parser_advance(parser);
        right = parse_multiplicative(parser, program, expression);
        if (right < 0)
            return -1;

        memset(&node, 0, sizeof(node));
        node.kind = EXPR_BINARY;
        node.operator_kind = operator_kind;
        node.left = left;
        node.right = right;
        left = expression_add_node(parser, expression, &node);
        if (left < 0)
            return -1;
    }

    return left;
}

static int parse_relational(
    parser_t *parser,
    const program_t *program,
    expression_t *expression)
{
    int left = parse_additive(parser, program, expression);

    if (left < 0)
        return -1;

    while (parser->current.kind == TOK_LT ||
           parser->current.kind == TOK_LE ||
           parser->current.kind == TOK_GT ||
           parser->current.kind == TOK_GE) {
        token_kind_t operator_kind = parser->current.kind;
        expression_node_t node;
        int right;

        parser_advance(parser);
        right = parse_additive(parser, program, expression);
        if (right < 0)
            return -1;

        memset(&node, 0, sizeof(node));
        node.kind = EXPR_BINARY;
        node.operator_kind = operator_kind;
        node.left = left;
        node.right = right;
        left = expression_add_node(parser, expression, &node);
        if (left < 0)
            return -1;
    }

    return left;
}

static int parse_equality(
    parser_t *parser,
    const program_t *program,
    expression_t *expression)
{
    int left = parse_relational(parser, program, expression);

    if (left < 0)
        return -1;

    while (parser->current.kind == TOK_EQ ||
           parser->current.kind == TOK_NE) {
        token_kind_t operator_kind = parser->current.kind;
        expression_node_t node;
        int right;

        parser_advance(parser);
        right = parse_relational(parser, program, expression);
        if (right < 0)
            return -1;

        memset(&node, 0, sizeof(node));
        node.kind = EXPR_BINARY;
        node.operator_kind = operator_kind;
        node.left = left;
        node.right = right;
        left = expression_add_node(parser, expression, &node);
        if (left < 0)
            return -1;
    }

    return left;
}

static int parse_expression(
    parser_t *parser,
    const program_t *program,
    expression_t *expression)
{
    memset(expression, 0, sizeof(*expression));
    expression->root = parse_equality(parser, program, expression);
    return expression->root >= 0 && !parser->failed;
}

static int statement_add(program_t *program, const statement_t *statement)
{
    if (program->statement_count >= MAX_STATEMENTS)
        return -1;

    program->statements[program->statement_count] = *statement;
    return (int)program->statement_count++;
}

static int parse_statement(
    parser_t *parser,
    program_t *program,
    int *statement_index_out);

static int parse_statement_sequence(
    parser_t *parser,
    program_t *program,
    int stop_at_rbrace,
    int *first_out)
{
    int first = -1;
    int last = -1;

    while (!parser->failed) {
        int current;

        if (stop_at_rbrace && parser->current.kind == TOK_RBRACE)
            break;

        if (!stop_at_rbrace &&
            (token_is_identifier(&parser->current, "return") ||
             parser->current.kind == TOK_RBRACE))
            break;

        if (parser->current.kind == TOK_EOF) {
            parser_error(parser,
                stop_at_rbrace ? "expected '}'" : "expected return statement or '}'");
            return 0;
        }

        if (!parse_statement(parser, program, &current))
            return 0;

        if (first < 0)
            first = current;
        if (last >= 0)
            program->statements[last].next = current;
        last = current;
    }

    *first_out = first;
    return !parser->failed;
}

static int parse_statement_or_block(
    parser_t *parser,
    program_t *program,
    int *first_out)
{
    if (parser->current.kind == TOK_LBRACE) {
        parser_advance(parser);

        if (!parse_statement_sequence(parser, program, 1, first_out))
            return 0;

        if (!parser_expect_kind(parser, TOK_RBRACE, "'}'"))
            return 0;

        return 1;
    }

    return parse_statement(parser, program, first_out);
}

static int parse_assignment_clause(
    parser_t *parser,
    program_t *program,
    int *target_local_out,
    expression_t *expression_out)
{
    int local_index;

    if (parser->current.kind != TOK_IDENTIFIER) {
        parser_error(parser, "expected local variable assignment");
        return 0;
    }

    local_index = find_local(program, &parser->current);
    if (local_index < 0) {
        parser_error(parser, "assignment to undeclared local variable");
        return 0;
    }

    parser_advance(parser);
    if (!parser_expect_kind(parser, TOK_ASSIGN, "'='"))
        return 0;
    if (!parse_expression(parser, program, expression_out))
        return 0;

    *target_local_out = local_index;
    return 1;
}

static int parse_statement(
    parser_t *parser,
    program_t *program,
    int *statement_index_out)
{
    statement_t statement;
    int index;

    memset(&statement, 0, sizeof(statement));
    statement.target_local = -1;
    statement.for_init_local = -1;
    statement.for_update_local = -1;
    statement.then_first = -1;
    statement.else_first = -1;
    statement.next = -1;

    if (token_is_identifier(&parser->current, "break")) {
        if (parser->loop_depth <= 0) {
            parser_error(parser, "break statement is not inside a loop");
            return 0;
        }
        statement.kind = STMT_BREAK;
        parser_advance(parser);
        if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'"))
            return 0;
        index = statement_add(program, &statement);
        if (index < 0) {
            parser_error(parser, "too many statements");
            return 0;
        }
        *statement_index_out = index;
        return 1;
    }

    if (token_is_identifier(&parser->current, "continue")) {
        if (parser->loop_depth <= 0) {
            parser_error(parser, "continue statement is not inside a loop");
            return 0;
        }
        statement.kind = STMT_CONTINUE;
        parser_advance(parser);
        if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'"))
            return 0;
        index = statement_add(program, &statement);
        if (index < 0) {
            parser_error(parser, "too many statements");
            return 0;
        }
        *statement_index_out = index;
        return 1;
    }

    if (token_is_identifier(&parser->current, "for")) {
        statement.kind = STMT_FOR;
        parser_advance(parser);

        if (!parser_expect_kind(parser, TOK_LPAREN, "'('"))
            return 0;
        if (!parse_assignment_clause(
                parser, program,
                &statement.for_init_local,
                &statement.for_init))
            return 0;
        if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'"))
            return 0;
        if (!parse_expression(parser, program, &statement.expression))
            return 0;
        if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'"))
            return 0;
        if (!parse_assignment_clause(
                parser, program,
                &statement.for_update_local,
                &statement.for_update))
            return 0;
        if (!parser_expect_kind(parser, TOK_RPAREN, "')'"))
            return 0;

        index = statement_add(program, &statement);
        if (index < 0) {
            parser_error(parser, "too many statements");
            return 0;
        }

        parser->loop_depth++;
        if (!parse_statement_or_block(
                parser, program, &program->statements[index].then_first)) {
            parser->loop_depth--;
            return 0;
        }
        parser->loop_depth--;

        *statement_index_out = index;
        return 1;
    }

    if (token_is_identifier(&parser->current, "while")) {
        statement.kind = STMT_WHILE;
        parser_advance(parser);

        if (!parser_expect_kind(parser, TOK_LPAREN, "'('"))
            return 0;
        if (!parse_expression(parser, program, &statement.expression))
            return 0;
        if (!parser_expect_kind(parser, TOK_RPAREN, "')'"))
            return 0;

        index = statement_add(program, &statement);
        if (index < 0) {
            parser_error(parser, "too many statements");
            return 0;
        }

        parser->loop_depth++;
        if (!parse_statement_or_block(
                parser, program, &program->statements[index].then_first)) {
            parser->loop_depth--;
            return 0;
        }
        parser->loop_depth--;

        *statement_index_out = index;
        return 1;
    }

    if (token_is_identifier(&parser->current, "if")) {
        statement.kind = STMT_IF;
        parser_advance(parser);

        if (!parser_expect_kind(parser, TOK_LPAREN, "'('"))
            return 0;
        if (!parse_expression(parser, program, &statement.expression))
            return 0;
        if (!parser_expect_kind(parser, TOK_RPAREN, "')'"))
            return 0;

        index = statement_add(program, &statement);
        if (index < 0) {
            parser_error(parser, "too many statements");
            return 0;
        }

        if (!parse_statement_or_block(
                parser, program, &program->statements[index].then_first))
            return 0;

        if (token_is_identifier(&parser->current, "else")) {
            parser_advance(parser);
            if (!parse_statement_or_block(
                    parser, program, &program->statements[index].else_first))
                return 0;
        }

        *statement_index_out = index;
        return 1;
    }

    if (token_is_identifier(&parser->current, "int")) {
        parser_error(parser,
            "Stage 17 requires local declarations before executable statements");
        return 0;
    }

    if (token_is_identifier(&parser->current, "return")) {
        statement.kind = STMT_RETURN;
        parser_advance(parser);
        if (!parse_expression(parser, program, &statement.expression))
            return 0;
        if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'"))
            return 0;

        index = statement_add(program, &statement);
        if (index < 0) {
            parser_error(parser, "too many statements");
            return 0;
        }

        *statement_index_out = index;
        return 1;
    }

    if (parser->current.kind != TOK_IDENTIFIER) {
        parser_error(parser, "expected assignment, if, while, for, break, continue, or return statement");
        return 0;
    }

    statement.target_local = find_local(program, &parser->current);
    if (statement.target_local < 0) {
        parser_error(parser, "assignment to undeclared local variable");
        return 0;
    }

    statement.kind = STMT_ASSIGN;
    parser_advance(parser);

    if (!parser_expect_kind(parser, TOK_ASSIGN, "'='"))
        return 0;
    if (!parse_expression(parser, program, &statement.expression))
        return 0;
    if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'"))
        return 0;

    index = statement_add(program, &statement);
    if (index < 0) {
        parser_error(parser, "too many statements");
        return 0;
    }

    *statement_index_out = index;
    return 1;
}


static int runtime_external_arity(const char *name)
{
    if (strcmp(name, "putchar") == 0)
        return 1;
    if (strcmp(name, "puts") == 0)
        return 1;
    return -1;
}

static int find_function(const translation_unit_t *unit, const char *name)
{
    size_t i;
    for (i = 0; i < unit->function_count; ++i) {
        if (strcmp(unit->functions[i].function_name, name) == 0)
            return (int)i;
    }
    return -1;
}

static int expression_validate_calls(
    parser_t *parser,
    const translation_unit_t *unit,
    const expression_t *expression)
{
    size_t i;
    for (i = 0; i < expression->node_count; ++i) {
        const expression_node_t *node = &expression->nodes[i];
        int function_index;
        if (node->kind != EXPR_CALL)
            continue;
function_index = find_function(unit, node->call_name);
        if (function_index < 0) {
            int arity = runtime_external_arity(node->call_name);
            if (arity < 0) {
                parser_error(parser, "call to undefined function");
                return 0;
            }
            if ((size_t)arity != node->call_arg_count) {
                parser_error(parser, "function argument count mismatch");
                return 0;
            }
            continue;
        }
        if (unit->functions[function_index].parameter_count != node->call_arg_count) {
            parser_error(parser, "function argument count mismatch");
            return 0;
        }
    }
    return 1;
}

static int validate_program_calls(
    parser_t *parser,
    const translation_unit_t *unit,
    const program_t *program)
{
    size_t i;
    for (i = program->parameter_count; i < program->local_count; ++i) {
        if (program->locals[i].has_initializer &&
            !expression_validate_calls(parser, unit, &program->locals[i].initializer))
            return 0;
    }
    for (i = 0; i < program->statement_count; ++i) {
        const statement_t *st = &program->statements[i];
        if (st->kind == STMT_ASSIGN &&
            !expression_validate_calls(parser, unit, &st->expression)) return 0;
        if ((st->kind == STMT_IF || st->kind == STMT_WHILE) &&
            !expression_validate_calls(parser, unit, &st->expression)) return 0;
        if (st->kind == STMT_FOR) {
            if (!expression_validate_calls(parser, unit, &st->for_init)) return 0;
            if (!expression_validate_calls(parser, unit, &st->expression)) return 0;
            if (!expression_validate_calls(parser, unit, &st->for_update)) return 0;
        }
        if (st->kind == STMT_RETURN &&
            !expression_validate_calls(parser, unit, &st->expression))
            return 0;
    }
    if (program->has_explicit_return &&
        !expression_validate_calls(parser, unit, &program->return_expression))
        return 0;
    return 1;
}

static int parse_function(parser_t *parser, program_t *program)
{
    token_t function_name;
    memset(program, 0, sizeof(*program));
    program->first_statement = -1;

    if (!parser_expect_word(parser, "int")) return 0;
    if (parser->current.kind != TOK_IDENTIFIER) {
        parser_error(parser, "expected function name");
        return 0;
    }
    function_name = parser->current;
    if (!copy_identifier(&function_name, program->function_name,
                         sizeof(program->function_name))) {
        parser_error(parser, "function name is too long");
        return 0;
    }
    program->is_main = strcmp(program->function_name, "main") == 0;
    parser_advance(parser);

    if (!parser_expect_kind(parser, TOK_LPAREN, "'('")) return 0;
    if (token_is_identifier(&parser->current, "void")) {
        parser_advance(parser);
    } else if (parser->current.kind != TOK_RPAREN) {
        for (;;) {
            local_t *parameter;
            token_t name;
            if (program->parameter_count >= 4) {
                parser_error(parser, "Stage 17 supports at most four parameters");
                return 0;
            }
            if (!parser_expect_word(parser, "int")) return 0;
            if (parser->current.kind != TOK_IDENTIFIER) {
                parser_error(parser, "expected parameter name");
                return 0;
            }
            name = parser->current;
            if (find_local(program, &name) >= 0) {
                parser_error(parser, "duplicate parameter name");
                return 0;
            }
            parameter = &program->locals[program->local_count];
            if (!copy_identifier(&name, parameter->name, sizeof(parameter->name))) {
                parser_error(parser, "parameter name is too long");
                return 0;
            }
            parameter->is_parameter = 1;
            parameter->parameter_index = (unsigned)program->parameter_count;
            parameter->has_initializer = 0;
            program->parameter_count++;
            program->local_count++;
            parser_advance(parser);
            if (parser->current.kind != TOK_COMMA)
                break;
            parser_advance(parser);
        }
    }
    if (!parser_expect_kind(parser, TOK_RPAREN, "')'")) return 0;
    if (program->is_main && program->parameter_count != 0) {
        parser_error(parser, "Stage 17 main must have no parameters");
        return 0;
    }
    if (!parser_expect_kind(parser, TOK_LBRACE, "'{'")) return 0;

    while (token_is_identifier(&parser->current, "int")) {
        token_t name_token;
        local_t *local;
        if (program->local_count >= MAX_LOCALS) {
            parser_error(parser, "too many local variables");
            return 0;
        }
        parser_advance(parser);
        if (parser->current.kind != TOK_IDENTIFIER) {
            parser_error(parser, "expected local variable name");
            return 0;
        }
        name_token = parser->current;
        if (find_local(program, &name_token) >= 0) {
            parser_error(parser, "duplicate local variable");
            return 0;
        }
        local = &program->locals[program->local_count];
        if (!copy_identifier(&name_token, local->name, sizeof(local->name))) {
            parser_error(parser, "local variable name is too long");
            return 0;
        }
        parser_advance(parser);
        if (parser->current.kind == TOK_ASSIGN) {
            local->has_initializer = 1;
            parser_advance(parser);
            if (!parse_expression(parser, program, &local->initializer)) return 0;
        } else {
            local->has_initializer = 0;
        }
        if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'")) return 0;
        program->local_count++;
    }

    if (!parse_statement_sequence(parser, program, 0, &program->first_statement))
        return 0;

    if (token_is_identifier(&parser->current, "return")) {
        program->has_explicit_return = 1;
        parser_advance(parser);
        if (!parse_expression(parser, program, &program->return_expression)) return 0;
        if (!parser_expect_kind(parser, TOK_SEMICOLON, "';'")) return 0;
    } else {
        program->has_explicit_return = 0;
        if (!program->is_main) {
            parser_error(parser, "non-main int function requires return value");
            return 0;
        }
    }
    if (!parser_expect_kind(parser, TOK_RBRACE, "'}'")) return 0;
    return !parser->failed;
}

static int parse_translation_unit(parser_t *parser, translation_unit_t *unit)
{
    size_t i;
    memset(unit, 0, sizeof(*unit));
    unit->main_index = -1;
    parser_advance(parser);

    while (parser->current.kind != TOK_EOF) {
        program_t *function;
        if (unit->function_count >= MAX_FUNCTIONS) {
            parser_error(parser, "too many functions");
            return 0;
        }
        function = &unit->functions[unit->function_count];
        if (!parse_function(parser, function)) return 0;
        for (i = 0; i < unit->function_count; ++i) {
            if (strcmp(unit->functions[i].function_name, function->function_name) == 0) {
                parser_error(parser, "duplicate function definition");
                return 0;
            }
        }
        if (function->is_main) {
            if (unit->main_index >= 0) {
                parser_error(parser, "duplicate main definition");
                return 0;
            }
            unit->main_index = (int)unit->function_count;
        }
        unit->function_count++;
    }
    if (unit->main_index < 0) {
        parser_error(parser, "translation unit requires main");
        return 0;
    }
    for (i = 0; i < unit->function_count; ++i) {
        if (!validate_program_calls(parser, unit, &unit->functions[i]))
            return 0;
    }
    return !parser->failed;
}

static void emit_local_address(FILE *output, uint32_t local_index)
{
    uint32_t offset = local_index * 4u;

    if (offset == 0u)
        return;

    fprintf(output, "    movi r10, 0x%08X\n", offset);
    fprintf(output, "    add  r10, r15, r10\n");
}

static void emit_load_local(
    FILE *output,
    uint32_t local_index,
    unsigned reg)
{
    if (local_index == 0u) {
        fprintf(output, "    ldw  r%u, [r15]\n", reg);
        return;
    }

    emit_local_address(output, local_index);
    fprintf(output, "    ldw  r%u, [r10]\n", reg);
}

static void emit_store_local(
    FILE *output,
    uint32_t local_index,
    unsigned reg)
{
    if (local_index == 0u) {
        fprintf(output, "    stw  r%u, [r15]\n", reg);
        return;
    }

    emit_local_address(output, local_index);
    fprintf(output, "    stw  r%u, [r10]\n", reg);
}

static void emit_operand(FILE *output, const operand_t *operand, unsigned reg)
{
    if (operand->kind == OPERAND_LOCAL)
        emit_load_local(output, operand->value, reg);
    else
        fprintf(output, "    movi r%u, 0x%08X\n", reg, operand->value);
}

static void emit_bool_nonzero(FILE *output, unsigned reg)
{
    /* Normalize any 32-bit value to 0 or 1 without consuming condition flags. */
    fprintf(output, "    movi r10, 0x00000000\n");
    fprintf(output, "    sub  r10, r10, r%u\n", reg);
    fprintf(output, "    or   r%u, r%u, r10\n", reg, reg);
    fprintf(output, "    movi r10, 0x0000001F\n");
    fprintf(output, "    shr  r%u, r%u, r10\n", reg, reg);
}

static void emit_bool_invert(FILE *output, unsigned reg)
{
    /* Input is already normalized to 0/1. */
    fprintf(output, "    movi r10, 0x00000001\n");
    fprintf(output, "    xor  r%u, r%u, r10\n", reg, reg);
}

static void emit_signed_less_than(
    FILE *output,
    unsigned destination,
    unsigned left_reg,
    unsigned right_reg)
{
    /*
     * Signed x < y without relying on hidden condition flags.
     * If signs differ, x's sign decides. If signs match, the sign of x-y
     * decides. The bitwise selection below also handles subtraction overflow.
     */
    fprintf(output, "    sub  r10, r%u, r%u\n", left_reg, right_reg);
    fprintf(output, "    xor  r11, r%u, r%u\n", left_reg, right_reg);
    fprintf(output, "    and  r12, r11, r%u\n", left_reg);
    fprintf(output, "    not  r11, r11\n");
    fprintf(output, "    and  r11, r11, r10\n");
    fprintf(output, "    or   r10, r12, r11\n");
    fprintf(output, "    movi r11, 0x0000001F\n");
    fprintf(output, "    shr  r%u, r10, r11\n", destination);
}


#define MAX_CALL_SCRATCH_SLOTS 16

static uint32_t expression_scratch_base = 0;
static unsigned expression_scratch_depth = 0;

static int expression_node_contains_call(
    const expression_t *expression,
    int node_index)
{
    const expression_node_t *node;
    size_t i;

    if (node_index < 0 || (size_t)node_index >= expression->node_count)
        return 0;

    node = &expression->nodes[node_index];
    if (node->kind == EXPR_CALL)
        return 1;
    if (node->kind == EXPR_BINARY)
        return expression_node_contains_call(expression, node->left) ||
               expression_node_contains_call(expression, node->right);

    for (i = 0; i < node->call_arg_count; ++i) {
        if (expression_node_contains_call(expression, node->call_args[i]))
            return 1;
    }
    return 0;
}

static int expression_contains_call(const expression_t *expression)
{
    return expression_node_contains_call(expression, expression->root);
}

static int scratch_store(FILE *output, unsigned reg, unsigned *slot_out)
{
    unsigned slot;

    if (expression_scratch_depth >= MAX_CALL_SCRATCH_SLOTS) {
        fprintf(stderr,
            "t32-cc: Stage 17 expression call nesting exceeds scratch capacity\n");
        return 0;
    }

    slot = expression_scratch_depth++;
    emit_store_local(output, expression_scratch_base + slot, reg);
    if (slot_out)
        *slot_out = slot;
    return 1;
}

static void scratch_load_slot(FILE *output, unsigned slot, unsigned reg)
{
    emit_load_local(output, expression_scratch_base + slot, reg);
}

static void scratch_release_to(unsigned depth)
{
    expression_scratch_depth = depth;
}

static int program_contains_calls(const program_t *program)
{
    size_t i;

    for (i = program->parameter_count; i < program->local_count; ++i) {
        if (program->locals[i].has_initializer &&
            expression_contains_call(&program->locals[i].initializer))
            return 1;
    }

    for (i = 0; i < program->statement_count; ++i) {
        const statement_t *st = &program->statements[i];
        if ((st->kind == STMT_ASSIGN ||
             st->kind == STMT_IF ||
             st->kind == STMT_WHILE ||
             st->kind == STMT_RETURN) &&
            expression_contains_call(&st->expression))
            return 1;
        if (st->kind == STMT_FOR &&
            (expression_contains_call(&st->for_init) ||
             expression_contains_call(&st->expression) ||
             expression_contains_call(&st->for_update)))
            return 1;
    }

    return program->has_explicit_return &&
           expression_contains_call(&program->return_expression);
}

static int emit_expression_node(
    FILE *output,
    const expression_t *expression,
    int node_index,
    unsigned destination,
    unsigned next_register)
{
    const expression_node_t *node;

    if (node_index < 0 || (size_t)node_index >= expression->node_count)
        return 0;

    node = &expression->nodes[node_index];

    if (node->kind == EXPR_OPERAND) {
        emit_operand(output, &node->operand, destination);
        return 1;
    }

    if (node->kind == EXPR_STRING) {
        fprintf(output, "    movi r%u, .Lstr_%u\n",
                destination, node->string_id);
        return 1;
    }

    if (node->kind == EXPR_CALL) {
        size_t i;
        unsigned start_depth = expression_scratch_depth;
        unsigned arg_slots[4];

        /*
         * Evaluate argument expressions first and stage their values in
         * caller-owned scratch slots. Only after all arguments are stable do
         * we load r0-r3. This makes nested calls safe even though r0-r7 are
         * caller-saved.
         */
        for (i = 0; i < node->call_arg_count; ++i) {
            if (!emit_expression_node(
                    output, expression, node->call_args[i], 0, 1))
                return 0;
            if (!scratch_store(output, 0, &arg_slots[i]))
                return 0;
        }

        for (i = 0; i < node->call_arg_count; ++i)
            scratch_load_slot(output, arg_slots[i], (unsigned)i);

        scratch_release_to(start_depth);
        fprintf(output, "    call %s\n", node->call_name);
        if (destination != 0u)
            fprintf(output, "    mov  r%u, r0\n", destination);
        return 1;
    }

    /*
     * Stage 17 deliberately uses a simple temporary-register stack.
     * r10-r13 are reserved for comparison/remainder scratch work. This keeps
     * expression code generation transparent until a real allocator/spill
     * policy is introduced.
     */
    if (next_register >= 10) {
        fprintf(stderr,
            "t32-cc: expression requires more temporary registers than Stage 17 provides\n");
        return 0;
    }

    if (!emit_expression_node(
            output, expression, node->left, destination, next_register))
        return 0;

    if (expression_node_contains_call(expression, node->right)) {
        unsigned saved_slot;
        unsigned saved_depth = expression_scratch_depth;

        if (!scratch_store(output, destination, &saved_slot))
            return 0;
        if (!emit_expression_node(
                output, expression, node->right,
                next_register, next_register + 1))
            return 0;
        scratch_load_slot(output, saved_slot, destination);
        scratch_release_to(saved_depth);
    } else {
        if (!emit_expression_node(
                output, expression, node->right,
                next_register, next_register + 1))
            return 0;
    }

    if (node->operator_kind == TOK_PLUS) {
        fprintf(output, "    add  r%u, r%u, r%u\n",
            destination, destination, next_register);
    } else if (node->operator_kind == TOK_MINUS) {
        fprintf(output, "    sub  r%u, r%u, r%u\n",
            destination, destination, next_register);
    } else if (node->operator_kind == TOK_STAR) {
        fprintf(output, "    mul  r%u, r%u, r%u\n",
            destination, destination, next_register);
    } else if (node->operator_kind == TOK_SLASH) {
        fprintf(output, "    div  r%u, r%u, r%u\n",
            destination, destination, next_register);
    } else if (node->operator_kind == TOK_PERCENT) {
        /* C remainder: a - (a / b) * b, with division toward zero. */
        fprintf(output, "    div  r13, r%u, r%u\n",
            destination, next_register);
        fprintf(output, "    mul  r13, r13, r%u\n", next_register);
        fprintf(output, "    sub  r%u, r%u, r13\n",
            destination, destination);
    } else if (node->operator_kind == TOK_EQ ||
               node->operator_kind == TOK_NE) {
        fprintf(output, "    xor  r%u, r%u, r%u\n",
            destination, destination, next_register);
        emit_bool_nonzero(output, destination);
        if (node->operator_kind == TOK_EQ)
            emit_bool_invert(output, destination);
    } else if (node->operator_kind == TOK_LT) {
        emit_signed_less_than(
            output, destination, destination, next_register);
    } else if (node->operator_kind == TOK_GT) {
        emit_signed_less_than(
            output, destination, next_register, destination);
    } else if (node->operator_kind == TOK_LE) {
        emit_signed_less_than(
            output, destination, next_register, destination);
        emit_bool_invert(output, destination);
    } else if (node->operator_kind == TOK_GE) {
        emit_signed_less_than(
            output, destination, destination, next_register);
        emit_bool_invert(output, destination);
    } else {
        return 0;
    }

    return 1;
}

static int emit_expression(
    FILE *output,
    const expression_t *expression,
    unsigned destination,
    unsigned temporary)
{
    expression_scratch_depth = 0;
    return emit_expression_node(
        output, expression, expression->root, destination, temporary);
}

typedef enum {
    LOOP_NONE = 0,
    LOOP_WHILE,
    LOOP_FOR
} loop_kind_t;

typedef struct loop_context {
    loop_kind_t kind;
    unsigned id;
    const struct loop_context *outer;
} loop_context_t;

static int emit_statement_sequence(
    FILE *output,
    const program_t *program,
    int first_statement,
    unsigned *label_counter,
    const loop_context_t *loop_context);

static const char *current_function_epilogue = NULL;

static int emit_statement(
    FILE *output,
    const program_t *program,
    int statement_index,
    unsigned *label_counter,
    const loop_context_t *loop_context)
{
    const statement_t *statement;

    if (statement_index < 0 ||
        (size_t)statement_index >= program->statement_count)
        return 0;

    statement = &program->statements[statement_index];

    if (statement->kind == STMT_ASSIGN) {
        fprintf(output, "    ; assign %s\n",
            program->locals[statement->target_local].name);
        if (!emit_expression(output, &statement->expression, 1, 2))
            return 0;
        emit_store_local(output, (uint32_t)statement->target_local, 1);
        return 1;
    }

    if (statement->kind == STMT_RETURN) {
        if (!emit_expression(output, &statement->expression, 0, 1))
            return 0;
        if (!current_function_epilogue)
            return 0;
        fprintf(output, "    jmp  %s\n", current_function_epilogue);
        return 1;
    }

    if (statement->kind == STMT_BREAK) {
        if (!loop_context)
            return 0;
        if (loop_context->kind == LOOP_WHILE)
            fprintf(output, "    jmp  .Lendwhile_%u\n", loop_context->id);
        else
            fprintf(output, "    jmp  .Lendfor_%u\n", loop_context->id);
        return 1;
    }

    if (statement->kind == STMT_CONTINUE) {
        if (!loop_context)
            return 0;
        if (loop_context->kind == LOOP_WHILE)
            fprintf(output, "    jmp  .Lwhile_%u\n", loop_context->id);
        else
            fprintf(output, "    jmp  .Lfor_update_%u\n", loop_context->id);
        return 1;
    }

    if (statement->kind == STMT_IF) {
        unsigned id = (*label_counter)++;
        const char *false_prefix =
            statement->else_first >= 0 ? ".Lelse" : ".Lendif";

        fprintf(output, "    ; if condition\n");
        if (!emit_expression(output, &statement->expression, 0, 1))
            return 0;

        fprintf(output, "    jz   r0, %s_%u\n", false_prefix, id);

        if (!emit_statement_sequence(
                output, program, statement->then_first,
                label_counter, loop_context))
            return 0;

        if (statement->else_first >= 0) {
            fprintf(output, "    jmp  .Lendif_%u\n", id);
            fprintf(output, ".Lelse_%u:\n", id);

            if (!emit_statement_sequence(
                    output, program, statement->else_first,
                    label_counter, loop_context))
                return 0;
        }

        fprintf(output, ".Lendif_%u:\n", id);
        return 1;
    }

    if (statement->kind == STMT_WHILE) {
        unsigned id = (*label_counter)++;
        loop_context_t inner;

        inner.kind = LOOP_WHILE;
        inner.id = id;
        inner.outer = loop_context;

        fprintf(output, ".Lwhile_%u:\n", id);
        fprintf(output, "    ; while condition\n");
        if (!emit_expression(output, &statement->expression, 0, 1))
            return 0;
        fprintf(output, "    jz   r0, .Lendwhile_%u\n", id);

        if (!emit_statement_sequence(
                output, program, statement->then_first,
                label_counter, &inner))
            return 0;

        fprintf(output, "    jmp  .Lwhile_%u\n", id);
        fprintf(output, ".Lendwhile_%u:\n", id);
        return 1;
    }

    if (statement->kind == STMT_FOR) {
        unsigned id = (*label_counter)++;
        loop_context_t inner;

        inner.kind = LOOP_FOR;
        inner.id = id;
        inner.outer = loop_context;

        fprintf(output, "    ; for initializer\n");
        if (!emit_expression(output, &statement->for_init, 1, 2))
            return 0;
        emit_store_local(
            output, (uint32_t)statement->for_init_local, 1);

        fprintf(output, ".Lfor_%u:\n", id);
        fprintf(output, "    ; for condition\n");
        if (!emit_expression(output, &statement->expression, 0, 1))
            return 0;
        fprintf(output, "    jz   r0, .Lendfor_%u\n", id);

        if (!emit_statement_sequence(
                output, program, statement->then_first,
                label_counter, &inner))
            return 0;

        fprintf(output, ".Lfor_update_%u:\n", id);
        fprintf(output, "    ; for update\n");
        if (!emit_expression(output, &statement->for_update, 1, 2))
            return 0;
        emit_store_local(
            output, (uint32_t)statement->for_update_local, 1);
        fprintf(output, "    jmp  .Lfor_%u\n", id);
        fprintf(output, ".Lendfor_%u:\n", id);
        return 1;
    }

    return 0;
}

static int emit_statement_sequence(
    FILE *output,
    const program_t *program,
    int first_statement,
    unsigned *label_counter,
    const loop_context_t *loop_context)
{
    int current = first_statement;

    while (current >= 0) {
        int next = program->statements[current].next;

        if (!emit_statement(
                output, program, current,
                label_counter, loop_context))
            return 0;

        current = next;
    }

    return 1;
}

static int emit_function(
    FILE *output,
    const program_t *program,
    unsigned *global_label_counter)
{
    size_t local_index;
    int has_calls = program_contains_calls(program);
    uint32_t frame_slots = (uint32_t)program->local_count +
        (has_calls ? MAX_CALL_SCRATCH_SLOTS : 0u);
    uint32_t frame_size = frame_slots * 4u;
    char epilogue_label[96];

    snprintf(epilogue_label, sizeof(epilogue_label),
             ".Lreturn_%s", program->function_name);
    current_function_epilogue = epilogue_label;
    expression_scratch_base = (uint32_t)program->local_count;
    expression_scratch_depth = 0;

    fprintf(output, "%s:\n", program->function_name);

    if (frame_size > 0) {
        if (program->is_main && program->parameter_count == 0) {
            fprintf(output,
                "    ; %zu local int(s), %u-byte stack frame\n"
                "    subi r15, r15, %u\n",
                program->local_count, frame_size, frame_size);
        } else {
            fprintf(output,
                "    ; %zu parameter/local slot(s), %u-byte stack frame\n"
                "    subi r15, r15, %u\n",
                program->local_count, frame_size, frame_size);
        }

        for (local_index = 0; local_index < program->parameter_count; ++local_index) {
            fprintf(output, "    ; parameter int %s from r%zu\n",
                program->locals[local_index].name, local_index);
            emit_store_local(output, (uint32_t)local_index, (unsigned)local_index);
        }

        for (local_index = program->parameter_count;
             local_index < program->local_count; ++local_index) {
            if (!program->locals[local_index].has_initializer) {
                fprintf(output,
                    "    ; uninitialized local int %s at [r15 + %zu]\n",
                    program->locals[local_index].name, local_index * 4u);
                continue;
            }
            fprintf(output,
                "    ; initialize local int %s at [r15 + %zu]\n",
                program->locals[local_index].name, local_index * 4u);
            if (!emit_expression(output, &program->locals[local_index].initializer, 1, 2))
                return 0;
            emit_store_local(output, (uint32_t)local_index, 1);
        }
    }

    if (!emit_statement_sequence(output, program, program->first_statement,
                                 global_label_counter, NULL))
        return 0;

    if (program->has_explicit_return) {
        if (!emit_expression(output, &program->return_expression, 0, 1))
            return 0;
    } else {
        fprintf(output,
            "    ; implicit return 0 from main\n"
            "    movi r0, 0x00000000\n");
    }

    fprintf(output, "%s:\n", epilogue_label);
    if (frame_size > 0)
        fprintf(output, "    addi r15, r15, %u\n", frame_size);
    fprintf(output, "    ret\n");
    current_function_epilogue = NULL;
    return 1;
}

static void emit_expression_strings(FILE *output, const expression_t *expression)
{
    size_t i;
    for (i = 0; i < expression->node_count; ++i) {
        const expression_node_t *node = &expression->nodes[i];
        size_t j;
        if (node->kind != EXPR_STRING)
            continue;
        fprintf(output, ".Lstr_%u:\n", node->string_id);
        j = 0;
        for (;;) {
            size_t column = 0;
            fprintf(output, "    .byte ");
            while (column < 8) {
                unsigned value;
                if (node->string_value[j] == '\0')
                    value = 0;
                else
                    value = (unsigned)(unsigned char)node->string_value[j];

                if (column)
                    fprintf(output, ", ");
                fprintf(output, "%u", value);
                ++column;

                if (node->string_value[j] == '\0')
                    break;
                ++j;
            }
            fprintf(output, "\n");
            if (node->string_value[j] == '\0')
                break;
        }
    }
}

static void emit_program_strings(FILE *output, const program_t *program)
{
    size_t i;
    for (i = 0; i < program->local_count; ++i)
        if (program->locals[i].has_initializer)
            emit_expression_strings(output, &program->locals[i].initializer);

    for (i = 0; i < program->statement_count; ++i) {
        const statement_t *st = &program->statements[i];
        emit_expression_strings(output, &st->expression);
        emit_expression_strings(output, &st->for_init);
        emit_expression_strings(output, &st->for_update);
    }

    if (program->has_explicit_return)
        emit_expression_strings(output, &program->return_expression);
}

static int expression_uses_call_name(
    const expression_t *expression,
    const char *name)
{
    size_t i;
    for (i = 0; i < expression->node_count; ++i) {
        if (expression->nodes[i].kind == EXPR_CALL &&
            strcmp(expression->nodes[i].call_name, name) == 0)
            return 1;
    }
    return 0;
}

static int program_uses_call_name(const program_t *program, const char *name)
{
    size_t i;
    for (i = 0; i < program->local_count; ++i)
        if (program->locals[i].has_initializer &&
            expression_uses_call_name(&program->locals[i].initializer, name))
            return 1;

    for (i = 0; i < program->statement_count; ++i) {
        const statement_t *st = &program->statements[i];
        if (expression_uses_call_name(&st->expression, name) ||
            expression_uses_call_name(&st->for_init, name) ||
            expression_uses_call_name(&st->for_update, name))
            return 1;
    }

    return program->has_explicit_return &&
           expression_uses_call_name(&program->return_expression, name);
}

static int unit_uses_call_name(const translation_unit_t *unit, const char *name)
{
    size_t i;
    for (i = 0; i < unit->function_count; ++i)
        if (program_uses_call_name(&unit->functions[i], name))
            return 1;
    return 0;
}

static int emit_assembly(const char *path, const translation_unit_t *unit)
{
    FILE *output = fopen(path, "w");
    size_t i;
    unsigned label_counter = 0;
    if (!output) {
        fprintf(stderr, "t32-cc: cannot create %s: %s\n", path, strerror(errno));
        return 0;
    }
    fprintf(output,
        "; generated by t32-cc %s\n"
        "; Stage 17 ABI 0.1 translation unit\n\n"
        ".section .text\n"
        ".global main\n",
        T32CC_VERSION);

    if (unit_uses_call_name(unit, "putchar"))
        fprintf(output, ".extern putchar\n");
    if (unit_uses_call_name(unit, "puts"))
        fprintf(output, ".extern puts\n");
    fprintf(output, "\n");

    for (i = 0; i < unit->function_count; ++i) {
        if (i > 0)
            fprintf(output, "\n");
        if (!emit_function(output, &unit->functions[i], &label_counter)) {
            fclose(output); remove(path); return 0;
        }
    }

    if (next_string_id > 0) {
        fprintf(output, "\n.section .data\n");
        for (i = 0; i < unit->function_count; ++i)
            emit_program_strings(output, &unit->functions[i]);
    }

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
    static translation_unit_t unit;
    int success = 0;

    if (argc == 1) { print_usage(stderr, argv[0]); return EXIT_FAILURE; }
    if (!parse_options(argc, argv, &options, default_output, sizeof(default_output))) return EXIT_FAILURE;

    source = read_entire_file(options.input_path, &source_length);
    if (!source) return EXIT_FAILURE;
    memset(&parser, 0, sizeof(parser));
    next_string_id = 0;
    parser.lexer.path = options.input_path;
    parser.lexer.source = source;
    parser.lexer.length = source_length;
    parser.lexer.line = 1;
    parser.lexer.column = 1;
    if (!parse_translation_unit(&parser, &unit)) { free(source); return EXIT_FAILURE; }

    if (options.mode == MODE_ASSEMBLY) assembly_path = options.output_path;
    else { make_temp_path(options.output_path, ".t32cc.s", temp_assembly, sizeof(temp_assembly)); assembly_path = temp_assembly; }

    if (options.verbose) printf("compile: %s -> %s\n", options.input_path, assembly_path);
    if (!emit_assembly(assembly_path, &unit)) goto done;

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

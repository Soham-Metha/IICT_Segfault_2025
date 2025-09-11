#include <Frontend/Layer_Tokens.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#define MAX_CACHED_TOKENS 2
Token tok_cache[MAX_CACHED_TOKENS];
int cachedCnt = 0;

static bool isName(char x)
{
    return isalnum(x) || x == '_';
}

static bool isNumber(char x)
{
    return isalnum(x) || x == '.' || x == '-';
}

const char* token_get_name(TokenType type)
{
    switch (type) {
    case TOKEN_TYPE_STR:
        return "token String literal";
    case TOKEN_TYPE_CHAR:
        return "token Character literal";
    case TOKEN_TYPE_NUMBER:
        return "token Numeric value";
    case TOKEN_TYPE_NAME:
        return "token Name value";
    case TOKEN_TYPE_OPEN_PAREN:
        return "token Open parenthesis";
    case TOKEN_TYPE_CLOSING_PAREN:
        return "token Closing parenthesis";
    case TOKEN_TYPE_OPEN_CURLY:
        return "token Open curly brace";
    case TOKEN_TYPE_CLOSING_CURLY:
        return "token Closing curly brace";
    case TOKEN_TYPE_COMMA:
        return "token Comma";
    case TOKEN_TYPE_COLON:
        return "token Colon";
    case TOKEN_TYPE_EQUAL:
        return "token Assignment operator";
    case TOKEN_TYPE_EOL:
        return "token End of line reached!";
    case TOKEN_TYPE_THEN:
        return "token Conditional pattern match";
    case TOKEN_TYPE_REPEAT:
        return "token Conditional retetition";
    case TOKEN_TYPE_PLUS:
        return "Binary operator +";
    case TOKEN_TYPE_MINUS:
        return "Binary operator -";
    case TOKEN_TYPE_MULT:
        return "Binary operator *";
    case TOKEN_TYPE_LT:
        return "Binary operator <";
    case TOKEN_TYPE_GE:
        return "Binary operator >=";
    case TOKEN_TYPE_NE:
        return "Binary operator !=";
    case TOKEN_TYPE_AND:
        return "Binary operator &&";
    case TOKEN_TYPE_OR:
        return "Binary operator ||";
    case TOKEN_TYPE_EQEQ:
        return "Binary operator ==";
    case TOKEN_TYPE_STATEMENT_END:
        return "Statement ended with";
    case TOKEN_TYPE_CNT:
    default:
        {
            assert(0 && "token_get_name: unreachable");
            exit(1);
        }
    }
}

Token token_expect_next(Line_Context* ctx, TokenType expected)
{
    update_indent(1);
    Token token = token_peek_next(ctx);
    assert(cachedCnt >= 0);
    log_to_ctx(ctx, LOG_FORMAT("[TOKEN CHECK]", "[TOKN]", "Expected: '%s'", token_get_name(expected), token_get_name(token.type)));

    if (!token_consume(ctx)) {
        print(ctx, WIN_STDERR, ": ERROR: expected token `%s`\n", token_get_name(expected));
        exit(1);
    }

    if (token.type != expected) {
        print(ctx, WIN_STDERR, ": ERROR: expected token `%s`, but got `%s`\n", token_get_name(expected), token_get_name(token.type));
        exit(1);
    }
    update_indent(-1);

    return token;
}

typedef struct TokenTextLUT {
    String txt;
    TokenType type;
} TokenTextLUT;

static TokenTextLUT tokenTextLUT[] = {
    {        .type = TOKEN_TYPE_REPEAT, .txt = { .data = "repeat", .len = 6 } },
    {          .type = TOKEN_TYPE_THEN,   .txt = { .data = "then", .len = 4 } },
    {        .type = TOKEN_TYPE_REPEAT,    .txt = { .data = "<->", .len = 3 } },
    {          .type = TOKEN_TYPE_THEN,     .txt = { .data = "->", .len = 2 } },
    {          .type = TOKEN_TYPE_EQEQ,     .txt = { .data = "==", .len = 2 } },
    {           .type = TOKEN_TYPE_AND,     .txt = { .data = "&&", .len = 2 } },
    {            .type = TOKEN_TYPE_GE,     .txt = { .data = ">=", .len = 2 } },
    {            .type = TOKEN_TYPE_NE,     .txt = { .data = "!=", .len = 2 } },
    {            .type = TOKEN_TYPE_OR,     .txt = { .data = "||", .len = 2 } },

    {            .type = TOKEN_TYPE_LT,      .txt = { .data = "<", .len = 1 } },
    {          .type = TOKEN_TYPE_MULT,      .txt = { .data = "*", .len = 1 } },
    {          .type = TOKEN_TYPE_PLUS,      .txt = { .data = "+", .len = 1 } },
    {         .type = TOKEN_TYPE_MINUS,      .txt = { .data = "-", .len = 1 } },
    {         .type = TOKEN_TYPE_COMMA,      .txt = { .data = ",", .len = 1 } },
    {         .type = TOKEN_TYPE_COLON,      .txt = { .data = ":", .len = 1 } },
    {         .type = TOKEN_TYPE_EQUAL,      .txt = { .data = "=", .len = 1 } },
    {    .type = TOKEN_TYPE_OPEN_CURLY,      .txt = { .data = "{", .len = 1 } },
    {    .type = TOKEN_TYPE_OPEN_PAREN,      .txt = { .data = "(", .len = 1 } },
    { .type = TOKEN_TYPE_CLOSING_PAREN,      .txt = { .data = ")", .len = 1 } },
    { .type = TOKEN_TYPE_CLOSING_CURLY,      .txt = { .data = "}", .len = 1 } },
    { .type = TOKEN_TYPE_STATEMENT_END,      .txt = { .data = ";", .len = 1 } },
};

static const int tokensLUT_len = sizeof(tokenTextLUT) / sizeof(tokenTextLUT[0]);

Token token_peek_next(Line_Context* ctx)
{
    String* line = &ctx->line;
    while (cachedCnt < MAX_CACHED_TOKENS) {
        Token token = { 0 };
        (*line)     = trim(*line);

        if (line->len == 0) {
            break;
        }

        int old_cached_cnt = cachedCnt;
        for (int i = 0; i < tokensLUT_len; i++) {
            if (starts_with(*line, tokenTextLUT[i].txt)) {
                token.type           = tokenTextLUT[i].type;
                token.text           = split_str_by_len(line, tokenTextLUT[i].txt.len);
                tok_cache[cachedCnt] = token;
                cachedCnt++;
                break;
            }
        }
        if (old_cached_cnt != cachedCnt)
            continue;

        switch (line->data[0]) {
        case '"':
            {
                split_str_by_len(line, 1);     // discard opening "

                token.type   = TOKEN_TYPE_STR;
                size_t index = 0;
                if (!get_index_of(*line, '"', &index)) {
                    print(ctx, WIN_STDERR, "ERROR: Could not find closing \"\n");
                    exit(1);
                }
                token.text = split_str_by_len(line, index);

                split_str_by_len(line, 1);     // discard closing "
            }
            break;

        case '\'':
            {
                split_str_by_len(line, 1);     // discard opening '

                token.type   = TOKEN_TYPE_CHAR;
                size_t index = 0;
                if (!get_index_of(*line, '\'', &index)) {
                    print(ctx, WIN_STDERR, "ERROR: Could not find closing \'\n");
                    exit(1);
                }
                token.text = split_str_by_len(line, index);

                split_str_by_len(line, 1);     // discard closing '
            }
            break;

        default:
            {
                if (isalpha(line->data[0])) {
                    token.type = TOKEN_TYPE_NAME;
                    token.text = split_str_by_condition(line, isName);
                } else if (isdigit(line->data[0])) {
                    token.type = TOKEN_TYPE_NUMBER;
                    token.text = split_str_by_condition(line, isNumber);
                } else {
                    print(ctx, WIN_STDERR, "Unknown token starting with '%c'\n", line->data[0]);
                    exit(1);
                }
            }
        }
        tok_cache[cachedCnt] = token;
        cachedCnt++;
    }

    return tok_cache[0];
}

bool token_consume(Line_Context* ctx)
{
    (void)ctx;
    if (cachedCnt > 0) {
        update_indent(1);
        log_to_ctx(ctx, LOG_FORMAT("", "[TOKN]", "<%s '%.*s'>", token_get_name(tok_cache[0].type), Str_Fmt(tok_cache[0].text)));
        update_indent(-1);
        for (int i = 0; i < cachedCnt; i++) {
            tok_cache[i] = tok_cache[i + 1];
        }
        cachedCnt--;
        return true;
    }
    return false;
}

Token token_peek_next_next(Line_Context* ctx)
{
    (void)token_peek_next(ctx);
    assert(cachedCnt > 1);
    return tok_cache[1];
}
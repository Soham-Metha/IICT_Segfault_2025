#ifndef EXPR_LAYER_FRONTEND
#define EXPR_LAYER_FRONTEND
#include <Utils/strings.h>

enum TokenType {
	TOKEN_TYPE_STR,
	TOKEN_TYPE_CHAR,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_NAME,
	TOKEN_TYPE_OPEN_PAREN,
	TOKEN_TYPE_CLOSING_PAREN,
	TOKEN_TYPE_OPEN_CURLY,
	TOKEN_TYPE_CLOSING_CURLY,
	TOKEN_TYPE_STATEMENT_END,
	TOKEN_TYPE_COMMA,
	TOKEN_TYPE_COLON,
	TOKEN_TYPE_EQUAL,
	TOKEN_TYPE_EOL,
};

typedef struct Token Token;
typedef enum TokenType TokenType;
struct Token {
	TokenType type;
	String text;
};

Token token_fetch_next(String *line);
const char *token_get_name(TokenType type);
Token token_expect_next(String *line, TokenType expected);
bool discard_cached_token();

#endif
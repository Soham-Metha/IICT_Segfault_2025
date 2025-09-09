#ifndef TOKN_LAYER_FRONTEND
#define TOKN_LAYER_FRONTEND
#include <Utils/strings.h>
#include <Frontend/Layer_Line.h>

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
	TOKEN_TYPE_THEN,
	TOKEN_TYPE_REPEAT,
	TOKEN_TYPE_COMMA,
	TOKEN_TYPE_COLON,
	TOKEN_TYPE_EQUAL,
	TOKEN_TYPE_EOL,
	TOKEN_TYPE_PLUS,
	TOKEN_TYPE_MINUS,
	TOKEN_TYPE_MULT,
	TOKEN_TYPE_LT,
	TOKEN_TYPE_GE,
	TOKEN_TYPE_NE,
	TOKEN_TYPE_AND,
	TOKEN_TYPE_OR,
	TOKEN_TYPE_EQEQ,
	TOKEN_TYPE_CNT,
};

typedef struct Token Token;
typedef enum TokenType TokenType;
typedef struct Line_Context Line_Context;
struct Token {
	TokenType type;
	String text;
};

Token token_peek_next(Line_Context *ctx);
const char *token_get_name(TokenType type);
Token token_expect_next(Line_Context *ctx, TokenType expected);
bool token_consume(Line_Context *ctx);
Token token_peek_next_next(Line_Context *ctx);

#endif
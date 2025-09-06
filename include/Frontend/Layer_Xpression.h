#ifndef EXPR_LAYER_FRONTEND
#define EXPR_LAYER_FRONTEND
#include <Utils/strings.h>
#include <Frontend/Layer_Line.h>

enum ExprType {
	EXPR_TYPE_STR,
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
};

typedef struct Token Token;
typedef enum ExprType ExprType;
typedef struct Line_Context Line_Context;
struct Token {
	ExprType type;
	String text;
};

Token token_peek_next(Line_Context *ctx);
const char *token_get_name(ExprType type);
Token token_expect_next(Line_Context *ctx, ExprType expected);
bool token_consume(Line_Context *ctx);

#endif
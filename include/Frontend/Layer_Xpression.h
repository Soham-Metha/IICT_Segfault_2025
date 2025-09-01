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
	TOKEN_TYPE_COMMA
};

struct Token {
	TokenType type;
	String text;
};

void getNextToken(String line);

#endif
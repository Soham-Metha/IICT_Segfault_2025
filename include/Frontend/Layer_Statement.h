#ifndef STMT_LAYER_FRONTEND
#define STMT_LAYER_FRONTEND
#include <Frontend/Layer_Line.h>
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
	TOKEN_TYPE_FUNC,
	TOKEN_TYPE_COMMA
};

typedef struct Token Token;
typedef enum TokenType TokenType;
struct Token {
	TokenType type;
	String text;
};

Line_View getNextStmt(String line);

#endif
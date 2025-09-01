#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>
#include <assert.h>

const char *getTokenName(TokenType type)
{
	switch (type) {
	case TOKEN_TYPE_STR:
		return "string";
	case TOKEN_TYPE_CHAR:
		return "character";
	case TOKEN_TYPE_NUMBER:
		return "number";
	case TOKEN_TYPE_NAME:
		return "name";
	case TOKEN_TYPE_OPEN_PAREN:
		return "open paren";
	case TOKEN_TYPE_CLOSING_PAREN:
		return "closing paren";
	case TOKEN_TYPE_OPEN_CURLY:
		return "open curly";
	case TOKEN_TYPE_CLOSING_CURLY:
		return "closing curly";
	case TOKEN_TYPE_COMMA:
		return "comma";
	case TOKEN_TYPE_FUNC:
		return "func";
	default: {
		assert(0 && "getTokenName: unreachable");
		exit(1);
	}
	}
}

Line_View getNextStmt(String line)
{
	Line_View result = { 0 };
	(void)line;
	Token tok;
	while (line.len > 0) {
		line = trim(line);
		tok = getNextToken(&line);
		print(WIN_STDOUT, "\n[STMT] identified token '%.*s' as '%s'",
		      tok.text.len, tok.text.data, getTokenName(tok.type));
	}
	return result;
}
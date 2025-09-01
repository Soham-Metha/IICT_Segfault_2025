#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

Stmt getNextStmt(String line)
{
	Stmt result = { 0 };
	line = trim(line);
	Token tok = getNextToken(&line);
	// while (line.len > 0) {
	// 	print(WIN_STDOUT, "\n[STMT] identified token '%.*s' as '%s'",
	// 	      tok.text.len, tok.text.data, getTokenName(tok.type));
	// }

	switch (tok.type) {
	case TOKEN_TYPE_CHAR:
		tok = getNextToken(&line);
		discard_cached_token();

		if (tok.text.len != 1) {
			print(WIN_STDERR,
			      "ERROR: the length of char literal has to be exactly one\n");
			exit(1);
		}

		result.type = STMT_LIT_CHAR;
		result.value.as_char = tok.text.data[0];
		break;

	case TOKEN_TYPE_STR:
		result.type = STMT_LIT_STR;
		// result.value.as_str = ;
		assert(0);
		break;
	case TOKEN_TYPE_NAME:
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_OPEN_PAREN:
	case TOKEN_TYPE_OPEN_CURLY:
	case TOKEN_TYPE_STATEMENT_END:
	case TOKEN_TYPE_FUNC:
		assert(0);
		break;
	case TOKEN_TYPE_COMMA:
	case TOKEN_TYPE_CLOSING_PAREN:
	case TOKEN_TYPE_CLOSING_CURLY:
		print(WIN_STDERR, "ERROR: exprected a statement but found %s\n",
		      getTokenName(tok.type));
		exit(1);
		break;

	default:
		assert(false && ": unreachable");
		exit(1);
	}

	return result;
}
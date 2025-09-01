#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

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
	case TOKEN_TYPE_STATEMENT_END:
		return "statement end";
	default: {
		assert(0 && "getTokenName: unreachable");
		exit(1);
	}
	}
}

Stmt getNextStmt(String line)
{
	line = trim(line);
	Token tok = getNextToken(&line);
	while (line.len > 0) {
		print(WIN_STDOUT, "\n[STMT] identified token '%.*s' as '%s'",
		      tok.text.len, tok.text.data, getTokenName(tok.type));
	}

	Stmt result = { 0 };

	switch (tok.type) {
	case TOKEN_TYPE_STR:
		result.type = STMT_LIT_STR;
		result.value.as_str =
			ParseStrFromSasmTokens(tokenizer, location);
		break;

	case TOKEN_TYPE_CHAR:
		discard_cached_token();

		if (tok.text.len != 1) {
			print(WIN_STDERR,
			      "ERROR: the length of char literal has to be exactly one\n");
			exit(1);
		}

		result.type = STMT_LIT_CHAR;
		result.value.as_char = tok.text.data[0];
		break;

	case TOKEN_TYPE_NAME:
		discard_cached_token();

		Token next = { 0 };
		if (fetchCachedSasmTokenFromSasmTokenizer(tokenizer, &next,
							  location) &&
		    next.type == TOKEN_TYPE_OPEN_PAREN) {
			result.type = STMT_FUNCALL;
			result.value.as_funcall =
				allocateRegion(region, sizeof(Funcall));
			result.value.as_funcall->name = tok.text;
			result.value.as_funcall->args =
				parseFuncallArgs(region, tokenizer, location);
		} else {
			result.value.as_var = tok.text;
			result.type = STMT_VARIABLE;
		}
		break;

	case TOKEN_TYPE_NUMBER:
		return parseNumFromSasmTokens(region, tokenizer, location);

	case TOKEN_TYPE_OPEN_PAREN:
		discard_cached_token();
		Stmt stmt = getNextStmt(line);

		if (!moveSasmTokenizerToNextToken(tokenizer, &tok, location) ||
		    tok.type != TOKEN_TYPE_CLOSING_PAREN) {
			print(WIN_STDERR, "ERROR: Expected '%s'",
			      getTokenName(TOKEN_TYPE_CLOSING_PAREN));
			exit(1);
		}
		return stmt;
		break;
	case TOKEN_TYPE_COMMA:
	case TOKEN_TYPE_CLOSING_PAREN:
		print(WIN_STDERR, "ERROR: exprected a statement but found %s\n",
		      getTokenName(tok.type));
		exit(1);
		break;

	default:
		assert(false && "parsePrimaryOfSasmTokens: unreachable");
		exit(1);
	}

	return result;
}
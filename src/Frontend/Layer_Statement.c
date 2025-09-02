#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

// ------------------------------ INDIVIDUAL TOKEN HANDLERS ------------------------------
static inline Stmt __TOKEN_TYPE_CHAR(Token tok)
{
	if (tok.text.len != 1) {
		print(WIN_STDERR,
		      "ERROR: the length of char literal has to be exactly one\n");
		exit(1);
	}

	Stmt result = { 0 };
	result.type = STMT_LIT_CHAR;
	result.value.as_char = tok.text.data[0];

	print(WIN_STDOUT, "\n[STMT] identified '%c'(%s) as a char literal",
	      tok.text.data[0], getTokenName(tok.type));

	discard_cached_token();
	return result;
}

static inline Stmt __TOKEN_TYPE_STR(Token tok)
{
	Stmt result = { 0 };
	result.type = STMT_LIT_STR;
	result.value.as_str = tok.text;

	print(WIN_STDOUT, "\n[STMT] identified '%.*s'(%s) as a str literal",
	      tok.text.len, tok.text.data, getTokenName(tok.type));

	discard_cached_token();
	return result;
}

static inline Stmt __TOKEN_TYPE_NAME(Token tok, String *line)
{
	discard_cached_token();
	Token next = getNextToken(line);

	Stmt result = { 0 };
	if (next.type == TOKEN_TYPE_OPEN_PAREN) {
		result.type = STMT_FUNCALL;
		result.value.as_funcall = malloc(sizeof(Funcall));
		result.value.as_funcall->name = tok.text;
		// result.value.as_funcall->args = parseFuncallArgs(); // UNIMPLEMENTED!

		print(WIN_STDOUT,
		      "\n[STMT] identified '%.*s'(%s) as a function call",
		      tok.text.len, tok.text.data, getTokenName(tok.type));
	} else {
		result.value.as_var = tok.text;
		result.type = STMT_VARIABLE;

		print(WIN_STDOUT,
		      "\n[STMT] identified '%.*s'(%s) as a variable name",
		      tok.text.len, tok.text.data, getTokenName(tok.type));
	}
	return result;
}

static inline Stmt __TOKEN_TYPE_FUNC(Token tok, String *line)
{
	discard_cached_token();
	Token next = getNextToken(line);
	if (next.type != TOKEN_TYPE_NAME) {
		print(WIN_STDERR,
		      "ERROR: expected a function name but found %s\n",
		      getTokenName(tok.type));
		exit(1);
	}

	print(WIN_STDOUT,
	      "\n[STMT] identified '%.*s %.*s'(%s) as a function call declaration",
	      tok.text.len, tok.text.data, next.text.len, next.text.data,
	      getTokenName(tok.type));

	discard_cached_token();
	next = getNextToken(line);
	if (next.type != TOKEN_TYPE_OPEN_PAREN) {
		print(WIN_STDERR,
		      "ERROR: expected function arg list but found %s\n",
		      getTokenName(tok.type));
		exit(1);
	}

	Stmt result = { 0 };
	result.type = STMT_FUNCALL_DECLARATION;
	result.value.as_funcall = malloc(sizeof(Funcall));
	result.value.as_funcall->name = tok.text;
	// result.value.as_funcall->args = parseFuncallArgs(); // UNIMPLEMENTED!
	return result;
}

static inline Stmt __TOKEN_TYPE_OPEN_CURLY(Token tok)
{
	Stmt result = { 0 };
	result.type = STMT_BLOCK_START;

	print(WIN_STDOUT,
	      "\n[STMT] identified '%.*s'(%s) as a code block start",
	      tok.text.len, tok.text.data, getTokenName(tok.type));

	discard_cached_token();
	return result;
}

static inline Stmt __TOKEN_TYPE_CLOSING_CURLY(Token tok)
{
	Stmt result = { 0 };
	result.type = STMT_BLOCK_END;

	print(WIN_STDOUT, "\n[STMT] identified '%.*s'(%s) as a code block end",
	      tok.text.len, tok.text.data, getTokenName(tok.type));

	discard_cached_token();
	return result;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

Stmt getNextStmt(String *line)
{
	Token tok = getNextToken(line);

	switch (tok.type) {
	case TOKEN_TYPE_CHAR:
		return __TOKEN_TYPE_CHAR(tok);
	case TOKEN_TYPE_STR:
		return __TOKEN_TYPE_STR(tok);
	case TOKEN_TYPE_NAME:
		return __TOKEN_TYPE_NAME(tok, line);
	case TOKEN_TYPE_FUNC:
		return __TOKEN_TYPE_FUNC(tok, line);
	case TOKEN_TYPE_OPEN_CURLY:
		return __TOKEN_TYPE_OPEN_CURLY(tok);
	case TOKEN_TYPE_CLOSING_CURLY:
		return __TOKEN_TYPE_CLOSING_CURLY(tok);

	case TOKEN_TYPE_OPEN_PAREN:
	case TOKEN_TYPE_STATEMENT_END:
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_COMMA:
	case TOKEN_TYPE_CLOSING_PAREN:
		discard_cached_token();
		return (Stmt){ 0 };

	default:
		assert(false && ": unreachable");
		exit(1);
	}
}

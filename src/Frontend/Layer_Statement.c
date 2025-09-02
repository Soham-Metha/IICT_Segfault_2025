#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

FuncallArg *functions_parse_arglist(String *line)
{
	// split arguments from single comma seperated string to linked list of strings.
	Token token = token_fetch_next(line);

	if (!discard_cached_token() || token.type != TOKEN_TYPE_OPEN_PAREN) {
		print(WIN_STDERR, 
			" ERROR: expected %s\n", token_get_name(TOKEN_TYPE_OPEN_PAREN));
		exit(1);
	}

	token = token_fetch_next(line);
	if (token.type == TOKEN_TYPE_CLOSING_PAREN) {
		discard_cached_token();
		return NULL;
	}

	FuncallArg *first = NULL;
	FuncallArg *last  = NULL;

	do {
		FuncallArg *arg = malloc(sizeof(FuncallArg));
		arg->value 		= stmt_fetch_next(line);
		print(WIN_STDOUT,
		      "\n[STMT] identified '%.*s'(%s) as a function arg",
		      Str_Fmt(arg->value.value.as_str), token_get_name(token.type));

		if (first == NULL) {
			first 		= arg;
			last  		= arg;
		} else {
			last->next 	= arg;
			last 		= arg;
		}

		token = token_fetch_next(line);
		if (!discard_cached_token()) {
			print(WIN_STDERR, " ERROR: expected %s or %s\n",
			      token_get_name(TOKEN_TYPE_CLOSING_PAREN),
			      token_get_name(TOKEN_TYPE_COMMA));
			exit(1);
		}

	} while (token.type == TOKEN_TYPE_COMMA);

	if (token.type != TOKEN_TYPE_CLOSING_PAREN) {
		print(WIN_STDERR, " ERROR: expected %s\n",
		      token_get_name(TOKEN_TYPE_CLOSING_PAREN));
		exit(1);
	}

	return first;
}

// ------------------------------ INDIVIDUAL TOKEN HANDLERS ------------------------------

static inline Stmt __TOKEN_TYPE_CHAR(Token tok)
{
	Stmt result 		 = { 0 };
	result.type 		 = STMT_LIT_CHAR;
	result.value.as_char = tok.text.data[0];

	// print(WIN_STDOUT, "\n[STMT] identified '%c'(%s) as a char literal",
	//       tok.text.data[0], token_get_name(tok.type));

	if (tok.text.len != 1) {
		print(WIN_STDERR,
		      "ERROR: the length of char literal has to be exactly one\n");
		exit(1);
	}

	discard_cached_token();
	return result;
}

static inline Stmt __TOKEN_TYPE_STR(Token tok)
{
	Stmt result 		= { 0 };
	result.type 		= STMT_LIT_STR;
	result.value.as_str = tok.text;

	// print(WIN_STDOUT, "\n[STMT] identified '%.*s'(%s) as a str literal",
	//       tok.text.len, tok.text.data, token_get_name(tok.type));

	discard_cached_token();
	return result;
}

static inline Stmt __TOKEN_TYPE_OPEN_CURLY(Token tok)
{
	(void)tok;
	Stmt result 		 = { 0 };
	result.type 		 = STMT_BLOCK_START;
	result.value.as_char = '{';

	// print(WIN_STDOUT,
	//       "\n[STMT] identified '%.*s'(%s) as a code block start",
	//       tok.text.len, tok.text.data, token_get_name(tok.type));

	discard_cached_token();
	return result;
}

static inline Stmt __TOKEN_TYPE_CLOSING_CURLY(Token tok)
{
	(void)tok;
	Stmt result 		 = { 0 };
	result.type 		 = STMT_BLOCK_END;
	result.value.as_char = '}';

	// print(WIN_STDOUT, "\n[STMT] identified '%.*s'(%s) as a code block end",
	//       tok.text.len, tok.text.data, token_get_name(tok.type));

	discard_cached_token();
	return result;
}

static inline Stmt __TOKEN_TYPE_FUNC(Token tok, String *line)
{
	discard_cached_token();
	Stmt result = { 0 };
	Token name 	= token_expect_next(line, TOKEN_TYPE_NAME);

	// the 'func' keyword must be followed by a name, and comma
	// seperated argument list within braces.

	result.type 				  = STMT_FUNCALL_DECL;
	result.value.as_funcall 	  = malloc(sizeof(Funcall));
	result.value.as_funcall->name = tok.text;
	result.value.as_funcall->args = functions_parse_arglist(line);

	print(WIN_STDOUT,
		"\n[STMT] identified '%.*s %.*s'(%s) as a function call declaration",
		Str_Fmt(tok.text), Str_Fmt(name.text), token_get_name(tok.type));
	return result;
}

static inline Stmt __TOKEN_TYPE_NAME(Token tok, String *line)
{
	discard_cached_token();
	Stmt result = { 0 };
	Token next 	= token_fetch_next(line);

	// Both variables and functions are 'names', the only difference
	// between both is that a function name is followed by an open parenthesis
	// so, if the next token is a paren, then it's a function, else it's a variable!
	if (next.type == TOKEN_TYPE_OPEN_PAREN) {
		result.type 				  = STMT_FUNCALL;
		result.value.as_funcall 	  = malloc(sizeof(Funcall));
		result.value.as_funcall->name = tok.text;
		result.value.as_funcall->args = functions_parse_arglist(line);

		print(WIN_STDOUT,
		      "\n[STMT] identified '%.*s'(%s) as a function call",
		      tok.text.len, tok.text.data, token_get_name(tok.type));
	} else {
		result.value.as_var = tok.text;
		result.type 		= STMT_VARIABLE;

		print(WIN_STDOUT,
		      "\n[STMT] identified '%.*s'(%s) as a variable name",
		      tok.text.len, tok.text.data, token_get_name(tok.type));
	}
	return result;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

Stmt stmt_fetch_next(String *line)
{
	Token tok = token_fetch_next(line);

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
#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Frontend/Layer_File.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

FuncallArg *functions_parse_arglist(Line_Context *ctx)
{
	String* line = &ctx->line;
	// split arguments from single comma seperated string to linked list of strings.
	Token token = token_expect_next(line,TOKEN_TYPE_OPEN_PAREN);

	token = token_fetch_next(line);
	if (token.type == TOKEN_TYPE_CLOSING_PAREN) {
		discard_cached_token();
		return NULL;
	}

	FuncallArg *first = NULL;
	FuncallArg *last  = NULL;

	do {
		FuncallArg *arg = malloc(sizeof(FuncallArg));
		arg->value 		= stmt_fetch_next(ctx);
		arg->next		= NULL;
		// print(WIN_STDOUT,
		//       "\n[STMT] identified '%.*s'(%s) as a function arg",
		//       Str_Fmt(arg->value.value.as_str), token_get_name(token.type));

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

String parse_var_decl(String *line)
{
	(void)token_expect_next(line, TOKEN_TYPE_COLON);		// colon
	Token type 	= token_expect_next(line, TOKEN_TYPE_NAME); // datatype

	return type.text;
}

Var parse_var(String *line)
{
	Var res 	= { 0 };
	Token name 	= token_expect_next(line, TOKEN_TYPE_NAME); // var name
	res.name 	= name.text;
	res.mode 	= VAR_ACCS;

	Token next 	= token_fetch_next(line);
	if (next.type == TOKEN_TYPE_COLON) {
		res.type	 = parse_var_decl(line);
		res.mode 	|= VAR_DECL;
		next 		 = token_fetch_next(line);
	}
	if (next.type == TOKEN_TYPE_EQUAL) {
		res.defn_val = NULL;
		res.mode 	|= VAR_DEFN;
	}

	return res;
}

// ------------------------------ INDIVIDUAL TOKEN HANDLERS ------------------------------

static inline Stmt __TOKEN_TYPE_OPEN_CURLY(Token tok)
{
	(void)tok;
	Stmt result 		 = { 0 };
	result.type 		 = STMT_BLOCK_START;

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
	result.value.as_token= tok;

	// print(WIN_STDOUT, "\n[STMT] identified '%.*s'(%s) as a code block end",
	//       tok.text.len, tok.text.data, token_get_name(tok.type));

	discard_cached_token();
	return result;
}

static inline Stmt __TOKEN_TYPE_NAME(Token tok, Line_Context* ctx)
{
	String *line = &ctx->line;
	(void)tok;
	Stmt result = { 0 };
	result.type = STMT_VAR;
	result.value.as_var = parse_var(line);
	Token next = token_fetch_next(line);

	// Both variables and functions are 'names', the only difference
	// between both is that a function name is followed by an open parenthesis
	// so, if the next token is a paren, then it's a function, else it's a variable!
	if (next.type == TOKEN_TYPE_OPEN_PAREN) {
		result.type 				  = STMT_FUNCALL;
		result.value.as_funcall 	  = malloc(sizeof(Funcall));
		result.value.as_funcall->name = tok.text;
		result.value.as_funcall->args = functions_parse_arglist(ctx);

		log_to_ctx(ctx,
		      "\n[STMT] identified '%.*s'(%s) as a function call",
		      tok.text.len, tok.text.data, token_get_name(tok.type));
	}// else {
	// 	result.value.as_var = tok.text;
	// 	result.type 		= STMT_VARIABLE;

	// 	print(WIN_STDOUT,
	// 	      "\n[STMT] identified '%.*s'(%s) as a variable name",
	// 	      tok.text.len, tok.text.data, token_get_name(tok.type));
	// }

	// (void)token_expect_next(line,TOKEN_TYPE_STATEMENT_END);
	return result;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

Stmt stmt_fetch_next(Line_Context* ctx)
{
	String *line = &ctx->line;
	Token tok = token_fetch_next(line);

	switch (tok.type) {
	
	case TOKEN_TYPE_NAME:
		return __TOKEN_TYPE_NAME(tok, ctx);
	case TOKEN_TYPE_OPEN_CURLY:
		return __TOKEN_TYPE_OPEN_CURLY(tok);
	case TOKEN_TYPE_CLOSING_CURLY:
		return __TOKEN_TYPE_CLOSING_CURLY(tok);

	case TOKEN_TYPE_CHAR:
	case TOKEN_TYPE_STR:
	case TOKEN_TYPE_OPEN_PAREN:
	case TOKEN_TYPE_STATEMENT_END:
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_COMMA:
	case TOKEN_TYPE_COLON:
	case TOKEN_TYPE_EQUAL:
	case TOKEN_TYPE_EOL:
	case TOKEN_TYPE_CLOSING_PAREN: {
		Stmt result = { 0 };
		result.type = STMT_TOKEN;
		result.value.as_token = tok;
		discard_cached_token();
		return result;
	}
	default:
		assert(false && ": unreachable");
		exit(1);
	}
}
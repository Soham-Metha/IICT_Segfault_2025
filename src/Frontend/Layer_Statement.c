#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Frontend/Layer_File.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

FuncallArg *functions_parse_arglist(Line_Context *ctx)
{
	// split arguments from single comma seperated string to linked list of strings.
	Token token = token_expect_next(ctx,TOKEN_TYPE_OPEN_PAREN);
	update_indent(1);
	log_to_ctx(ctx,
		      LOG_FORMAT "- Arguments:",LOG_CTX("[IDENTIFICATION]","[STMT]"));

	update_indent(1);
	token = token_fetch_next(ctx);
	if (token.type == TOKEN_TYPE_CLOSING_PAREN) {
		discard_cached_token();
		log_to_ctx(ctx,
				  LOG_FORMAT " NO ARGS !",LOG_CTX("[IDENTIFICATION]","[STMT]"));
		update_indent(-2);
		return NULL;
	}

	FuncallArg *first = NULL;
	FuncallArg *last  = NULL;

	do {
		FuncallArg *arg = malloc(sizeof(FuncallArg));
		arg->value 		= stmt_fetch_next(ctx);
		arg->next		= NULL;

		if (first == NULL) {
			first 		= arg;
			last  		= arg;
		} else {
			last->next 	= arg;
			last 		= arg;
		}

		token = token_fetch_next(ctx);
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
	update_indent(-2);

	return first;
}

String parse_var_decl(Line_Context* ctx)
{
	(void)token_expect_next(ctx, TOKEN_TYPE_COLON);		// colon
	Token type 	= token_expect_next(ctx, TOKEN_TYPE_NAME); // datatype

	return type.text;
}

Var parse_var(Line_Context* ctx)
{
	Var res 	= { 0 };
	res.mode 	= VAR_ACCS;
	update_indent(1);

	Token next 	= token_fetch_next(ctx);
	if (next.type == TOKEN_TYPE_COLON) {

		res.type	 = parse_var_decl(ctx);
		res.mode 	|= VAR_DECL;
		next 		 = token_fetch_next(ctx);

	log_to_ctx(ctx, LOG_FORMAT "- declared type: '%.*s'", LOG_CTX("[IDENTIFICATION]","[STMT]"), res.type.len,
		res.type.data);
	}
	if (next.type == TOKEN_TYPE_EQUAL) {
		res.defn_val = NULL;
		res.mode 	|= VAR_DEFN;
		log_to_ctx(ctx, LOG_FORMAT "- definition follows (next statement is the definition) ", LOG_CTX("[IDENTIFICATION]","[STMT]"));
	}

	update_indent(-1);
	return res;
}

// ------------------------------ INDIVIDUAL TOKEN HANDLERS ------------------------------

static inline Stmt __TOKEN_TYPE_OPEN_CURLY(Token tok, Line_Context* ctx)
{
	(void)tok;
	Stmt result 		 = { 0 };
	result.type 		 = STMT_BLOCK_START;

	update_indent(2);
	log_to_ctx(ctx,
	      LOG_FORMAT, LOG_CTX("[BLOCK START]","[STMT]"));
	update_indent(-2);

	discard_cached_token();
	return result;
}

static inline Stmt __TOKEN_TYPE_CLOSING_CURLY(Token tok, Line_Context* ctx)
{
	(void)tok;
	Stmt result 		 = { 0 };
	result.type 		 = STMT_BLOCK_END;
	result.value.as_token= tok;

	update_indent(5);
	log_to_ctx(ctx,
		LOG_FORMAT, LOG_CTX("[BLOCK END]","[STMT]"));
	update_indent(-5);

	discard_cached_token();
	return result;
}

static inline Stmt __TOKEN_TYPE_NAME(Token tok, Line_Context* ctx)
{
	Stmt result = { 0 };
	discard_cached_token();
	Token next = token_fetch_next(ctx);

	if (next.type == TOKEN_TYPE_OPEN_PAREN) {
		log_to_ctx(ctx,
		      LOG_FORMAT "function call: '%.*s'", LOG_CTX("[IDENTIFICATION]","[STMT]"),
		      tok.text.len, tok.text.data, token_get_name(tok.type));
		result.type 				  = STMT_FUNCALL;
		result.value.as_funcall 	  = malloc(sizeof(Funcall));
		result.value.as_funcall->name = tok.text;
		result.value.as_funcall->args = functions_parse_arglist(ctx);

	} else {
		log_to_ctx(ctx, 
			  LOG_FORMAT "variable:      '%.*s'", LOG_CTX("[IDENTIFICATION]", "[STMT]"),
			  tok.text.len, tok.text.data);
		result.type                   = STMT_VAR;
		result.value.as_var           = parse_var(ctx);
		result.value.as_var.name      = tok.text;

	}
	// (void)token_expect_next(line,TOKEN_TYPE_STATEMENT_END);
	return result;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

Stmt stmt_fetch_next(Line_Context* ctx)
{
	Token tok = token_fetch_next(ctx);

	switch (tok.type) {
	
	case TOKEN_TYPE_NAME:
		return __TOKEN_TYPE_NAME(tok, ctx);
	case TOKEN_TYPE_OPEN_CURLY:
		return __TOKEN_TYPE_OPEN_CURLY(tok, ctx);
	case TOKEN_TYPE_CLOSING_CURLY:
		return __TOKEN_TYPE_CLOSING_CURLY(tok, ctx);

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
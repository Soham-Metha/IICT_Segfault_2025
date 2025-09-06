#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Frontend/Layer_File.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

FuncallArg *functions_parse_arglist(Line_Context *ctx)
{
	// split arguments from single comma seperated string to linked list of strings.
	Expr token = expr_expect_next(ctx, EXPR_TYPE_OPEN_PAREN);
	update_indent(1);
	log_to_ctx(ctx, LOG_FORMAT "- Arguments:",
		   LOG_CTX("[IDENTIFICATION]", "[STMT]"));

	update_indent(1);
	token = expr_peek_next(ctx);
	if (token.type == EXPR_TYPE_CLOSING_PAREN) {
		token_consume(ctx);
		log_to_ctx(ctx, LOG_FORMAT " NO ARGS !",
			   LOG_CTX("[IDENTIFICATION]", "[STMT]"));
		update_indent(-2);
		return NULL;
	}

	FuncallArg *first = NULL;
	FuncallArg *last = NULL;

	do {
		FuncallArg *arg = region_allocate(sizeof(FuncallArg));
		arg->statement = stmt_fetch_next(ctx);
		arg->next = NULL;

		if (first == NULL) {
			first = arg;
			last = arg;
		} else {
			last->next = arg;
			last = arg;
		}

		token = expr_peek_next(ctx);
		if (!token_consume(ctx)) {
			print(ctx, WIN_STDERR, " ERROR: expected %s or %s\n",
			      expr_get_name(EXPR_TYPE_CLOSING_PAREN),
			      expr_get_name(EXPR_TYPE_COMMA));
			exit(1);
		}

	} while (token.type == EXPR_TYPE_COMMA);

	if (token.type != EXPR_TYPE_CLOSING_PAREN) {
		print(ctx, WIN_STDERR, " ERROR: expected %s\n",
		      expr_get_name(EXPR_TYPE_CLOSING_PAREN));
		exit(1);
	}
	update_indent(-2);

	return first;
}

void parse_var_decl(Line_Context *ctx, Var *out)
{
	(void)expr_expect_next(ctx, EXPR_TYPE_COLON); // colon
	Expr type = expr_expect_next(ctx, EXPR_TYPE_NAME); // datatype

	out->type = type.text;
	if (compare_str(type.text, STR("func")) ||
	    compare_str(type.text, STR("struct"))) {
		out->arglist = functions_parse_arglist(ctx);
	}
}

Var parse_var(Line_Context *ctx)
{
	Var res = { 0 };
	res.mode = VAR_ACCS;

	Expr next = expr_peek_next(ctx);
	if (next.type == EXPR_TYPE_COLON) {
		parse_var_decl(ctx, &res);
		res.mode |= VAR_DECL;
		next = expr_peek_next(ctx);

		update_indent(1);
		log_to_ctx(ctx, LOG_FORMAT "- declared type: '%.*s'",
			   LOG_CTX("[IDENTIFICATION]", "[STMT]"), res.type.len,
			   res.type.data);
		update_indent(-1);
	}
	if (next.type == EXPR_TYPE_EQUAL) {
		token_consume(ctx);
		res.mode |= VAR_DEFN;
		// log_to_ctx(ctx, LOG_FORMAT "---------------DEFINITION START---------------", LOG_CTX("[IDENTIFICATION]","[STMT]"));
	}

	return res;
}

StmtConditional get_stmt_conditional(Expr tok, Line_Context *ctx)
{
	(void)tok;
	StmtConditional res = { 0 };

	expr_expect_next(ctx, EXPR_TYPE_OPEN_PAREN);
	res.cond = expr_expect_next(ctx, EXPR_TYPE_NAME);
	expr_expect_next(ctx, EXPR_TYPE_CLOSING_PAREN);

	Expr next = expr_peek_next(ctx);

	if (next.type == EXPR_TYPE_THEN) {
		res.repeat = false;
	} else if (next.type == EXPR_TYPE_REPEAT) {
		res.repeat = true;
	} else {
		assert(0 && "EXPECTED THEN OR REPEAT");
	}

	return res;
}

// ------------------------------ INDIVIDUAL TOKEN HANDLERS ------------------------------

static inline Stmt __TOKEN_TYPE_OPEN_PAREN(Expr tok, Line_Context *ctx)
{
	Stmt res = { 0 };
	res.type = STMT_CONDITIONAL;

	res.as.cond = get_stmt_conditional(tok, ctx);

	return res;
}

static inline Stmt __TOKEN_TYPE_OPEN_CURLY(Expr tok, Line_Context *ctx)
{
	(void)tok;
	Stmt result = { 0 };
	result.type = STMT_BLOCK_START;

	log_to_ctx(ctx, LOG_FORMAT, LOG_CTX("[BLOCK START]", "[STMT]"));

	token_consume(ctx);
	return result;
}

static inline Stmt __TOKEN_TYPE_CLOSING_CURLY(Expr tok, Line_Context *ctx)
{
	(void)tok;
	Stmt result = { 0 };
	result.type = STMT_BLOCK_END;
	// result.as.token= tok;

	log_to_ctx(ctx, LOG_FORMAT, LOG_CTX("[BLOCK END]", "[STMT]"));

	token_consume(ctx);
	return result;
}

static inline Stmt __TOKEN_TYPE_NAME(Expr tok, Line_Context *ctx)
{
	Stmt result = { 0 };
	token_consume(ctx);
	Expr next = expr_peek_next(ctx);

	if (compare_str(tok.text, STR("match"))) {
		log_to_ctx(ctx, LOG_FORMAT "pattern match: '%.*s'",
			   LOG_CTX("[IDENTIFICATION]", "[STMT]"),
			   Str_Fmt(tok.text));
		result.type = STMT_MATCH;

	} else if (next.type == EXPR_TYPE_OPEN_PAREN) {
		log_to_ctx(ctx, LOG_FORMAT "function call: '%.*s'",
			   LOG_CTX("[IDENTIFICATION]", "[STMT]"),
			   Str_Fmt(tok.text));
		result.type = STMT_FUNCALL;
		result.as.funcall = region_allocate(sizeof(Funcall));
		result.as.funcall->name = tok.text;
		result.as.funcall->args = functions_parse_arglist(ctx);

	} else {
		log_to_ctx(ctx, LOG_FORMAT "variable: '%.*s'",
			   LOG_CTX("[IDENTIFICATION]", "[STMT]"),
			   Str_Fmt(tok.text));
		result.type = STMT_VAR;
		result.as.var = parse_var(ctx);
		result.as.var.name = tok.text;
	}
	// (void)token_expect_next(ctx,TOKEN_TYPE_STATEMENT_END);
	return result;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

Stmt stmt_fetch_next(Line_Context *ctx)
{
	assert(ctx);
	Expr tok = expr_peek_next(ctx);
	// log_to_ctx(ctx, LOG_FORMAT "Checking the first token of the statement to identify statement type, found:", LOG_CTX("[IDENTIFY]","[STMT]"));
	switch (tok.type) {
	case EXPR_TYPE_NAME:
		return __TOKEN_TYPE_NAME(tok, ctx);
	case EXPR_TYPE_OPEN_CURLY:
		return __TOKEN_TYPE_OPEN_CURLY(tok, ctx);
	case EXPR_TYPE_CLOSING_CURLY:
		return __TOKEN_TYPE_CLOSING_CURLY(tok, ctx);
	case EXPR_TYPE_OPEN_PAREN:
		return __TOKEN_TYPE_OPEN_PAREN(tok, ctx);

	case EXPR_TYPE_NUMBER:
	case EXPR_TYPE_CHAR:
	case EXPR_TYPE_STR:
	case EXPR_TYPE_THEN:
	case EXPR_TYPE_REPEAT:
	case EXPR_TYPE_STATEMENT_END: {
		Stmt result = { 0 };
		result.type = STMT_TOKEN;
		result.as.token = tok;
		token_consume(ctx);
		return result;
	}
	case EXPR_TYPE_CLOSING_PAREN:
	case EXPR_TYPE_COMMA:
	case EXPR_TYPE_COLON:
	case EXPR_TYPE_EQUAL:
	case EXPR_TYPE_EOL:
	default:
		token_consume(ctx);
		print(ctx, WIN_STDERR, "Unexpected token found!");
		exit(1);
	}

	expr_expect_next(ctx, EXPR_TYPE_STATEMENT_END);
}
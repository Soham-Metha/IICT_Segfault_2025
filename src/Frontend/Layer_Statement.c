#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Frontend/Layer_File.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

void parse_var_decl(Line_Context *ctx, Var *out)
{
	(void)expr_expect_next(ctx, EXPR_TYPE_COLON); // colon
	Expr type = expr_expect_next(ctx, EXPR_TYPE_VAR); // datatype

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
		expr_consume(ctx);
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
	res.cond = expr_expect_next(ctx, EXPR_TYPE_VAR);
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

	expr_consume(ctx);
	return result;
}

static inline Stmt __TOKEN_TYPE_CLOSING_CURLY(Expr tok, Line_Context *ctx)
{
	(void)tok;
	Stmt result = { 0 };
	result.type = STMT_BLOCK_END;
	// result.as.token= tok;

	log_to_ctx(ctx, LOG_FORMAT, LOG_CTX("[BLOCK END]", "[STMT]"));

	expr_consume(ctx);
	return result;
}
static inline Stmt __EXPR_VAR(Expr tok, Line_Context *ctx)
{
	Stmt result = { 0 };
	expr_consume(ctx);

	log_to_ctx(ctx, LOG_FORMAT "variable: '%.*s'",
		   LOG_CTX("[IDENTIFICATION]", "[STMT]"), Str_Fmt(tok.text));
	result.type = STMT_VAR;
	result.as.var = parse_var(ctx);
	result.as.var.name = tok.text;

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
	case EXPR_TYPE_VAR:
		return __EXPR_VAR(tok, ctx);
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
		expr_consume(ctx);
		return result;
	}
	case EXPR_TYPE_TOKEN:
	case EXPR_TYPE_CLOSING_PAREN:
	case EXPR_TYPE_COMMA:
	case EXPR_TYPE_COLON:
	case EXPR_TYPE_EQUAL:
	case EXPR_TYPE_EOL:
	default:
		expr_consume(ctx);
		print(ctx, WIN_STDERR, "Unexpected expr found!");
		exit(1);
	}

	expr_expect_next(ctx, EXPR_TYPE_STATEMENT_END);
}
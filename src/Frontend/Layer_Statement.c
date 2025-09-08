#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>
#include <Utils/mem_manager.h>
#include <assert.h>

StmtConditional get_stmt_conditional(Line_Context *ctx)
{
	StmtConditional res = { 0 };

	res.cond = expr_parse(ctx);
	assert(res.cond.type == EXPR_TYPE_BIN_OPR ||
	       res.cond.type == EXPR_TYPE_BOOL ||
	       res.cond.type == EXPR_TYPE_NUMBER);

	Token next = token_peek_next(ctx);

	if (next.type == TOKEN_TYPE_THEN) {
		token_consume(ctx);
		res.repeat = false;
	} else if (next.type == TOKEN_TYPE_REPEAT) {
		token_consume(ctx);
		res.repeat = true;
	} else {
		assert(0 && "EXPECTED THEN OR REPEAT");
	}

	return res;
}

VarDecl stmt_parse_var_decl(Line_Context *ctx)
{
	VarDecl res = { 0 };
	res.name = token_expect_next(ctx, TOKEN_TYPE_NAME).text;
	(void)token_expect_next(ctx, TOKEN_TYPE_COLON);
	res.type = token_expect_next(ctx, TOKEN_TYPE_NAME).text;
	if (compare_str(res.type, STR("func"))) {
		res.args = parse_funcall_arglist(ctx);
	}
	Token nxt = token_peek_next(ctx);
	if (nxt.type == TOKEN_TYPE_EQUAL) {
		token_consume(ctx);
		res.has_init = true;
		res.init = region_allocate(sizeof(Stmt));
		*res.init = stmt_fetch_next(ctx);
	}
	return res;
}
VarDefn stmt_parse_var_defn(Line_Context *ctx)
{
	VarDefn res = { 0 };
	res.name = token_expect_next(ctx, TOKEN_TYPE_NAME).text;
	(void)token_expect_next(ctx, TOKEN_TYPE_EQUAL);

	res.val = region_allocate(sizeof(Stmt));
	*res.val = stmt_fetch_next(ctx);
	return res;
}

static inline Stmt __TOKEN_TYPE_NAME(Token tok, Line_Context *ctx)
{
	Stmt result = { 0 };

	Token next = token_peek_next_next(ctx);

	// if (compare_str(tok.text, STR("match"))) {
	// 	log_to_ctx(ctx, LOG_FORMAT "pattern match: '%.*s'",
	// 		   LOG_CTX("[IDENTIFICATION]", "[STMT]"),
	// 		   Str_Fmt(tok.text));
	// 	result.type = STMT_MATCH;

	// 	Stmt result = { 0 };
	// 	expr_consume(ctx);
	// }

	log_to_ctx(ctx, LOG_FORMAT("[IDENTIFICATION]", "[STMT]",
				   "variable: '%.*s'", Str_Fmt(tok.text)));

	if (next.type == TOKEN_TYPE_COLON) {
		result.type = STMT_VAR_DECL;
		result.as.var_decl = stmt_parse_var_decl(ctx);
		return result;
	} else if (next.type == TOKEN_TYPE_EQUAL) {
		result.type = STMT_VAR_DEFN;
		result.as.var_defn = stmt_parse_var_defn(ctx);
		return result;
	} else {
		result.type = STMT_EXPR;
		result.as.expr = expr_parse(ctx);
		return result;
	}
	// (void)token_expect_next(ctx,TOKEN_TYPE_STATEMENT_END);
	return result;
}

Stmt stmt_fetch_next(Line_Context *ctx)
{
	Stmt result = { 0 };
	assert(ctx);
	Token tok = token_peek_next(ctx);
	// log_to_ctx(ctx, LOG_FORMAT "Checking the first token of the statement to identify statement type, found:", LOG_CTX("[IDENTIFY]","[STMT]"));
	switch (tok.type) {
	case TOKEN_TYPE_NAME:
		return __TOKEN_TYPE_NAME(tok, ctx);
	case TOKEN_TYPE_OPEN_CURLY: {
		token_consume(ctx);
		result.type = STMT_BLOCK_START;
		result.as.block = NULL;
		return result;
	} break;
	case TOKEN_TYPE_CLOSING_CURLY: {
		token_consume(ctx);
		result.type = STMT_BLOCK_END;
		return result;
	} break;
	case TOKEN_TYPE_OPEN_PAREN: {
		result.type = STMT_CONDITIONAL;
		result.as.cond = get_stmt_conditional(ctx);
		return result;
	}
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_CHAR:
	case TOKEN_TYPE_STR:
	case TOKEN_TYPE_THEN:
	case TOKEN_TYPE_REPEAT: {
		result.type = STMT_EXPR;
		result.as.expr = expr_parse(ctx);
		token_expect_next(ctx, TOKEN_TYPE_STATEMENT_END);
		return result;
	}
	case TOKEN_TYPE_STATEMENT_END:
	case TOKEN_TYPE_CLOSING_PAREN:
	case TOKEN_TYPE_COMMA:
	case TOKEN_TYPE_COLON:
	case TOKEN_TYPE_EQUAL:
	case TOKEN_TYPE_EOL:
	default:
		token_consume(ctx);

		log_to_ctx(ctx, LOG_FORMAT("[IDENTIFICATION]", "[STMT]",
					   "%s %.*s", token_get_name(tok.type),
					   Str_Fmt(tok.text)));
		print(ctx, WIN_STDERR, "Unexpected token found!");
		exit(1);
	}

	token_expect_next(ctx, TOKEN_TYPE_STATEMENT_END);
}
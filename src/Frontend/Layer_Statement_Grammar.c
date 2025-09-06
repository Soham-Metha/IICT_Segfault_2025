#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>

VarDecl stmt_parse_var_decl(Line_Context* ctx);
VarDefn stmt_parse_var_defn(Line_Context* ctx);

static inline Stmt __TOKEN_TYPE_NAME(Token tok, Line_Context *ctx)
{
	Stmt result = { 0 };
	token_consume(ctx);
	Token next = token_peek_next(ctx);

	// if (compare_str(tok.text, STR("match"))) {
	// 	log_to_ctx(ctx, LOG_FORMAT "pattern match: '%.*s'",
	// 		   LOG_CTX("[IDENTIFICATION]", "[STMT]"),
	// 		   Str_Fmt(tok.text));
	// 	result.type = STMT_MATCH;

	// 	Stmt result = { 0 };
	// 	expr_consume(ctx);
	// }

	log_to_ctx(ctx, LOG_FORMAT "variable: '%.*s'",
		   LOG_CTX("[IDENTIFICATION]", "[STMT]"), Str_Fmt(tok.text));

    if (next.type == TOKEN_TYPE_COLON) {
        result.type = STMT_VAR_DECL;
        result.as.var_decl = stmt_parse_var_decl(ctx);
    } else if (next.type == TOKEN_TYPE_EQUAL) {
        result.type = STMT_VAR_DEFN;
        result.as.var_defn = stmt_parse_var_defn(ctx);
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
	case TOKEN_TYPE_OPEN_CURLY:
	case TOKEN_TYPE_CLOSING_CURLY:
	case TOKEN_TYPE_OPEN_PAREN:
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_CHAR:
	case TOKEN_TYPE_STR:
	case TOKEN_TYPE_THEN:
	case TOKEN_TYPE_REPEAT:
	case TOKEN_TYPE_STATEMENT_END: {
		result.type = STMT_EXPR;
		result.as.expr = expr_parse(ctx);
		token_expect_next(ctx, TOKEN_TYPE_STATEMENT_END);
		return result;
	}
	case TOKEN_TYPE_CLOSING_PAREN:
	case TOKEN_TYPE_COMMA:
	case TOKEN_TYPE_COLON:
	case TOKEN_TYPE_EQUAL:
	case TOKEN_TYPE_EOL:
	default:
		token_consume(ctx);
		print(ctx, WIN_STDERR, "Unexpected token found!");
		exit(1);
	}

	token_expect_next(ctx, TOKEN_TYPE_STATEMENT_END);
}
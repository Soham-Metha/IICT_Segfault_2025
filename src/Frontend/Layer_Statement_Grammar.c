#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>

Stmt stmt_fetch_next(Line_Context *ctx)
{
	assert(ctx);
	Token tok = token_peek_next(ctx);
	// log_to_ctx(ctx, LOG_FORMAT "Checking the first token of the statement to identify statement type, found:", LOG_CTX("[IDENTIFY]","[STMT]"));
	switch (tok.type) {
	case TOKEN_TYPE_NAME:
		// return __TOKEN_TYPE_NAME(tok, ctx);
	case TOKEN_TYPE_OPEN_CURLY:
		// return __TOKEN_TYPE_OPEN_CURLY(tok, ctx);
	case TOKEN_TYPE_CLOSING_CURLY:
		// return __TOKEN_TYPE_CLOSING_CURLY(tok, ctx);
	case TOKEN_TYPE_OPEN_PAREN:
		// return __TOKEN_TYPE_OPEN_PAREN(tok, ctx);

	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_CHAR:
	case TOKEN_TYPE_STR:
	case TOKEN_TYPE_THEN:
	case TOKEN_TYPE_REPEAT:
	case TOKEN_TYPE_STATEMENT_END: {
		Stmt result = { 0 };
		// result.type = STMT_TOKEN;
		// result.as.token = tok;
		token_consume(ctx);
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
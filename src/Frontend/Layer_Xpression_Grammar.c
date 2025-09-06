#include <Frontend/Layer_Xpression.h>
#include <assert.h>

Expr expr_parse(Line_Context *ctx)
{
	assert(0);
}

Expr expr_parse_with_precedence(Line_Context *ctx, BinOprPrec p)
{
	assert(0);
}

Expr expr_peek_next(Line_Context *ctx)
{
	Expr expr = { 0 };

	Token token = token_peek_next(ctx);

	switch (token.type) {
	case TOKEN_TYPE_NAME: {
		if (compare_str(token.text, STR("true"))) {
			token_consume(ctx);
			expr.type = EXPR_TYPE_BOOL;
			expr.as.boolean = true;
		} else if (compare_str(token.text, STR("false"))) {
			token_consume(ctx);
			expr.type = EXPR_TYPE_BOOL;
			expr.as.boolean = false;
		} else {
			token_consume(ctx);
			Token next = token_peek_next(ctx);
			if (next.type == TOKEN_TYPE_OPEN_PAREN) {
				expr.type = EXPR_TYPE_FUNCALL;
				expr.as.funcall = parse_expr_funcall(ctx);
			} else {
				expr.type = EXPR_TYPE_VAR;
				expr.as.var_nm = token.text;
			}
		}
	} break;
	case TOKEN_TYPE_STR: {
		token_consume(ctx);
		expr.type = EXPR_TYPE_STR;
		expr.as.str = token.text;
	}
	case TOKEN_TYPE_CHAR: {
		token_consume(ctx);
		expr.type = EXPR_TYPE_STR;
		expr.as.str = token.text;
	}
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_OPEN_PAREN:
	case TOKEN_TYPE_CLOSING_PAREN:
	case TOKEN_TYPE_OPEN_CURLY:
	case TOKEN_TYPE_CLOSING_CURLY:
	case TOKEN_TYPE_COMMA:
	case TOKEN_TYPE_COLON:
	case TOKEN_TYPE_EQUAL:
	case TOKEN_TYPE_EOL:
	case TOKEN_TYPE_THEN:
	case TOKEN_TYPE_REPEAT:
	case TOKEN_TYPE_STATEMENT_END: {
		expr.type = EXPR_TYPE_TOKEN;
		expr.as.token.text = token.text;
		break;
	}
	default: {
		assert(0 && "token_get_name: unreachable");
		exit(1);
	}
	}
	return expr;
}
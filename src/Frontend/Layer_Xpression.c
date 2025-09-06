#include <Frontend/Layer_Xpression.h>
#include <Frontend/Layer_Tokens.h>
#include <Wrapper/IO.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

Expr cache;
bool cachedExpr = false;

static bool isName(char x)
{
	return isalnum(x) || x == '_';
}

static bool isNumber(char x)
{
	return isalnum(x) || x == '.' || x == '-';
}

const char *expr_get_name(ExprType type)
{
	switch (type) {
	case EXPR_TYPE_STR: 			return "String literal";
	case EXPR_TYPE_CHAR: 			return "Character literal";
	case EXPR_TYPE_NUMBER: 		    return "Numeric value";
	case EXPR_TYPE_NAME: 			return "Name value";
	case EXPR_TYPE_OPEN_PAREN: 	    return "Open parenthesis";
	case EXPR_TYPE_CLOSING_PAREN: 	return "Closing parenthesis";
	case EXPR_TYPE_OPEN_CURLY: 	    return "Open curly brace";
	case EXPR_TYPE_CLOSING_CURLY: 	return "Closing curly brace";
	case EXPR_TYPE_COMMA: 			return "Comma";
	case EXPR_TYPE_COLON:			return "Colon";
	case EXPR_TYPE_EQUAL:			return "Assignment operator";
	case EXPR_TYPE_EOL:			    return "End of line reached!";
	case EXPR_TYPE_THEN:			return "Conditional pattern match";
	case EXPR_TYPE_REPEAT:			return "Conditional retetition";
	// case TOKEN_TYPE_FUNC: 			return "func";
	case EXPR_TYPE_STATEMENT_END: 	return "Statement ended with";
	default: {
		assert(0 && "token_get_name: unreachable");
		exit(1);
	}
	}
}

Expr expr_expect_next(Line_Context *ctx, ExprType expected)
{
	update_indent(1);
	Expr token = expr_peek_next(ctx);
	log_to_ctx(ctx, LOG_FORMAT "Expected: '%s'",
		   LOG_CTX("[TOKEN CHECK]", "[EXPR]"), expr_get_name(expected),
		   expr_get_name(token.type));

	if (!expr_consume(ctx)) {
		print(ctx, WIN_STDERR, ": ERROR: expected token `%s`\n",
		      expr_get_name(expected));
		exit(1);
	}

	if (token.type != expected) {
		print(ctx, WIN_STDERR,
		      ": ERROR: expected token `%s`, but got `%s`\n",
		      expr_get_name(expected), expr_get_name(token.type));
		exit(1);
	}
	update_indent(-1);

	return token;
}

Expr expr_peek_next(Line_Context *ctx)
{
	if (cachedExpr)
	return cache;
	Expr expr = {0};

	Token token = token_peek_next(ctx);
	switch (token.type) {

	}
	cache = expr;
	cachedExpr = true;
	return expr;
}

bool expr_consume(Line_Context *ctx)
{
	(void)ctx;
	if (cachedExpr) {
		update_indent(1);
		log_to_ctx(ctx, LOG_FORMAT "<%s '%.*s'>", LOG_CTX("", "[EXPR]"),
			   expr_get_name(cache.type), cache.text.len,
			   cache.text.data);
		update_indent(-1);
		cachedExpr = false;
		return true;
	}
	return false;
}
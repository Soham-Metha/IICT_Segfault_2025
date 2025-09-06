#include <Frontend/Layer_Xpression.h>
#include <Frontend/Layer_Tokens.h>
#include <Wrapper/IO.h>
#include <Utils/mem_manager.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

FuncallArg *functions_parse_arglist(Line_Context *ctx)
{
	// split arguments from single comma seperated string to linked list of strings.
	Token token = token_expect_next(ctx, TOKEN_TYPE_OPEN_PAREN);
	update_indent(1);
	log_to_ctx(ctx, LOG_FORMAT "- Arguments:",
		   LOG_CTX("[IDENTIFICATION]", "[STMT]"));

	update_indent(1);
	token = token_peek_next(ctx);
	if (token.type == TOKEN_TYPE_CLOSING_PAREN) {
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
		arg->expr = expr_peek_next(ctx);
		arg->next = NULL;

		if (first == NULL) {
			first = arg;
			last = arg;
		} else {
			last->next = arg;
			last = arg;
		}

		token = token_peek_next(ctx);
		if (!token_consume(ctx)) {
			print(ctx, WIN_STDERR, " ERROR: expected %s or %s\n",
			      token_get_name(TOKEN_TYPE_CLOSING_PAREN),
			      token_get_name(TOKEN_TYPE_COMMA));
			exit(1);
		}

	} while (token.type == TOKEN_TYPE_COMMA);

	if (token.type != TOKEN_TYPE_CLOSING_PAREN) {
		print(ctx, WIN_STDERR, " ERROR: expected %s\n",
		      token_get_name(TOKEN_TYPE_CLOSING_PAREN));
		exit(1);
	}
	update_indent(-2);

	return first;
}

static inline Expr __TOKEN_TYPE_NAME(Expr expr, Line_Context *ctx)
{
	Expr result = { 0 };
	expr_consume(ctx);
	Expr next = expr_peek_next(ctx);

	// if (compare_str(tok.text, STR("match"))) {
	// 	log_to_ctx(ctx, LOG_FORMAT "pattern match: '%.*s'",
	// 		   LOG_CTX("[IDENTIFICATION]", "[STMT]"),
	// 		   Str_Fmt(tok.text));
	// 	result.type = STMT_MATCH;

	if (next.type == EXPR_TYPE_OPEN_PAREN) {
		log_to_ctx(ctx, LOG_FORMAT "function call: '%.*s'",
			   LOG_CTX("[IDENTIFICATION]", "[STMT]"),
			   Str_Fmt(expr.as.token.text));
		result.type = EXPR_TYPE_FUNCALL;
		result.as.funcall = region_allocate(sizeof(Funcall));
		result.as.funcall->name = expr.as.token.text;
		result.as.funcall->args = functions_parse_arglist(ctx);

	} else {
		// log_to_ctx(ctx, LOG_FORMAT "variable: '%.*s'",
		// 	   LOG_CTX("[IDENTIFICATION]", "[STMT]"),
		// 	   Str_Fmt(tok.text));
		// result.type = STMT_VAR;
		// result.as.var = parse_var(ctx);
		// result.as.var.name = tok.text;
	}
	// (void)token_expect_next(ctx,TOKEN_TYPE_STATEMENT_END);
	return result;
}

Expr expr_cache;
bool cachedExpr = false;

const char *expr_get_name(ExprType type)
{
	switch (type) {
	case EXPR_TYPE_FUNCALL:
		return "Function call";
	case EXPR_TYPE_STR:
		return "String literal";
	case EXPR_TYPE_CHAR:
		return "Character literal";
	case EXPR_TYPE_NUMBER:
		return "Numeric value";
	case EXPR_TYPE_VAR:
		return "Variable Name";
	case EXPR_TYPE_OPEN_PAREN:
		return "Open parenthesis";
	case EXPR_TYPE_CLOSING_PAREN:
		return "Closing parenthesis";
	case EXPR_TYPE_OPEN_CURLY:
		return "Open curly brace";
	case EXPR_TYPE_CLOSING_CURLY:
		return "Closing curly brace";
	case EXPR_TYPE_COMMA:
		return "Comma";
	case EXPR_TYPE_COLON:
		return "Colon";
	case EXPR_TYPE_EQUAL:
		return "Assignment operator";
	case EXPR_TYPE_THEN:
		return "Conditional pattern match";
	case EXPR_TYPE_REPEAT:
		return "Conditional retetition";
	case EXPR_TYPE_TOKEN:
		return "token";
	case EXPR_TYPE_STATEMENT_END:
		return "Statement ended with";
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
		   LOG_CTX("[EXPR CHECK]", "[EXPR]"), expr_get_name(expected),
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
		return expr_cache;
	Expr expr = { 0 };

	token_consume(ctx);
	Token token = token_peek_next(ctx);
	switch (token.type) {
	case TOKEN_TYPE_STR:
	case TOKEN_TYPE_CHAR:
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_NAME:
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
		expr.type = (int)token.type;
		expr.as.token.text = token.text;
		break;
	}
	default: {
		assert(0 && "token_get_name: unreachable");
		exit(1);
	}
	}
	expr_cache = expr;
	cachedExpr = true;
	return expr;
}

bool expr_consume(Line_Context *ctx)
{
	(void)ctx;
	if (cachedExpr) {
		update_indent(1);
		log_to_ctx(ctx, LOG_FORMAT "<%s '%.*s'>", LOG_CTX("", "[EXPR]"),
			   expr_get_name(expr_cache.type),
			   Str_Fmt(expr_cache.as.token.text));
		update_indent(-1);
		cachedExpr = false;
		return true;
	}
	return false;
}
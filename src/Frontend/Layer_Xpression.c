#include <Frontend/Layer_Xpression.h>
#include <Frontend/Layer_Tokens.h>
#include <Wrapper/IO.h>
#include <Utils/mem_manager.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

const char *expr_get_name(ExprType type)
{
	switch (type) {
	case EXPR_TYPE_FUNCALL:
		return "Function call";
	case EXPR_TYPE_STR:
		return "String literal";
	case EXPR_TYPE_NUMBER:
		return "Numeric value";
	case EXPR_TYPE_VAR:
		return "Variable Name";
		return "Closing parenthesis";
	case EXPR_TYPE_OPEN_CURLY:
		return "Open curly brace";
	case EXPR_TYPE_CLOSING_CURLY:
		return "Closing curly brace";
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
	case EXPR_TYPE_BIN_OPR:
		return "BINARY OPERATION";
	case EXPR_TYPE_BOOL:
		return "BOOLEAN";
	case EXPR_TYPE_STATEMENT_END:
		return "Statement ended with";
	default: {
		assert(0 && "token_get_name: unreachable");
		exit(1);
	}
	}
}

FuncallArg *parse_funcall_arglist(Line_Context *ctx)
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

Funcall parse_expr_funcall(Line_Context *ctx)
{
	Funcall res = { 0 };
	Token nm = token_expect_next(ctx, TOKEN_TYPE_NAME);
	res.name = nm.text;
	res.args = parse_funcall_arglist(ctx);
	return res;
}

Expr expr_parse_with_precedence(Line_Context *ctx, BinOprPrec p)
{
	// if (p > COUNT_BIN_OPR_PRECEDENCE) {
	//     return expr_peek_next(ctx);
	// }
	// // traverse left side of expr tree
	// Expr lhs = expr_parse_with_precedence(ctx, p + 1);
	assert(0 && "TODO");
}

Expr expr_parse(Line_Context *ctx)
{
	return expr_parse_with_precedence(ctx, 0);
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
	} break;
	case TOKEN_TYPE_CHAR: {
		token_consume(ctx);
		expr.type = EXPR_TYPE_STR;
		expr.as.str = token.text;
	} break;
	case TOKEN_TYPE_NUMBER: {
		token_consume(ctx);
		int64_t res = 0;
		for (size_t i = 0; i < token.text.len; i++) {
			assert(isdigit(token.text.data[i]));
			res = res * 10 + (token.text.data[i] - '0');
		}
		expr.type = EXPR_TYPE_NUMBER;
		expr.as.num = res;
	} break;
	case TOKEN_TYPE_OPEN_PAREN: {
		token_consume(ctx);
		expr = expr_parse(ctx);
		token_expect_next(ctx, TOKEN_TYPE_CLOSING_PAREN);
	} break;
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
		assert(0 && "INVALID TOKEN TYPE ...for now");
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
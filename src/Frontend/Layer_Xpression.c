#include <Frontend/Layer_Xpression.h>
#include <Frontend/Layer_Tokens.h>
#include <Wrapper/IO.h>
#include <Utils/mem_manager.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

BinOprLUT binOprLUT[BIN_OPR_CNT] = {
	[BIN_OPR_AND] = {
		.type = BIN_OPR_AND,
		.tokn = TOKEN_TYPE_AND,
		.prec = BIN_OPR_P0
	},
	[BIN_OPR_OR] = {
        .type = BIN_OPR_OR,
        .tokn = TOKEN_TYPE_OR,
        .prec = BIN_OPR_P0,
    },
    [BIN_OPR_LT] = {
        .type = BIN_OPR_LT,
        .tokn = TOKEN_TYPE_LT,
        .prec = BIN_OPR_P1,
    },
    [BIN_OPR_GE] = {
        .type = BIN_OPR_GE,
        .tokn = TOKEN_TYPE_GE,
        .prec = BIN_OPR_P1,
    },
    [BIN_OPR_NE] = {
        .type = BIN_OPR_NE,
        .tokn = TOKEN_TYPE_NE,
        .prec = BIN_OPR_P1,
    },
    [BIN_OPR_EQ] = {
        .type = BIN_OPR_EQ,
        .tokn = TOKEN_TYPE_EQEQ,
        .prec = BIN_OPR_P1,
    },
    [BIN_OPR_PLUS] = {
        .type = BIN_OPR_PLUS,
        .tokn = TOKEN_TYPE_PLUS,
        .prec = BIN_OPR_P2
    },
    [BIN_OPR_MINUS] = {
        .type = BIN_OPR_MINUS,
        .tokn = TOKEN_TYPE_MINUS,
        .prec = BIN_OPR_P2
    },
    [BIN_OPR_MULT] = {
        .type = BIN_OPR_MULT,
        .tokn = TOKEN_TYPE_MULT,
        .prec = BIN_OPR_P3
    }
};

bool bin_opr_get_def(TokenType tok, BinOprLUT *out)
{
	for (int i = 0; i < BIN_OPR_CNT; i++) {
		if (binOprLUT[i].tokn == tok) {
			if (out)
				*out = binOprLUT[i];
			return true;
		}
	}
	return false;
}

const char *bin_opr_get_name(BinOprType type)
{
	return token_get_name(binOprLUT[type].tokn);
}

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
	log_to_ctx(ctx, LOG_FORMAT("[IDENTIFICATION]", "[STMT]",
				   "- Arguments:", "none"));

	update_indent(1);
	token = token_peek_next(ctx);
	if (token.type == TOKEN_TYPE_CLOSING_PAREN) {
		token_consume(ctx);
		log_to_ctx(ctx, LOG_FORMAT("[IDENTIFICATION]", "[STMT]",
					   " NO ARGS !", "none"));
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
	token_consume(ctx); // closing paren
	update_indent(-2);

	return first;
}

Funcall parse_expr_funcall(Line_Context *ctx)
{
	Funcall res = { 0 };
	res.name = token_expect_next(ctx, TOKEN_TYPE_NAME).text;
	res.args = parse_funcall_arglist(ctx);
	return res;
}

Expr expr_parse_with_precedence(Line_Context *ctx, BinOprPrec p)
{
	(void)ctx;
	(void)p;
	if (p > COUNT_BIN_OPR_PRECEDENCE) {
		return expr_peek_next(ctx);
	}
	// traverse left side of expr tree
	Expr lhs = expr_parse_with_precedence(ctx, p + 1);

	Token tok = token_peek_next(ctx);
	BinOprLUT dets;

	// if next token is a binary opr, then traverse the rhs
	while (bin_opr_get_def(tok.type, &dets) && dets.prec == p) {
		log_to_ctx(ctx, LOG_FORMAT("[EXPR]","[BINOP]", "found bin op %d",dets.type));
		assert(token_consume(ctx));
		Expr expr = { 0 };
		expr.type = EXPR_TYPE_BIN_OPR;
		BinOpr *opr = region_allocate(sizeof(*opr));

		opr->type = dets.type;
		opr->lhs = lhs;
		opr->rhs = expr_parse_with_precedence(ctx, p + 1);

		expr.as.bin_opr = opr;

		lhs = expr;
		tok = token_peek_next(ctx);
	}

	return lhs;
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
			Token next = token_peek_next_next(ctx);
			if (next.type == TOKEN_TYPE_OPEN_PAREN) {
				log_to_ctx(ctx,
					   LOG_FORMAT("[IDENTIFICATION]",
						      "[EXPR]",
						      "funcall: '%.*s'",
						      Str_Fmt(token.text)));
				expr.type = EXPR_TYPE_FUNCALL;
				expr.as.funcall = parse_expr_funcall(ctx);
				token_expect_next(ctx,
						  TOKEN_TYPE_STATEMENT_END);
				return expr;
			} else {
				expr.type = EXPR_TYPE_VAR;
				expr.as.var_nm = token.text;
				token_consume(ctx);
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
	case TOKEN_TYPE_PLUS:
	case TOKEN_TYPE_MINUS:
	case TOKEN_TYPE_MULT:
	case TOKEN_TYPE_LT:
	case TOKEN_TYPE_GE:
	case TOKEN_TYPE_NE:
	case TOKEN_TYPE_AND:
	case TOKEN_TYPE_OR:
	case TOKEN_TYPE_EQEQ:
	case TOKEN_TYPE_STATEMENT_END:
	case TOKEN_TYPE_CNT:
	default: {
		log_to_ctx(ctx,
			   LOG_FORMAT("[IDENTIFICATION]", "[STMT]",
				      "got %s %.*s", token_get_name(token.type),
				      Str_Fmt(token.text)));
		// assert(0 && "expr peek: unreachable");
		exit(1);
	}
	}
	return expr;
}
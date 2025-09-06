#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

Token cache;
bool cachedToken = false;

static bool isName(char x)
{
	return isalnum(x) || x == '_';
}

static bool isNumber(char x)
{
	return isalnum(x) || x == '.' || x == '-';
}

const char *token_get_name(ExprType type)
{
	switch (type) {
	case EXPR_TYPE_STR: 			return "String literal";
	case EXPR_TYPE_CHAR: 			return "Character literal";
	case EXPR_TYPE_NUMBER: 		return "Numeric value";
	case EXPR_TYPE_NAME: 			return "Name value";
	case EXPR_TYPE_OPEN_PAREN: 	return "Open parenthesis";
	case EXPR_TYPE_CLOSING_PAREN: 	return "Closing parenthesis";
	case EXPR_TYPE_OPEN_CURLY: 	return "Open curly brace";
	case EXPR_TYPE_CLOSING_CURLY: 	return "Closing curly brace";
	case TOKEN_TYPE_COMMA: 			return "Comma";
	case TOKEN_TYPE_COLON:			return "Colon";
	case TOKEN_TYPE_EQUAL:			return "Assignment operator";
	case TOKEN_TYPE_EOL:			return "End of line reached!";
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

Token token_expect_next(Line_Context *ctx, ExprType expected)
{
	update_indent(1);
	Token token = token_peek_next(ctx);
	log_to_ctx(ctx, LOG_FORMAT "Expected: '%s'",
		   LOG_CTX("[TOKEN CHECK]", "[EXPR]"), token_get_name(expected),
		   token_get_name(token.type));

	if (!token_consume(ctx)) {
		print(ctx, WIN_STDERR, ": ERROR: expected token `%s`\n",
		      token_get_name(expected));
		exit(1);
	}

	if (token.type != expected) {
		print(ctx, WIN_STDERR,
		      ": ERROR: expected token `%s`, but got `%s`\n",
		      token_get_name(expected), token_get_name(token.type));
		exit(1);
	}
	update_indent(-1);

	return token;
}

Token token_peek_next(Line_Context *ctx)
{
	String *line = &ctx->line;
	if (cachedToken)
		return cache;
	Token token = { 0 };
	(*line) = trim(*line);

	if (line->len == 0) {
		token.type = TOKEN_TYPE_EOL;
		return token;
	}

	switch (line->data[0]) {
	case '(': {
		token.type = EXPR_TYPE_OPEN_PAREN;
		token.text = split_str_by_len(line, 1);
	} break;

	case ';': {
		token.type = EXPR_TYPE_STATEMENT_END;
		token.text = split_str_by_len(line, 1);
	} break;

	case ')': {
		token.type = EXPR_TYPE_CLOSING_PAREN;
		token.text = split_str_by_len(line, 1);
	} break;

	case '{': {
		token.type = EXPR_TYPE_OPEN_CURLY;
		token.text = split_str_by_len(line, 1);
	} break;

	case '}': {
		token.type = EXPR_TYPE_CLOSING_CURLY;
		token.text = split_str_by_len(line, 1);
	} break;

	case ',': {
		token.type = TOKEN_TYPE_COMMA;
		token.text = split_str_by_len(line, 1);
	} break;

	case ':': {
		token.type = TOKEN_TYPE_COLON;
		token.text = split_str_by_len(line, 1);
	} break;

	case '=': {
		token.type = TOKEN_TYPE_EQUAL;
		token.text = split_str_by_len(line, 1);
	} break;

	case '"': {
		split_str_by_len(line, 1); // discard opening "

		token.type = EXPR_TYPE_STR;
		size_t index = 0;
		if (!get_index_of(*line, '"', &index)) {
			print(ctx, WIN_STDERR,
			      "ERROR: Could not find closing \"\n");
			exit(1);
		}
		token.text = split_str_by_len(line, index);

		split_str_by_len(line, 1); // discard closing "
	} break;

	case '\'': {
		split_str_by_len(line, 1); // discard opening '

		token.type = EXPR_TYPE_CHAR;
		size_t index = 0;
		if (!get_index_of(*line, '\'', &index)) {
			print(ctx, WIN_STDERR,
			      "ERROR: Could not find closing \'\n");
			exit(1);
		}
		token.text = split_str_by_len(line, index);

		split_str_by_len(line, 1); // discard closing '
	} break;

	default: {
		if (starts_with(*line, STR("->"))) {
			token.type = EXPR_TYPE_THEN;
			token.text = split_str_by_len(line, 2);
		} else if (starts_with(*line, STR("then"))) {
			token.type = EXPR_TYPE_THEN;
			token.text = split_str_by_len(line, 4);
		} else if (starts_with(*line, STR("<->"))) {
			token.type = EXPR_TYPE_REPEAT;
			token.text = split_str_by_len(line, 3);
		} else if (starts_with(*line, STR("repeat"))) {
			token.type = EXPR_TYPE_REPEAT;
			token.text = split_str_by_len(line, 6);
		} else if (isalpha(line->data[0])) {
			token.type = EXPR_TYPE_NAME;
			token.text = split_str_by_condition(line, isName);
		} else if (isdigit(line->data[0]) || line->data[0] == '-') {
			token.type = EXPR_TYPE_NUMBER;
			token.text = split_str_by_condition(line, isNumber);
		} else {
			print(ctx, WIN_STDERR,
			      "Unknown token starting with '%c'\n",
			      line->data[0]);
			exit(1);
		}
	}
	}
	cache = token;
	cachedToken = true;
	return token;
}

bool token_consume(Line_Context *ctx)
{
	(void)ctx;
	if (cachedToken) {
		update_indent(1);
		log_to_ctx(ctx, LOG_FORMAT "<%s '%.*s'>", LOG_CTX("", "[EXPR]"),
			   token_get_name(cache.type), cache.text.len,
			   cache.text.data);
		update_indent(-1);
		cachedToken = false;
		return true;
	}
	return false;
}
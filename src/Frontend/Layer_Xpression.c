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

const char *token_get_name(TokenType type)
{
	switch (type) {
	case TOKEN_TYPE_STR: 			return "String literal";
	case TOKEN_TYPE_CHAR: 			return "Character literal";
	case TOKEN_TYPE_NUMBER: 		return "Numeric value";
	case TOKEN_TYPE_NAME: 			return "Name value";
	case TOKEN_TYPE_OPEN_PAREN: 	return "Open parenthesis";
	case TOKEN_TYPE_CLOSING_PAREN: 	return "Closing parenthesis";
	case TOKEN_TYPE_OPEN_CURLY: 	return "Open curly brace";
	case TOKEN_TYPE_CLOSING_CURLY: 	return "Closing curly brace";
	case TOKEN_TYPE_COMMA: 			return "Comma";
	case TOKEN_TYPE_COLON:			return "Colon";
	case TOKEN_TYPE_EQUAL:			return "Assignment operator";
	case TOKEN_TYPE_EOL:			return "End of line reached!";
	// case TOKEN_TYPE_FUNC: 			return "func";
	case TOKEN_TYPE_STATEMENT_END: 	return "Statement ended with";
	default: {
		assert(0 && "token_get_name: unreachable");
		exit(1);
	}
	}
}

Token token_expect_next(Line_Context* ctx, TokenType expected)
{
	Token token = token_fetch_next(ctx);
	update_indent(1);
	log_to_ctx(ctx, LOG_FORMAT "Expected: '%s'",LOG_CTX("[TOKEN CHECK]","[EXPR]"),
	token_get_name(expected), token_get_name(token.type));
	update_indent(-1);

	if (!discard_cached_token()) {
		print(ctx, WIN_STDERR, 
			": ERROR: expected token `%s`\n", token_get_name(expected));
		exit(1);
	}

	if (token.type != expected) {
		print(ctx, WIN_STDERR,
			  ": ERROR: expected token `%s`, but got `%s`\n",
			  token_get_name(expected), token_get_name(token.type));
		exit(1);
	}

	return token;
}

Token token_fetch_next(Line_Context* ctx)
{
	String *line = &ctx->line;
	if (cachedToken) return cache;
	Token token = { 0 };
	(*line) 	= trim(*line);

	if (line->len == 0) {
		token.type = TOKEN_TYPE_EOL;

		log_to_ctx(ctx, LOG_FORMAT "End of line", LOG_CTX("","[EXPR]"));
		return token;
	}

	switch (line->data[0]) {
	case '(': {
		token.type = TOKEN_TYPE_OPEN_PAREN;
		token.text = split_str_by_len(line, 1);
	} break;

	case ';': {
		token.type = TOKEN_TYPE_STATEMENT_END;
		token.text = split_str_by_len(line, 1);
	} break;

	case ')': {
		token.type = TOKEN_TYPE_CLOSING_PAREN;
		token.text = split_str_by_len(line, 1);
	} break;

	case '{': {
		token.type = TOKEN_TYPE_OPEN_CURLY;
		token.text = split_str_by_len(line, 1);
	} break;

	case '}': {
		token.type = TOKEN_TYPE_CLOSING_CURLY;
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

		token.type = TOKEN_TYPE_STR;
		size_t index = 0;
		if (!get_index_of(*line, '"', &index)) {
			print(ctx, WIN_STDERR, "ERROR: Could not find closing \"\n");
			exit(1);
		}
		token.text = split_str_by_len(line, index);

		split_str_by_len(line, 1); // discard closing "
	} break;

	case '\'': {
		split_str_by_len(line, 1); // discard opening '

		token.type = TOKEN_TYPE_CHAR;
		size_t index = 0;
		if (!get_index_of(*line, '\'', &index)) {
			print(ctx, WIN_STDERR, "ERROR: Could not find closing \'\n");
			exit(1);
		}
		token.text = split_str_by_len(line, index);

		split_str_by_len(line, 1); // discard closing '
	} break;

	default: {
		if (isalpha(line->data[0])) {
			token.type = TOKEN_TYPE_NAME;
			token.text = split_str_by_condition(line, isName);
		} else if (isdigit(line->data[0]) || line->data[0] == '-') {
			token.type = TOKEN_TYPE_NUMBER;
			token.text = split_str_by_condition(line, isNumber);
		} else {
			print(ctx, WIN_STDERR,
				  "ERROR: Unknown token '%c' '%c'\n",
				  line->data[0],line->data[1]);
			// exit(1);
		}
	}
	}
	log_to_ctx(ctx, LOG_FORMAT "%s '%.*s'",LOG_CTX("","[EXPR]"),token_get_name(token.type),
		  token.text.len, token.text.data);
	cache = token;
	cachedToken = true;
	return token;
}

bool discard_cached_token()
{
	if (cachedToken) {
		cachedToken = false;
		return true;
	}
	return false;
}
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

const char *getTokenName(TokenType type)
{
	switch (type) {
	case TOKEN_TYPE_STR:
		return "string";
	case TOKEN_TYPE_CHAR:
		return "character";
	case TOKEN_TYPE_NUMBER:
		return "number";
	case TOKEN_TYPE_NAME:
		return "name";
	case TOKEN_TYPE_OPEN_PAREN:
		return "open paren";
	case TOKEN_TYPE_CLOSING_PAREN:
		return "closing paren";
	case TOKEN_TYPE_OPEN_CURLY:
		return "open curly";
	case TOKEN_TYPE_CLOSING_CURLY:
		return "closing curly";
	case TOKEN_TYPE_COMMA:
		return "comma";
	case TOKEN_TYPE_FUNC:
		return "func";
	case TOKEN_TYPE_STATEMENT_END:
		return "statement end";
	default: {
		assert(0 && "getTokenName: unreachable");
		exit(1);
	}
	}
}

Token getNextToken(String *line)
{
	*line = trim(*line);
	if (cachedToken) {
		return cache;
	}

	Token token = { 0 };
	if (starts_with(*line, STR("func"))) {
		token.type = TOKEN_TYPE_FUNC;
		token.text = split_str_by_len(line, 4);
		cache = token;
		cachedToken = true;

		// print(WIN_STDOUT,
		//       "\n[EXPR] identified token '%.*s' as '%s' token type",
		//       token.text.len, token.text.data,
		//       getTokenName(token.type));
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

	case '"': {
		split_str_by_len(line, 1);

		size_t index = 0;

		if (get_index_of(*line, '"', &index)) {
			String text = split_str_by_len(line, index);
			split_str_by_len(line, 1);
			token.type = TOKEN_TYPE_STR;
			token.text = text;
		} else {
			print(WIN_STDERR, "ERROR: Could not find closing \"\n");
			exit(1);
		}
	} break;

	case '\'': {
		split_str_by_len(line, 1);

		size_t index = 0;

		if (get_index_of(*line, '\'', &index)) {
			String text = split_str_by_len(line, index);
			split_str_by_len(line, 1);
			token.type = TOKEN_TYPE_CHAR;
			token.text = text;
		} else {
			print(WIN_STDERR, "ERROR: Could not find closing \'\n");
			exit(1);
		}
	} break;

	default: {
		if (isalpha(line->data[0])) {
			token.type = TOKEN_TYPE_NAME;
			token.text = split_str_by_condition(line, isName);
		} else if (isdigit(line->data[0]) || line->data[0] == '-') {
			token.type = TOKEN_TYPE_NUMBER;
			token.text = split_str_by_condition(line, isNumber);
		} else {
			print(WIN_STDERR,
			      "ERROR: Unknown token starts with '%c'\n",
			      line->data[0]);
			exit(1);
		}
	}
	}
	cache = token;
	cachedToken = true;
	// print(WIN_STDOUT, "\n[EXPR] identified token '%.*s' as '%s' token type",
	//       token.text.len, token.text.data, getTokenName(token.type));
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
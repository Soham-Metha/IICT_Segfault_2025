#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>
#include <ctype.h>

static bool isName(char x)
{
	return isalnum(x) || x == '_';
}

static bool isNumber(char x)
{
	return isalnum(x) || x == '.' || x == '-';
}

Token getNextToken(String *line)
{
	Token token = { 0 };
	switch (line->data[0]) {
	case '(': {
		token.type = TOKEN_TYPE_OPEN_PAREN;
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

		if (getIndexOf(line, '"', &index)) {
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

		if (getIndexOf(line, '\'', &index)) {
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
			      "ERROR: Unknown token starts with %c\n",
			      line->data[0]);
			exit(1);
		}
	}
	}
	return token;
}

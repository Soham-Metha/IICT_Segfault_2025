#include <Utils/strings.h>
#include <stddef.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

String ltrim(String s)
{
	size_t i = 0;
	while (i < s.len && isspace(s.data[i])) {
		i++;
	}
	return (String){ .len = s.len - i, .data = s.data + i };
}

String rtrim(String s)
{
	size_t i = s.len;
	while (i > 0 && isspace(s.data[i - 1])) {
		i--;
	}
	return (String){ .len = i, .data = s.data };
}

String trim(String s)
{
	return rtrim(ltrim(s));
}

bool compare_str(String a, String b)
{
	if (a.len != b.len) {
		return false;
	} else {
		return memcmp(a.data, b.data, a.len) == 0;
	}
}

bool get_index_of(String str, char c, size_t *index)
{
	size_t i = 0;
	while (i < str.len && str.data[i] != c) {
		i += 1;
	}

	if (i < str.len) {
		*index = i;
		return true;
	}
	return false;
}

bool starts_with(String str, String prefix)
{
	if (prefix.len <= str.len) {
		for (size_t i = 0; i < prefix.len; ++i) {
			if (prefix.data[i] != str.data[i]) {
				return false;
			}
		}
		return true;
	}
	return false;
}

String split_str_by_delim(String *s, char c)
{
	size_t i = 0;
	while (i < s->len && s->data[i] != c) {
		i++;
	}

	String res = split_str_by_len(s, i);

	if (i < s->len) { // discard the delim
		s->len -= 1;
		s->data += 1;
	}

	return res;
}

String split_str_by_condition(String *str, bool (*predicate)(char x))
{
	size_t i = 0;
	while (i < str->len && predicate(str->data[i])) {
		i += 1;
	}

	return split_str_by_len(str, i);
}

String split_str_by_len(String *str, size_t n)
{
	if (n > str->len) {
		n = str->len;
	}

	String result = {
		.data = str->data,
		.len = n,
	};

	str->data += n;
	str->len -= n;

	return result;
}

String split_str_by_len_reversed(String *str, size_t n)
{
	if (n > str->len) {
		n = str->len;
	}

	String result = { .data = str->data + str->len - n, .len = n };

	str->len -= n;

	return result;
}

bool ends_with(String str, String expectedSuffix)
{
	if (expectedSuffix.len <= str.len) {
		String actualSuffix = { .data = str.data + str.len -
						expectedSuffix.len,
					.len = expectedSuffix.len };

		return compare_str(expectedSuffix, actualSuffix);
	}

	return false;
}
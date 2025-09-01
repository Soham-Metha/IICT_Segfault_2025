#include <Frontend/Layer_Statement.h>
#include <Wrapper/IO.h>

Line_View getNextStmt(String line)
{
	Line_View result = { 0 };
	if (starts_with(line, STR("#"))) {
		split_str_by_len(&line, 1);
		result.type = LINE_TYPE_DIRECTIVE;
		result.value.as_Directive.name =
			trim(split_str_by_delim(&line, ' '));
		result.value.as_Directive.body = trim(line);
		print(WIN_STDOUT,
		      "\n[STMT] Identified Directive: '%.*s', body: '%.*s'",
		      result.value.as_Directive.name.len,
		      result.value.as_Directive.name.data,
		      result.value.as_Directive.body.len,
		      result.value.as_Directive.body.data);
	}
	return result;
}
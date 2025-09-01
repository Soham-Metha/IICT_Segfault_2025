#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_File.h>
#include <Utils/strings.h>
#include <Wrapper/IO.h>

String getNextLine()
{
	String line = { 0 };

	do {
		file.line_num += 1;
		file.contents = ltrim(file.contents);

		line = trim(split_str_by_delim(&file.contents, '\n'));
		print(WIN_STDOUT, "\n[LINE] Reading Line %3u : %.*s",
		      file.line_num, (int)line.len, line.data);

		if (get_index_of(line, COMMENT_SYMBOL, NULL)) {
			line = trim(split_str_by_delim(&line, COMMENT_SYMBOL));
			print(WIN_STDOUT, "\n[LINE] Discard Comments : %.*s",
			      (int)line.len, line.data);
		}
	} while (line.len == 0 && file.contents.len > 0);

	getNextStmt(line);
    while (line.len > 0) {
        Line_View line_v = getNextStmt(line);
        (void)line_v;
    }
}

void getNextStmt(String line)
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
	(void)result;
}
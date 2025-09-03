#include <Wrapper/IO.h>
#include <stdio.h>
#include <Frontend/Layer_File.h>
#include <stdlib.h>
#include <assert.h>

FILE *ast;

void print(int id, const char *str, ...)
{
	va_list args;
	va_start(args, str);

	if (id == WIN_AST) {
		if (!ast) {
			ast = fopen("ast.dot", "w");
		}
		vfprintf(ast, str, args);
	} else if (id == WIN_STDERR) {
		vprintf(str, args);
	} else {
		assert(id == WIN_STDERR);
	}

	va_end(args);
}

void log_to_ctx(Line_Context *ctx, const char *str, ...)
{
	va_list args;
	va_start(args, str);

	va_list args_copy;
	va_copy(args_copy, args);

	int size = vsnprintf(NULL, 0, str, args_copy);
	char *log = malloc(size + 1);

	vsnprintf(log, size + 1, str, args);

	ctx->logs[ctx->log_cnt++] = (String){ .data = log, .len = size };
	printf("%s\n", log);

	va_end(args);
}

int indent = 1;
char *space = "..............................";
String get_indent()
{
	return (String){ .data = space, .len = indent*4 };
}

void update_indent(int change)
{
	indent += change;
}
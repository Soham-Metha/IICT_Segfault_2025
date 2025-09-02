#include <Wrapper/IO.h>
#include <stdio.h>

FILE *ast;

void print(int id, const char *str, ...)
{
	(void)id; // unused for now
	va_list args;
	va_start(args, str);

	if (id == WIN_AST) {
		if (!ast) {
			ast = fopen("ast.dot", "w");
		}
		vfprintf(ast, str, args);
	} else {
		vprintf(str, args);
	}

	va_end(args);
}
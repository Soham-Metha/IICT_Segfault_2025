#include <Wrapper/IO.h>
#include <stdio.h>
#include <Frontend/Layer_File.h>
#include <stdlib.h>

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
	// } else if (id == WIN_STDERR) {
	// 	vprintf(str, args);
	} else {
		// vprintf(str, args);
		if (current_buffer && current_buffer_size) {
			va_list args_copy;
			va_copy(args_copy, args);
			int size = vsnprintf(NULL, 0, str, args_copy);
			char *log = malloc(size + 1);
			vsnprintf(log, size + 1, str, args);
			log[size] = '\0';
			current_buffer[*current_buffer_size] =
				(String){ .data = log, .len = size };
			printf("%.*s\t\t%d \n",
			       Str_Fmt(current_buffer[*current_buffer_size]),
			       *current_buffer_size);
			(*current_buffer_size)++;
		}
	}

	va_end(args);
}
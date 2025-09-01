#include <Wrapper/IO.h>
#include <stdio.h>

void print(int id, const char *str, ...)
{
    (void)id; // unused for now
	va_list args;
	va_start(args, str);

	printf(str, args);

	va_end(args);
}
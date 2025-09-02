#ifndef IO
#define IO
#include <stdarg.h>

enum WindowNames { WIN_STDIN, WIN_STDOUT, WIN_STDERR, WIN_AST, WIN_COUNT };
// stdin will probably remain unused, and other windows may get added as
// we improve the UI

/*
 * print a string!
 * id is used to identify the output window where string is to be printed!
 */
void print(int id, const char *str, ...);

#define AST(shape, color, lbl)                      \
	"  Expr_%d "                                \
	"[shape=" shape " style=filled "            \
	"fillcolor=" color " fontname=\"Courier\" " \
	"label=\"" lbl "\"];\n"
#endif
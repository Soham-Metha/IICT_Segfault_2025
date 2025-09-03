#ifndef IO
#define IO
#include <stdarg.h>
#include <Frontend/Layer_Line.h>

#define LOG_CTX(action, lvl) action, lvl
#define LOG_FORMAT " %20s %8s | "

enum WindowNames { WIN_STDIN, WIN_STDOUT, WIN_STDERR, WIN_AST, WIN_COUNT };
// stdin will probably remain unused, and other windows may get added as
// we improve the UI
typedef struct Line_Context Line_Context;
/*
 * print a string!
 * id is used to identify the output window where string is to be printed!
 */
void print(int id, const char *str, ...);

void log_to_ctx(Line_Context *ctx, const char *str, ...);

#define AST(shape, color, lbl)                      \
	"  Expr_%d "                                \
	"[shape=" shape " style=filled "            \
	"fillcolor=" color " fontname=\"Courier\" " \
	"label=\"" lbl "\"];\n"
#endif
#ifndef IO
#define IO
#include <stdarg.h>
#include <Frontend/Layer_Line.h>

#define LOG_CTX(action, lvl, str, ...) lvl action str, Str_Fmt(get_indent()), ##__VA_ARGS__

#define LOG_FORMAT "\n %6s | %20s %.*s "

#define IR_FORMAT(str, ...) "\n%.*s" str, Str_Fmt(get_indent()), ##__VA_ARGS__
#define print_IR(...) print(NULL, WIN_IR, __VA_ARGS__);
#define print_AST(...) print(NULL, WIN_AST, __VA_ARGS__);

enum WindowNames {
	WIN_STDIN,
	WIN_STDOUT,
	WIN_STDERR,
	WIN_AST,
	WIN_IR,
	WIN_COUNT
};

// stdin will probably remain unused, and other windows may get added as
// we improve the UI

/*
 * print a string!
 * id is used to identify the output window where string is to be printed!
 */
void print(Line_Context *ctx, int id, const char *str, ...);

void log_to_ctx(Line_Context *ctx, const char *str, ...);

String get_indent();
void update_indent(int change);

#define AST(shape, color, lbl)                      \
	"  Expr_%d "                                \
	"[shape=" shape " style=filled "            \
	"fillcolor=" color " fontname=\"Courier\" " \
	"label=\"" lbl "\"];\n"
#endif
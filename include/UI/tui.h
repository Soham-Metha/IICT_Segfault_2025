#ifndef TUI_H
#define TUI_H

#include <ncurses.h>

#define MAX_LINES 128
// this can be used instead of hard coded value
typedef struct {
	char *code;
	char *logs;
	char *ir;
	char *mc;
} CODE_LINE;

typedef struct {
	WINDOW *program_window;
	WINDOW *log_window;
	WINDOW *ir_window;
	WINDOW *mc_window;
	unsigned int selected_line;
	unsigned int total_lines;
	unsigned int capacity;
} TUI;

TUI *init_ui();

void run_ui(TUI *tui);

void destroy_ui(TUI *tui);

#endif

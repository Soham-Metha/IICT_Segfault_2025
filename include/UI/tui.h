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
    int selected_line;
    int total_lines;
    char *lines[MAX_LINES];
    // CODE_LINE *lines[MAX_LINES]; // same as prev commrnt, if this is to be used, we can comment out line 22
    int capacity;
} TUI;


TUI *init_ui(int screen_height, int screen_width); 

void run_ui(TUI *tui);

void destroy_ui(TUI *tui);

#endif

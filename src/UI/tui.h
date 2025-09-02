#ifndef TUI_H
#define TUI_H
#include <ncurses.h>

typedef struct {
    WINDOW *input_win;
    WINDOW *process_win;
    int height, width;
} TUI;

TUI *tui_init(int screen_height, int screen_width);

void tui_draw(TUI *tui);

void tui_get_input(TUI *tui, char *buffer, int size);

void tui_print_line(TUI *tui, const char *line);

void tui_destroy(TUI *tui);


#endif
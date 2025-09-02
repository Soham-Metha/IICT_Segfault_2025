#include "tui.h"
#include <stdlib.h>
#include <string.h>

TUI *tui_init(int screen_height, int screen_width) {
    initscr();
    cbreak();
    echo();
    curs_set(1);
    keypad(stdscr, TRUE);

    TUI *tui = malloc(sizeof(TUI));
    tui->height = screen_height;
    tui->width = screen_width;

    int half_width = screen_width / 2;

    tui->input_win = newwin(screen_height, screen_width - half_width, 0, half_width);
    
    tui->process_win = newwin(screen_height, half_width, 0, 0);
    
    tui_draw(tui);

    return tui;
}



void tui_draw(TUI *tui) {
    box(tui->process_win, 0, 0);
    box(tui->input_win, 0, 0);

    mvwprintw(tui->input_win, 0, 2, "[ Input Window ]");
    mvwprintw(tui->process_win, 0, 2, "[ Processing Window ]");

    wrefresh(tui->input_win);
    wrefresh(tui->process_win);
}



void tui_get_input(TUI *tui, char *buffer, int size) {
    mvwgetnstr(tui->input_win, 1, 1, buffer, size - 1);
}

void tui_print_line(TUI *tui, const char *line) {
    static int y = 1;
    mvwprintw(tui->process_win, y++, 1, "%s", line);
    wrefresh(tui->process_win);
}

void tui_destroy(TUI *tui) {
    delwin(tui->input_win);
    delwin(tui->process_win);
    free(tui);
    endwin();
}
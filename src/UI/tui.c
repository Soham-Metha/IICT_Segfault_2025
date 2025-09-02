#include <UI/tui.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <assert.h>
#include <Frontend/Layer_File.h>

TUI *init_ui()
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	// keypad(stdscr, TRUE);
	start_color();

	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(5, COLOR_RED, COLOR_BLACK);

	TUI *tui = malloc(sizeof(TUI));
	tui->selected_line = 0;
	tui->total_lines = 0;

	int screen_height = getmaxy(stdscr);
	int screen_width = getmaxx(stdscr);

	int mid_x = (int)(screen_width / 2);
	int mid_y = (int)(screen_height / 2);
	int x_by4 = (int)(screen_width / 4);

	tui->program_window = newwin(screen_height, screen_width / 2, 0, 0);
	tui->log_window = newwin(mid_y, mid_x, 0, mid_x);
	tui->ir_window = newwin(mid_y, x_by4, mid_y, mid_x);
	tui->mc_window = newwin(mid_y, x_by4, mid_y, mid_x + x_by4);

	return tui;
}

static void draw_program(TUI *tui)
{
	werase(tui->program_window);
	box(tui->program_window, 0, 0);
	wattron(tui->program_window, COLOR_PAIR(1));
	mvwprintw(tui->program_window, 0, 2, "[ Program ]");
	wattroff(tui->program_window, COLOR_PAIR(1));

	for (unsigned int i = 0; i <= file.line_num; i++) {
		if (i == tui->selected_line) {
			wattron(tui->program_window, COLOR_PAIR(2) | A_BOLD);
			mvwprintw(tui->program_window, i + 1, 2, "%.*s",
				  Str_Fmt(file.lines[i]));
			wattroff(tui->program_window, COLOR_PAIR(2) | A_BOLD);
		} else {
			mvwprintw(tui->program_window, i + 1, 2, "%.*s",
				  Str_Fmt(file.lines[i]));
		}
	}

	wrefresh(tui->program_window);
}

static void draw_log(TUI *tui)
{
	werase(tui->log_window);
	box(tui->log_window, 0, 0);
	wattron(tui->log_window, COLOR_PAIR(3));
	mvwprintw(tui->log_window, 0, 2, "[ Logs ]");
	wattroff(tui->log_window, COLOR_PAIR(3));

	// if (tui->total_lines > 0) {
	// 	mvwprintw(tui->log_window, 1, 1, "Selected line %d : %s",
	// 		  tui->selected_line + 1,
	// 		  tui->lines[tui->selected_line]);
	// }
	wrefresh(tui->log_window);
}

static void draw_ir_mc(TUI *tui)
{
	werase(tui->ir_window);
	box(tui->ir_window, 0, 0);
	wattron(tui->ir_window, COLOR_PAIR(4));
	mvwprintw(tui->ir_window, 0, 2, "[ IR ]");
	wattroff(tui->ir_window, COLOR_PAIR(4));
	mvwprintw(tui->ir_window, 1, 1, "IR for line %d",
		  tui->selected_line + 1);
	wrefresh(tui->ir_window);

	werase(tui->mc_window);
	box(tui->mc_window, 0, 0);
	wattron(tui->mc_window, COLOR_PAIR(5));
	mvwprintw(tui->mc_window, 0, 2, "[ Machine Code ]");
	wattroff(tui->mc_window, COLOR_PAIR(5));
	mvwprintw(tui->mc_window, 1, 1, "MC for line %d",
		  tui->selected_line + 1);
	wrefresh(tui->mc_window);
}

static void refresh_ui(TUI *tui)
{
	draw_program(tui);
	draw_log(tui);
	draw_ir_mc(tui);
}

void run_ui(TUI *tui)
{
	int ch;
	refresh_ui(tui);

	while ((ch = getch()) != 'q') {
		switch (ch) {
		case KEY_UP:
			tui->selected_line =
				(tui->selected_line - 1 + tui->total_lines) %
				tui->total_lines;
			break;
		case KEY_DOWN:
			tui->selected_line =
				(tui->selected_line + 1) % tui->total_lines;
			break;
		}
		refresh_ui(tui);
	}
}

void destroy_ui(TUI *tui)
{
	delwin(tui->program_window);
	delwin(tui->log_window);
	delwin(tui->ir_window);
	delwin(tui->mc_window);
	free(tui);
	endwin();
}

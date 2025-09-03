#include <UI/tui.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <assert.h>

TUI *init_ui(File_Context *ctx)
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	start_color();

	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(5, COLOR_RED, COLOR_BLACK);

	TUI *tui = malloc(sizeof(TUI));
	tui->selected_line = 0;

	int screen_height = getmaxy(stdscr);
	int screen_width = getmaxx(stdscr);

	int mid_x = (int)(screen_width / 2);
	int mid_y = (int)(screen_height / 2);
	int x_by4 = (int)(screen_width / 4);

	tui->program_window =
		newwin(screen_height - 4, mid_x - 4, 0 + 2, 0 + 2);
	tui->log_window = newwin(mid_y - 4, mid_x - 4, 0 + 2, mid_x + 2);
	tui->ir_window = newwin(mid_y - 4, x_by4 - 4, mid_y + 2, mid_x + 2);
	tui->mc_window =
		newwin(mid_y - 4, x_by4 - 4, mid_y + 2, mid_x + x_by4 + 2);
	tui->ctx = ctx;

	return tui;
}

static void draw_program(TUI *tui)
{
	WINDOW *bounds = newwin(getmaxy(stdscr), getmaxx(stdscr) / 2, 0, 0);
	box(bounds, 0, 0);
	wattron(bounds, COLOR_PAIR(1));
	mvwprintw(bounds, 0, 2, "[ Program ]");
	wattroff(bounds, COLOR_PAIR(1));
	wrefresh(bounds);
	werase(tui->program_window);

	for (unsigned int i = 0; i < tui->ctx->line_num; i++) {
		if (i == tui->selected_line) {
			wattron(tui->program_window, COLOR_PAIR(2) | A_BOLD);
		}
		wprintw(tui->program_window, "%3d | %s\n", i,
			tui->ctx->lines[i].line_start);
		wattroff(tui->program_window, COLOR_PAIR(2) | A_BOLD);
	}

	wrefresh(tui->program_window);
}

static void draw_log(TUI *tui)
{
	int screen_height = getmaxy(stdscr);
	int screen_width = getmaxx(stdscr);

	int mid_x = (int)(screen_width / 2);
	int mid_y = (int)(screen_height / 2);

	WINDOW *bounds = newwin(mid_y, mid_x, 0, mid_x);
	box(bounds, 0, 0);
	wattron(bounds, COLOR_PAIR(3));
	mvwprintw(bounds, 0, 2, "[ Logs ]");
	wattroff(bounds, COLOR_PAIR(3));
	wrefresh(bounds);
	werase(tui->log_window);

	if (tui->ctx->line_num > 0) {
		int i = 0;
		while (tui->ctx->lines[tui->selected_line].logs[i].len > 0) {
			wprintw(tui->log_window, "%.*s",
				Str_Fmt(tui->ctx->lines[tui->selected_line]
						.logs[i]));
			i++;
		}
	}
	wrefresh(tui->log_window);
}

static void draw_ir_mc(TUI *tui)
{
	int screen_height = getmaxy(stdscr);
	int screen_width = getmaxx(stdscr);

	int mid_x = (int)(screen_width / 2);
	int mid_y = (int)(screen_height / 2);
	int x_by4 = (int)(screen_width / 4);

	WINDOW *bot_l = newwin(mid_y, x_by4, mid_y, mid_x);
	WINDOW *bot_r = newwin(mid_y, x_by4, mid_y, mid_x + x_by4);

	box(bot_l, 0, 0);
	wattron(bot_l, COLOR_PAIR(4));
	mvwprintw(bot_l, 0, 2, "[ IR ]");
	wattroff(bot_l, COLOR_PAIR(4));
	box(bot_r, 0, 0);
	wattron(bot_r, COLOR_PAIR(5));
	mvwprintw(bot_r, 0, 2, "[ Machine Code ]");
	wattroff(bot_r, COLOR_PAIR(5));

	wrefresh(bot_l);
	wrefresh(bot_r);

	werase(tui->ir_window);
	mvwprintw(tui->ir_window, 1, 1, "IR for line %d",
		  tui->selected_line + 1);
	wrefresh(tui->ir_window);

	werase(tui->mc_window);
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
				(tui->selected_line + tui->ctx->line_num - 1) %
				tui->ctx->line_num;

			break;
		case KEY_DOWN:
			tui->selected_line =
				(tui->selected_line + 1) % tui->ctx->line_num;
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

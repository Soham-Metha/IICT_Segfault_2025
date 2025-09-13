#include <Wrapper/UI.h>
#include <Frontend/Layer_Tokens.h>
#include <ncurses.h>

#define COLOR_GREY 8

#define LERP(START, END, T) (START * (1 - T) + END * T)
#define BOUNDS(x1, y1, x2, y2) (y2 - y1, x2 - x1, y1, x1)
#define INNER_BOUNDS(x1, y1, x2, y2) BOUNDS(x1 + 1, y1 + 1, x2 - 3, y2 - 3)

int min_x, min_y, max_y, max_x, mid_x, mid_y, x_by4;

WINDOW* windows[UI_CNT];
WINDOW* bounds[UI_CNT];

File_Context* ctx;
unsigned int selected_line;

static String WindowNames[UI_CNT] = {
	[UI_PROG] = { .data = "PROGRAM", .len = 7 },
	[UI_LOGS] = { .data = "LOGS", .len = 4 },
	[UI_TOKEN_CHART] = { .data = "TOKEN CHART", .len = 27 },

};

void initColors()
{
    start_color();
    init_color(COLOR_BLACK, 102, 106, 149);
    init_color(COLOR_RED, 999, 0, 333);
    init_color(COLOR_GREEN, 278, 921, 705);
    init_color(COLOR_YELLOW, 999, 795, 419);
    // init_color(COLOR_BLUE,);
    init_color(COLOR_MAGENTA, 615, 431, 995);
    init_color(COLOR_CYAN, 537, 866, 999);
    init_color(COLOR_WHITE, 815, 815, 815);
    init_color(COLOR_GREY, 300, 300, 300);

    // Used for highlighting
    init_pair(1, COLOR_GREY, COLOR_BLACK);

    // Used for window names
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_BLACK);
}

void create_all_windows()
{
    min_x = 0;
    min_y = 0;
    max_y = getmaxy(stdscr);
    max_x = getmaxx(stdscr);

    mid_x = LERP(min_x, max_x, 0.4);
    mid_y = LERP(max_y, max_y, 0.5);
    x_by4 = LERP(mid_x, max_x, 0.5);
}

static void draw_bounds()
{
	bounds[UI_PROG] = newwin BOUNDS(min_x, min_y, mid_x, max_y);
	bounds[UI_LOGS] = newwin BOUNDS(mid_x, min_x, max_x, mid_y);
	bounds[UI_TOKEN_CHART] = newwin BOUNDS(mid_x, mid_y, max_x, max_y);

	windows[UI_PROG] = newwin INNER_BOUNDS(min_x, min_y, mid_x, max_y);
	windows[UI_LOGS] = newwin INNER_BOUNDS(mid_x, min_x, max_x, mid_y);
	windows[UI_TOKEN_CHART] = newwin INNER_BOUNDS(mid_x, mid_y, max_x, max_y);

	for (int i = 0; i < UI_CNT; i++) {
		box(bounds[i], 0, 0);
		wattron(bounds[i], COLOR_PAIR(i + 2));
		mvwprintw(bounds[i], 0, 2, "[ %.*s ]", Str_Fmt(WindowNames[i]));
		wattroff(bounds[i], COLOR_PAIR(i + 2));
		wrefresh(bounds[i]);
		wbkgd(windows[i], COLOR_PAIR(i + 2));
		// box(windows[i], 0, 0);
	}
}

static void draw_program()
{
    werase(windows[UI_PROG]);

    for (unsigned int i = 0; i < ctx->line_num; i++) {
        if (i == selected_line) {
            wattron(windows[UI_PROG], A_REVERSE);
        }
        wprintw(windows[UI_PROG], "\n%3d | %-*s", i + 1, mid_x - 12,
            ctx->lines[i].line_start);
        wattroff(windows[UI_PROG], A_REVERSE);
    }

    wrefresh(windows[UI_PROG]);
}

static void draw_log()
{
    werase(windows[UI_LOGS]);

    wattron(windows[UI_LOGS], COLOR_PAIR(1));
    if (selected_line > 0) {
        int i = 0;
        while (ctx->lines[selected_line - 1].logs[i].len > 0) {
            wprintw(windows[UI_LOGS], "%.*s",
                Str_Fmt(ctx->lines[selected_line - 1].logs[i]));
            i++;
        }
    } else {
        wprintw(windows[UI_LOGS],
            "\n ----x---- LOGS START HERE ----x---- \n");
    }
    wattroff(windows[UI_LOGS], COLOR_PAIR(1));

    if (ctx->line_num > 0) {
        int i = 0;
        while (ctx->lines[selected_line].logs[i].len > 0) {
            wprintw(windows[UI_LOGS], "%.*s",
                Str_Fmt(ctx->lines[selected_line].logs[i]));
            i++;
        }
    }

    wattron(windows[UI_LOGS], COLOR_PAIR(1));

    unsigned int j = selected_line + 1;
    while (j < ctx->line_num) {
        int i = 0;
        while (ctx->lines[j].logs[i].len > 0) {
            wprintw(windows[UI_LOGS], "%.*s",
                Str_Fmt(ctx->lines[j].logs[i]));
            i++;
        }
        j++;
    }
    wprintw(windows[UI_LOGS],
        "\n\n ----x---- LOGS END HERE ----x---- \n");

    wattroff(windows[UI_LOGS], COLOR_PAIR(1));
    wrefresh(windows[UI_LOGS]);
}

static void draw_token_chart() {
    werase(windows[UI_TOKEN_CHART]);

    int max_freq = 0;
    for (int t = 0; t < TOKEN_TYPE_CNT; t++) {
        int freq = token_freq_get(t);
        if (freq > max_freq)
            max_freq = freq;
    }
    if (max_freq == 0) max_freq = 1;

    int win_h, win_w;
	int dummy_var = 0 * win_w;
    getmaxyx(windows[UI_TOKEN_CHART], win_h, win_w);

    int chart_height = win_h - 4 + dummy_var;
    int col_width = 5;
    int x = 2;

    for (int t = 0; t < TOKEN_TYPE_CNT; t++) {
		if (t == TOKEN_TYPE_REPEAT ||
        t == TOKEN_TYPE_THEN ||
        t == TOKEN_TYPE_OPEN_CURLY ||
        t == TOKEN_TYPE_CLOSING_CURLY) {
        continue;
    }
        int freq = token_freq_get(t);
        if (freq > 0) {
            int bar_height = (freq * chart_height) / max_freq;

            for (int y = 0; y < bar_height; y++) {
                mvwaddch(windows[UI_TOKEN_CHART],
                         win_h - 2 - y,
                         x,
                         ACS_CKBOARD);
            }

            mvwprintw(windows[UI_TOKEN_CHART], win_h - 1, x, "%.3s",
                      token_get_name_forchart(t));

            mvwprintw(windows[UI_TOKEN_CHART], win_h - 2 - bar_height - 1,
                      x, "%d", freq);

            x += col_width;
        }
    }

    mvwprintw(windows[UI_TOKEN_CHART], 2, 2, "Total tokens parsed: %d", 
              max_freq > 0 ? max_freq : 0);

    wrefresh(windows[UI_TOKEN_CHART]);
}

static void refresh_ui()
{
	draw_bounds();
	draw_program();
	draw_log();
	draw_token_chart();
}

void onStartup(File_Context* ctx_in)
{
	initscr();
	clear();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);

	initColors();
	create_all_windows();

	ctx = ctx_in;
	selected_line = 0;
	int ch;
	refresh_ui();

	while ((ch = getch()) != 'q') {
		switch (ch) {
		case KEY_UP:
			selected_line = (selected_line + ctx->line_num - 1) %
					ctx->line_num;
			break;

		case KEY_DOWN:
			selected_line = (selected_line + 1) % ctx->line_num;
			break;
		// case 'p':
		// 	mid_x =(mid_x == min_x)?LERP(min_x,max_x,0.4):min_x;
		// 	break;
		case 'l':
			mid_y =(mid_y == max_y)?LERP(min_y,max_y,0.5):max_y;
			break;
		case 'i':
			x_by4 = (x_by4 == mid_x)?LERP(mid_x, max_x, 0.5):mid_x;
			break;
		case 'm':
			x_by4 =(x_by4 == max_x)?LERP(mid_x, max_x, 0.5):max_x;
			break;
		case 'r':
			token_freq_reset();
			refresh_ui();
			break;
		}

		refresh_ui();
	}
}

void onShutdown()
{
    for (int i = 0; i < UI_CNT; i++) {
        delwin(bounds[i]);
        delwin(windows[i]);
    }
    endwin();
}
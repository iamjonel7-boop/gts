#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "ui.h"

static WINDOW *output_win;
static WINDOW *input_win;
static WINDOW *status_win;

void init_ui(void)
{
		initscr();
		cbreak();
		noecho();
		keypad(stdscr, TRUE);

		int max_y, max_x;
		getmaxyx(stdscr, max_y, max_x);

		output_win = newwin(max_y -4, max_x, 0, 0);
		status_win = newwin(2, max_x, max_y -4, 0);
		input_win = newwin(2, max_x, max_y -2, 0);

		scrollok(output_win, TRUE);
		wrefresh(output_win);
		wrefresh(status_win);
		wrefresh(input_win);

		wprintw(output_win, "Global Time Service Client\n");
		wprintw(output_win, "Type 'help' for available commands\n\n");
		wrefresh(output_win);
}

void cleanup_ui(void)
{
		delwin(output_win);
		delwin(input_win);
		delwin(status_win);
		endwin();
}

void display_message(const char *msg)
{
		wprintw(output_win, "%s\n", msg);
		wrefresh(output_win);
}

void update_status(const char *status)
{
		wclear(status_win);
		wprintw(status_win, "Status: %s", status);
		wrefresh(status_win);
}

char* get_user_input(const char *prompt)
{
		static char input[256];
		wclear(input_win);
		wprintw(input_win, "%s", prompt);
		wrefresh(input_win);

		echo();
		wgetnstr(input_win, input, sizeof(input) -1);
		noecho();

		return input;
}

void show_help(void)
{
		display_message("Available commands:");
		display_message("\ttime\t- Get current UTC time");
		display_message("\tcreate <name> <offset_h> <offset_m> <location>\t- Create timezone");
		display_message("\tlist\t- List your timezones");
		display_message("\tping\t- Send ping to server");
		display_message("\thelp\t- Show this help");
		display_message("\tquit\t- Exit client");
}

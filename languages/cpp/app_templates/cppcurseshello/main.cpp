
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ncurses.h>

void create_box(int y, int x, int w, int h)
{
	mvaddch(y, x, '+');
	mvaddch(y, x + w, '+');
	mvaddch(y + h, x, '+');
	mvaddch(y + h, x + w, '+');
	mvhline(y, x + 1, '-', w - 1);
	mvhline(y + h, x + 1, '-', w - 1);
	mvvline(y + 1, x, '|', h - 1);
	mvvline(y + 1, x + w, '|', h - 1);
}

int main()
{
	int startx, starty, height, width;
	
	initscr();
	start_color();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();

	init_pair(1, COLOR_BLACK, COLOR_CYAN);
	
	height = 2;
	width = 30;
	starty = (LINES - height)/2;
	startx = (COLS - width)/2;

	attron(COLOR_PAIR(1));
	
	create_box(starty, startx, width, height);
	mvprintw(starty, startx + 3, " Hello World! " );
	mvprintw(starty+1,startx+1," Type any char to exit       ");
	mvprintw(0,0,"");
	refresh();
	getch();
	
	endwin();
	return 0;
}

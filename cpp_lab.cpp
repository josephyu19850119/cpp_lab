#include <iostream>
using namespace std;

#include <curses.h>

int main(int argc, char **argv)
{
    bool suspend = false;

    initscr();
    noecho();
    nodelay(stdscr, true);

    atexit([]()
           { endwin(); });

    while (true)
    {
        switch (getch())
        {
        case ' ':
            suspend = !suspend;
            nodelay(stdscr, !suspend);
            break;
        case '\n':
            return 0;
        }
        cout << "tick";
    }

    return 0;
}
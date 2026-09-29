#include "system_utils.hpp"
#include <signal.h>
#include <cstdlib>
#include <cstring>
#include <locale.h>  

#ifdef _WIN32
    #include <pdcurses.h>
    #include <winsock2.h>
    #define closesocket closesocket
#else
    #include <ncurses.h>
    #include <unistd.h>
    #define closesocket close
    typedef int SOCKET;
    const int INVALID_SOCKET = -1;
#endif

extern SOCKET matchmaking_socket;

void signal_handler(int sig) {
    endwin();
    
    #ifdef _WIN32
        WSACleanup();
    #endif
    
    exit(0);
}

void setupWindow() {
    setlocale(LC_ALL, "");  
    
    WINDOW *win = initscr();
    
    if (win == NULL) {
        printf("ERROR: initscr() failed!\n");
        exit(1);
    }
    
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    
    signal(SIGINT, signal_handler);
    attron(COLOR_PAIR(1));
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    drawTitle();
    refresh();
}

void drawTitle() {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    (void)maxY;
    
    const char* title[] = {
        "  ___            ___          _    _    _        ",
        " / __) ___  __ _| _ ) __ _ __| |_ | |_ | | ___   ",
        " \\__ \\/ -_)/ _` | _ \\/ _` (_-<  _||  _|| |/ -_)  ",
        " |___/\\___| \\__,_|___/\\__,_/__/\\__| \\__||_|\\___|  "
    };
    
    int titleWidth = strlen(title[0]);
    int startX = (maxX - titleWidth) / 2;
    
    for (int i = 0; i < 4; i++) {
        mvprintw(i, startX, title[i]);
    }
    
    mvprintw(5, (maxX - 50) / 2 - 1, "  ----------------------------------------------- ");
    mvprintw(6, (maxX - 50) / 2 - 1, " |          Choose your game mode below          |");
    mvprintw(7, (maxX - 50) / 2 - 1, "  ----------------------------------------------- ");
    
    refresh();
}
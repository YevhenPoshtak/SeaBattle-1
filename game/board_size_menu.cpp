#include "../ui/config.hpp"
#include "../ui/ui_helper.hpp"
#include <cstdio>

#ifdef _WIN32
    #include <pdcurses.h>
    #include <windows.h>
    #define SLEEP_MS(x) Sleep(x)
#else
    #include <ncurses.h>
    #include <unistd.h>
    #define SLEEP_MS(x) usleep((x) * 1000)
#endif

int selectBoardSize() {
    clear();
    
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    attron(COLOR_PAIR(1) | A_BOLD);     
    const char* title = "Select Board Size";
    mvprintw(2, (maxX - strlen(title)) / 2, "%s", title);
    attroff(A_BOLD);
    
    const char* hint1 = "Use UP/DOWN arrows or W/S to change size";
    const char* hint2 = "Press ENTER to confirm or ESC to cancel";
    mvprintw(4, (maxX - strlen(hint1)) / 2, "%s", hint1);
    mvprintw(5, (maxX - strlen(hint2)) / 2, "%s", hint2);
    
    int currentSize = getBoardSize(); 
    bool selecting = true;
    
    while (selecting) {
        attron(COLOR_PAIR(2) | A_BOLD);
        char sizeText[50];
        sprintf(sizeText, "Size: %dx%d", currentSize, currentSize);
        int centerX = (maxX - strlen(sizeText)) / 2;
        
        move(8, centerX - 5);
        for (int i = 0; i < 30; i++) printw(" ");
        
        mvprintw(8, centerX, "%s", sizeText);
        attroff(A_BOLD);
        
        attron(COLOR_PAIR(3));
        int barY = 10;
        int barStartX = (maxX - 40) / 2;
        int barWidth = 40;
        
        mvprintw(barY, barStartX, "[");
        mvprintw(barY, barStartX + barWidth - 1, "]");
        
        int filledWidth = ((currentSize - MIN_BOARD_SIZE) * (barWidth - 2)) / (MAX_BOARD_SIZE - MIN_BOARD_SIZE);
        for (int i = 0; i < barWidth - 2; i++) {
            if (i < filledWidth) {
                attron(COLOR_PAIR(2));
                mvprintw(barY, barStartX + 1 + i, "=");
            } else {
                attron(COLOR_PAIR(1));
                mvprintw(barY, barStartX + 1 + i, "-");
            }
        }
        attroff(COLOR_PAIR(3));
        
        attron(COLOR_PAIR(1));
        mvprintw(barY + 1, barStartX, "%d", MIN_BOARD_SIZE);
        char maxText[10];
        sprintf(maxText, "%d", MAX_BOARD_SIZE);
        mvprintw(barY + 1, barStartX + barWidth - strlen(maxText), "%s", maxText);
        
        int previewSize = 12;
        int previewY = 13;
        int previewX = (maxX - (previewSize * 2)) / 2;
        
        attron(COLOR_PAIR(3));
        mvprintw(previewY - 1, previewX + previewSize - 5, " Preview:");
        
        for (int i = 0; i < previewSize; i++) {
            move(previewY + i, previewX);
            for (int j = 0; j < previewSize; j++) {
                if ((i + j) % 2 == 0) {
                    printw("::");
                } else {
                    printw("..");
                }
            }
        }
        attroff(COLOR_PAIR(3));
        
        int reqY, reqX;
        getRequiredTerminalSize(currentSize, reqY, reqX);

        for (int i = 0; i < 5; i++) {
            move(previewY + previewSize + 2 + i, 0);
            clrtoeol();
            }

        attron(COLOR_PAIR(1));
        mvprintw(previewY + previewSize + 2, 2, "Required terminal size: %dx%d", reqX, reqY);
        mvprintw(previewY + previewSize + 3, 2, "Current terminal size:  %dx%d", maxX, maxY);
        
        if (canFitInterface(currentSize, maxY, maxX)) {
            attron(COLOR_PAIR(2));
            mvprintw(previewY + previewSize + 4, 2, "[OK] Interface will fit in terminal");
            attroff(COLOR_PAIR(2));
        } else {
            attron(COLOR_PAIR(4));
            mvprintw(previewY + previewSize + 4, 2, "[X] Interface will NOT fit - increase terminal size!");
            attroff(COLOR_PAIR(4));
        }
        
        refresh();
        
        flushinp();
        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'w':
            case 'W':
                if (currentSize < MAX_BOARD_SIZE) {
                    currentSize++;
                }
                break;
                
            case KEY_DOWN:
            case 's':
            case 'S':
                if (currentSize > MIN_BOARD_SIZE) {
                    currentSize--;
                }
                break;
                
            case KEY_RIGHT:
            case 'd':
            case 'D':
                if (currentSize < MAX_BOARD_SIZE) {
                    currentSize++;
                }
                break;
                
            case KEY_LEFT:
            case 'a':
            case 'A':
                if (currentSize > MIN_BOARD_SIZE) {
                    currentSize--;
                }
                break;
                
            case 10:
            case ' ':
                if (canFitInterface(currentSize, maxY, maxX)) {
                    setBoardSize(currentSize);
                    clear();
                    return currentSize;
                } else {
                    attron(COLOR_PAIR(4) | A_BOLD);
                    const char* warning = "Please increase terminal window size!";
                    mvprintw(previewY + previewSize + 6, (maxX - strlen(warning)) / 2, "%s", warning);
                    attroff(A_BOLD);
                    refresh();
                    SLEEP_MS(1500);
                    move(previewY + previewSize + 6, 0);
                    clrtoeol();
                }
                break;
                
            case 27: 
            case 'q':
            case 'Q':
                clear();
                return -1; 
                
            default:
                break;
        }
    }
    
    return currentSize;
}

char showBoardSizeQuickMenu() {
    clear();
    
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    (void)maxY;
    
    const char* title = "Select Board Size";
    mvprintw(6, (maxX - strlen(title)) / 2, "%s", title);
    
    int startY = 8;
    int startX = (maxX - 30) / 2;
    
    mvprintw(startY, startX, "1) 10x10 (Standard)");
    mvprintw(startY + 1, startX, "2) 15x15");
    mvprintw(startY + 2, startX, "3) 20x20");
    mvprintw(startY + 3, startX, "4) 26x26 (Maximum)");
    mvprintw(startY + 4, startX, "5) Custom size");
    mvprintw(startY + 5, startX, "ESC) Back to menu");
    
    refresh();
    
    while (true) {
        flushinp();
        int ch = getch();
        switch (ch) {
            case '1':
                setBoardSize(10);
                clear();
                return '1';
            case '2':
                setBoardSize(15);
                clear();
                return '2';
            case '3':
                setBoardSize(20);
                clear();
                return '3';
            case '4':
                setBoardSize(26);
                clear();
                return '4';
            case '5':
                if (selectBoardSize() != -1) {
                    return '5';
                }
                clear();
                return 0;
            case 27: 
            case 'q':
            case 'Q':
                clear();
                return 0;
        }
    }
}
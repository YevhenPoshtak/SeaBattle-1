#ifndef GAME_SETTINGS_HPP
#define GAME_SETTINGS_HPP

#include "ship_config.hpp"
#include "../ui/config.hpp"

#ifdef _WIN32
    #include <pdcurses.h>
#else
    #include <ncurses.h>
#endif

#include <algorithm>
#include <cstring>
#include <cstdlib>

struct GameSettings {
    int shotsPerTurn;  
    
    GameSettings() : shotsPerTurn(3) {}
};

inline int selectShotsPerTurn(int boardSize) {
    auto config = getShipConfig(boardSize);
    int recommended = config.shotsPerTurn;
    
    clear();
    
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    (void)maxY;
        
    attron(COLOR_PAIR(1) | A_BOLD);
    const char* title = "Select Shots Per Turn";
    mvprintw(2, (maxX - strlen(title)) / 2, "%s", title);
    attroff(A_BOLD);
    
    char info[100];
    sprintf(info, "Board size: %dx%d", boardSize, boardSize);
    mvprintw(4, (maxX - strlen(info)) / 2, "%s", info);
    
    sprintf(info, "Recommended: %d shots", recommended);
    attron(COLOR_PAIR(2));
    mvprintw(5, (maxX - strlen(info)) / 2, "%s", info);
    attroff(COLOR_PAIR(2));
    
    const char* hint1 = "Use UP/DOWN or W/S to change by 1";
    const char* hint2 = "Use LEFT/RIGHT or A/D to change by 5";
    const char* hint3 = "Press 'm' to enter custom value (1-26)";
    const char* hint4 = "Press ENTER to confirm";
    mvprintw(7, (maxX - strlen(hint1)) / 2, "%s", hint1);
    mvprintw(8, (maxX - strlen(hint2)) / 2, "%s", hint2);
    mvprintw(9, (maxX - strlen(hint3)) / 2, "%s", hint3);
    mvprintw(10, (maxX - strlen(hint4)) / 2, "%s", hint4);
    
    int currentShots = 3;
    int minShots = 1;
    int maxShots = 26;  
    
    bool selecting = true;
    
    while (selecting) {
        attron(COLOR_PAIR(2) | A_BOLD);
        char shotsText[50];
        sprintf(shotsText, "Shots: %d", currentShots);
        int centerX = (maxX - strlen(shotsText)) / 2;
        
        move(13, centerX - 10);
        for (int i = 0; i < 40; i++) printw(" ");
            
        mvprintw(13, centerX, "%s", shotsText);
        attroff(A_BOLD);
            
        int barY = 15;
        int barWidth = 50;
        int barStartX = (maxX - barWidth) / 2;
            
        attron(COLOR_PAIR(3));
        mvprintw(barY, barStartX, "[");
        mvprintw(barY, barStartX + barWidth - 1, "]"); 
        attroff(COLOR_PAIR(3));
        
        int innerWidth = barWidth - 2; 
        
        int filledWidth = 0;
        if (maxShots > minShots) {
            filledWidth = ((currentShots - minShots) * innerWidth) / (maxShots - minShots);
            
            if (currentShots == maxShots) filledWidth = innerWidth;
        }

        for (int i = 0; i < innerWidth; i++) {
            if (i < filledWidth) {
                attron(COLOR_PAIR(2));
                mvprintw(barY, barStartX + 1 + i, "=");
            } else {
                attron(COLOR_PAIR(1)); 
                mvprintw(barY, barStartX + 1 + i, "-");
            }
        }
        
        attron(COLOR_PAIR(1));
        mvprintw(barY + 1, barStartX, "%d", minShots); 
        
        char maxText[10];
        sprintf(maxText, "%d", maxShots);
        mvprintw(barY + 1, barStartX + barWidth - strlen(maxText), "%s", maxText); 
        
        const char* hint5 = "More shots = faster game, easier gameplay";
        const char* hint6 = "Fewer shots = longer game, more strategic";
        mvprintw(18, (maxX - strlen(hint5)) / 2, "%s", hint5);
        mvprintw(19, (maxX - strlen(hint6)) / 2, "%s", hint6);
        
        refresh();
        
        flushinp();
        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'w':
            case 'W':
                if (currentShots < maxShots) {
                    currentShots++;
                }
                break;
                
            case KEY_DOWN:
            case 's':
            case 'S':
                if (currentShots > minShots) {
                    currentShots--;
                }
                break;
                
            case KEY_RIGHT:
            case 'd':
            case 'D':
                currentShots = std::min(currentShots + 5, maxShots);
                break;
                
            case KEY_LEFT:
            case 'a':
            case 'A':
                currentShots = std::max(currentShots - 5, minShots);
                break;
                
            case 'm':
            case 'M':
                {
                    move(13, centerX - 10);
                    for (int i = 0; i < 40; i++) printw(" ");
                    
                    attron(COLOR_PAIR(5));
                    mvprintw(13, centerX - 5, "Enter shots (1-26): ");
                    attroff(COLOR_PAIR(5));
                    
                    echo();
                    curs_set(1);
                    
                    char input[10];
                    memset(input, 0, sizeof(input));
                    mvgetnstr(13, centerX + 15, input, 9);
                    
                    noecho();
                    curs_set(0);
                    
                    int value = atoi(input);
                    if (value >= minShots && value <= maxShots) {
                        currentShots = value;
                    } else if (value > maxShots) {
                        currentShots = maxShots;
                        attron(COLOR_PAIR(4));
                        mvprintw(21, (maxX - 30) / 2 + 6, "Max is 26! Set to 26.");
                        attroff(COLOR_PAIR(4));
                        refresh();
                        napms(1500);
                        move(21, 0);
                        clrtoeol();
                    } else if (value < minShots) {
                        currentShots = minShots;
                        attron(COLOR_PAIR(4));
                        mvprintw(21, (maxX - 30) / 2 + 5, "Min is 1! Set to 1.");
                        attroff(COLOR_PAIR(4));
                        refresh();
                        napms(1500);
                        move(21, 0);
                        clrtoeol();
                    }
                    
                    move(13, centerX - 15);
                    for (int i = 0; i < 40; i++) printw(" ");
                }
                break;
                
            case 10:
            case ' ':
                clear();
                return currentShots;
                
            case 27:
            case 'q':
            case 'Q':
                clear();
                return 3;
                
            default:
                break;
        }
    }
    
    return 3;
}

#endif
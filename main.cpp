#include "ui/config.hpp"
#include "ui/ui_helper.hpp"
#include "ui/animation.hpp"
#include "game/board_size_menu.hpp"
#include "game/board_layout.hpp"
#include "game/ship_config.hpp"
#include "game/game_settings.hpp"
#include "game/game_board.hpp"
#include "game/game_piece.hpp"
#include "user/user.hpp"
#include "ai/ai_player.hpp"
#include "utils/system_utils.hpp"
#include "utils/text_utils.hpp"
#include "utils/game_logic_utils.hpp"
#include <signal.h>
#include <locale.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>

using namespace std;

GameSettings g_gameSettings;

char startMenu();

int main(int argc, char **argv) {
    setlocale(LC_ALL, "");
    
    #ifdef _WIN32
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            printf("WSAStartup failed: %d\n", iResult);
            return 1;
        }
    #endif

    srand(time(NULL));
    setBoardSize(10);
    g_gameSettings.shotsPerTurn = 5;

    setupWindow();
    
    char ret = 'e';
    while (ret != 'h' && ret != 'c' && ret != 'a' && ret != 's') {
        for (int i = 8; i < 30; i++) {
            mvprintw(i, 0, "\n");
        }
        refresh();
        
        ret = startMenu();
        
        if (ret == 'a') {
            AIPlayer::playGame(EASY);
            clear();
            drawTitle();
            ret = 'e'; 
            
        } else if (ret == 's') {
            AIPlayer::playGame(SMART);
            clear();
            drawTitle();
            ret = 'e'; 
            
        } else if (ret == 'h') {
            for (int i = 7; i < 30; i++) {
                mvprintw(i, 0, "\n");
            }
            refresh();
            
            Host *h = new Host();
            h->connect(); 
            delete h;
            
            ret = 'e';
            
        } else if (ret == 'c') {
            for (int i = 7; i < 30; i++) {
                mvprintw(i, 0, "\n");
            }
            refresh();
            char *str = (char *) malloc(100);
            echo();
            mvprintw(8, 2, "Enter host IP address: ");
            getnstr(str, 100);
            noecho();
            
            Client *c = new Client(str);
            c->connect();
            delete c;
            free(str);

            ret = 'e';
        }
    }

    endwin();
    
    #ifdef _WIN32
        WSACleanup();
    #endif
    
    return 0;
}

char startMenu() {
    drawTitle();

    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    (void)maxY;
    
    string options[6] = {
        "1) vs Easy AI", 
        "2) vs Smart AI", 
        "3) Host (Multiplayer)", 
        "4) Client (Multiplayer)",
        "5) Board Size Settings",
        "6) Quit"
    };
    
    char ret = 'e';
    int i = 0, prev = 0;
    curs_set(0);
    
    int menuStartY = 10;
    int longestOption = 0;
    for (int j = 0; j < 6; j++) {
        if ((int)options[j].length() > longestOption) {
            longestOption = options[j].length();
        }
    }
    int menuStartX = (maxX - longestOption) / 2 + 3;
    int cursorX = menuStartX - 4; 
    
    char sizeInfo[50];
    sprintf(sizeInfo, "Current board: %dx%d", getBoardSize(), getBoardSize());
    attron(COLOR_PAIR(6));
    mvprintw(menuStartY - 2, (maxX - strlen(sizeInfo)) / 2 - 1, "%s", sizeInfo);
    attroff(COLOR_PAIR(6));
    
    for (int j = 0; j < 6; j++) {
        if (j == 0) {
            attron(COLOR_PAIR(2) | A_BOLD);
            mvprintw(menuStartY + j, cursorX, ">>>");
            attroff(COLOR_PAIR(2) | A_BOLD);
            attron(A_STANDOUT);
        }
        mvprintw(menuStartY + j, menuStartX, options[j].c_str());
        if (j == 0) {
            attroff(A_STANDOUT);
        }
    }
    
    nodelay(stdscr, TRUE);
    
    bool isActiveMenu = true;
    int frame = 0;
    
    while (isActiveMenu) {
        prev = i;
        int c = getch();
        
        if (c != ERR) {
            switch (c) {
                case KEY_UP:
                case 'w':
                case 'W':
                    i--;
                    i = (i < 0) ? 5 : i;
                    break;
                case KEY_DOWN:
                case 's':
                case 'S':
                    i++;
                    i = (i > 5) ? 0 : i;
                    break;
                case 10:
                case ' ':
                    nodelay(stdscr, FALSE);
                    switch (i) {
                        case 0:
                            if (!UIHelper::checkTerminalSize(getBoardSize())) {
                                UIHelper::showTerminalSizeError(getBoardSize());
                                clear();
                                drawTitle();
                                sprintf(sizeInfo, "Current board: %dx%d", getBoardSize(), getBoardSize());
                                attron(COLOR_PAIR(6));
                                mvprintw(menuStartY - 2, (maxX - strlen(sizeInfo)) / 2, "%s", sizeInfo);
                                attroff(COLOR_PAIR(6));
                                for (int j = 0; j < 6; j++) {
                                    mvprintw(menuStartY + j, menuStartX, options[j].c_str());
                                }
                                attron(COLOR_PAIR(2) | A_BOLD);
                                mvprintw(menuStartY + i, cursorX, ">>>");
                                attroff(COLOR_PAIR(2) | A_BOLD);
                                nodelay(stdscr, TRUE);
                                break;
                            }
                            ret = 'a'; 
                            isActiveMenu = false;
                            break;
                        case 1:
                            if (!UIHelper::checkTerminalSize(getBoardSize())) {
                                UIHelper::showTerminalSizeError(getBoardSize());
                                clear();
                                drawTitle();
                                sprintf(sizeInfo, "Current board: %dx%d", getBoardSize(), getBoardSize());
                                attron(COLOR_PAIR(6));
                                mvprintw(menuStartY - 2, (maxX - strlen(sizeInfo)) / 2, "%s", sizeInfo);
                                attroff(COLOR_PAIR(6));
                                for (int j = 0; j < 6; j++) {
                                    mvprintw(menuStartY + j, menuStartX, options[j].c_str());
                                }
                                attron(COLOR_PAIR(2) | A_BOLD);
                                mvprintw(menuStartY + i, cursorX, ">>>");
                                attroff(COLOR_PAIR(2) | A_BOLD);
                                nodelay(stdscr, TRUE);
                                break;
                            }
                            ret = 's'; 
                            isActiveMenu = false;
                            break;
                        case 2:
                            if (!UIHelper::checkTerminalSize(getBoardSize())) {
                                UIHelper::showTerminalSizeError(getBoardSize());
                                clear();
                                drawTitle();
                                sprintf(sizeInfo, "Current board: %dx%d", getBoardSize(), getBoardSize());
                                attron(COLOR_PAIR(6));
                                mvprintw(menuStartY - 2, (maxX - strlen(sizeInfo)) / 2, "%s", sizeInfo);
                                attroff(COLOR_PAIR(6));
                                for (int j = 0; j < 6; j++) {
                                    mvprintw(menuStartY + j, menuStartX, options[j].c_str());
                                }
                                attron(COLOR_PAIR(2) | A_BOLD);
                                mvprintw(menuStartY + i, cursorX, ">>>");
                                attroff(COLOR_PAIR(2) | A_BOLD);
                                nodelay(stdscr, TRUE);
                                break;
                            }
                            ret = 'h'; 
                            isActiveMenu = false;
                            break;
                        case 3:
                            if (!UIHelper::checkTerminalSize(getBoardSize())) {
                                UIHelper::showTerminalSizeError(getBoardSize());
                                clear();
                                drawTitle();
                                sprintf(sizeInfo, "Current board: %dx%d", getBoardSize(), getBoardSize());
                                attron(COLOR_PAIR(6));
                                mvprintw(menuStartY - 2, (maxX - strlen(sizeInfo)) / 2, "%s", sizeInfo);
                                attroff(COLOR_PAIR(6));
                                for (int j = 0; j < 6; j++) {
                                    mvprintw(menuStartY + j, menuStartX, options[j].c_str());
                                }
                                attron(COLOR_PAIR(2) | A_BOLD);
                                mvprintw(menuStartY + i, cursorX, ">>>");
                                attroff(COLOR_PAIR(2) | A_BOLD);
                                nodelay(stdscr, TRUE);
                                break;
                            }
                            ret = 'c'; 
                            isActiveMenu = false;
                            break;
                        case 4:
                            {
                                selectBoardSize();
                                clear();
                                drawTitle();
                                
                                sprintf(sizeInfo, "Current board: %dx%d", getBoardSize(), getBoardSize());
                                attron(COLOR_PAIR(6));
                                mvprintw(menuStartY - 2, (maxX - strlen(sizeInfo)) / 2, "%s", sizeInfo);
                                attroff(COLOR_PAIR(6));
                                
                                for (int j = 0; j < 6; j++) {
                                    mvprintw(menuStartY + j, menuStartX, options[j].c_str());
                                }
                                attron(COLOR_PAIR(2) | A_BOLD);
                                mvprintw(menuStartY + i, cursorX, ">>>");
                                attroff(COLOR_PAIR(2) | A_BOLD);
                                nodelay(stdscr, TRUE);
                            }
                            break;
                        case 5:
                            endwin();
                            #ifdef _WIN32
                                WSACleanup();
                            #endif
                            exit(0);
                    }
            }
            
            mvprintw(menuStartY + prev, cursorX, "   ");
            attroff(A_STANDOUT);
            mvprintw(menuStartY + prev, menuStartX, options[prev].c_str());
            
            attron(COLOR_PAIR(2) | A_BOLD);
            mvprintw(menuStartY + i, cursorX, ">>>");
            attroff(COLOR_PAIR(2) | A_BOLD);
            attron(A_STANDOUT);
            mvprintw(menuStartY + i, menuStartX, options[i].c_str());
            attroff(A_STANDOUT);
        }
        
        drawMenuAnimation(frame);
        refresh();
        
        #ifdef _WIN32
            Sleep(100);
        #else
            usleep(100000);
        #endif
        
        frame++;
        if (frame >= 60) {
            frame = 0;
        }
    }
    
    nodelay(stdscr, FALSE);
    curs_set(2);
    return ret;
}
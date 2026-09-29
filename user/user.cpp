#include "user.hpp"
#include <stdio.h>
#include <cmath>
#include "../ui/config.hpp"
#include "../game/board_layout.hpp"
#include "../game/ship_config.hpp"
#include "../game/game_settings.hpp"
#include "../game/board_size_menu.hpp"
#include "../ui/animation.hpp"
#include "../ui/ui_helper.hpp"
#include "../utils/game_logic_utils.hpp"
#include "../utils/text_utils.hpp"
#include <queue> 

extern GameSettings g_gameSettings;

std::queue<string> logQueue;

void User::gameLoop(SOCKET client_socket) {
    clear();
    
    int size;
    int shots;
    
    if (isHost) {
        size = getBoardSize();
        shots = selectShotsPerTurn(size);
        
        send(client_socket, (const char*)&size, sizeof(int), 0);
        send(client_socket, (const char*)&shots, sizeof(int), 0);
        
        g_gameSettings.shotsPerTurn = shots;
        
        clear();
        mvprintw(2, 2, "Multiplayer Game (Host)");
        mvprintw(3, 2, "Board: %dx%d | Shots: %d per turn", size, size, shots);
        mvprintw(4, 2, "Waiting for client to setup board...");
        refresh();
    } else {
        clear();
        mvprintw(2, 2, "Multiplayer Game (Client)");
        mvprintw(3, 2, "Waiting for host settings...");
        refresh();
        
        if (recv(client_socket, (char*)&size, sizeof(int), MSG_WAITALL) <= 0) {
            attrset(A_NORMAL); 
            clear(); 
            mvprintw(5, 2, "Error: Connection lost!");
            mvprintw(6, 2, "Press any key to exit...");
            refresh();
            flushinp(); 
            getch();
            clear(); 
            closesocket(client_socket);
            return;
        }
        
        if (recv(client_socket, (char*)&shots, sizeof(int), MSG_WAITALL) <= 0) {
            attrset(A_NORMAL); 
            clear(); 
            mvprintw(5, 2, "Error: Connection lost!");
            mvprintw(6, 2, "Press any key to exit...");
            refresh();
            flushinp(); 
            getch();
            clear(); 
            closesocket(client_socket);
            return;
        }
        
        g_gameSettings.shotsPerTurn = shots;
        
        clear();
        mvprintw(2, 2, "Multiplayer Game (Client)");
        mvprintw(3, 2, "Board: %dx%d | Shots: %d per turn", size, size, shots);
        mvprintw(4, 2, "Host has chosen the settings!");
        refresh();
        
        #ifdef _WIN32
            Sleep(2000);
        #else
            usleep(2000000);
        #endif
    }
    
    setBoardSize(size);
    
    Gameboard myBoard;
    Gameboard enemyBoard;  
    
    int shotsPerTurn = g_gameSettings.shotsPerTurn;
    int maxHits = getTotalShipCells(size);
    int totalShips = getTotalShips(size);

    int boardResult = 0;
    while (boardResult != 1) {
        boardResult = myBoard.generateRandomBoard(isHost);
        
        if (boardResult == 0) {
            boardResult = myBoard.generateManualBoard();
        }
    }

    BoardLayout layout = calculateBoardLayout(size);
    
    int boardWidth = size * 4 + 8;
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    clear();
    
    const char* yourBoardTitle;
    const char* oppBoardTitle;
    
    if (size >= 20) {
        yourBoardTitle = "You";
        oppBoardTitle = "Opp";
    } else if (size >= 15) {
        yourBoardTitle = "Your";
        oppBoardTitle = "Opponent";
    } else {
        yourBoardTitle = "Your Board";
        oppBoardTitle = "Opp. Board";
    }
    
    int yourBoardLen = strlen(yourBoardTitle);
    int oppBoardLen = strlen(oppBoardTitle);
    
    int leftPad1 = (boardWidth - yourBoardLen) / 2;
    int rightPad1 = boardWidth - leftPad1 - yourBoardLen;
    int leftPad2 = (boardWidth - oppBoardLen) / 2;
    int rightPad2 = boardWidth - leftPad2 - oppBoardLen;
    
    move(layout.startY, layout.board1StartX);
    for (int i = 0; i < leftPad1; i++) printw("-");
    printw("%s", yourBoardTitle);
    for (int i = 0; i < rightPad1; i++) printw("-");
    
    move(layout.startY, layout.separatorX);
    printw("~~~~~");
    
    move(layout.startY, layout.board2StartX);
    for (int i = 0; i < leftPad2; i++) printw("-");
    printw("%s", oppBoardTitle);
    for (int i = 0; i < rightPad2; i++) printw("-");
    printw("\n");

    move(layout.startY + 1, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("_");
    move(layout.startY + 1, layout.separatorX);
    printw("~~~~~");
    move(layout.startY + 1, layout.board2StartX);
    for (int i = 0; i < boardWidth; i++) printw("_");
    printw("\n");

    move(layout.startY + 2, layout.board1StartX);
    printw("|  | A |");
    for (int i = 1; i < size; i++) {
        printw(" %c |", 'A' + i);
    }
    
    move(layout.startY + 2, layout.separatorX);
    printw("~~~~~");
    
    move(layout.startY + 2, layout.board2StartX + 4);
    printw("|  | A |");
    for (int i = 1; i < size; i++) {
        printw(" %c |", 'A' + i);
    }
    printw("\n");

    for (int i = 0; i < size; i++) {
        move(layout.startY + 3 + i, layout.board1StartX);
        printw("|%2d|", i + 1);
        for (int j = 0; j < size; j++) {
            printw("   |");
        }
        
        move(layout.startY + 3 + i, layout.separatorX);
        printw("~~~~~");
        
        move(layout.startY + 3 + i, layout.board2StartX + 4);
        printw("|%2d|", i + 1);
        for (int j = 0; j < size; j++) {
            printw("   |");
        }
    }

    move(layout.startY + 3 + size, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("-");
    move(layout.startY + 3 + size, layout.separatorX);
    printw("~~~~~");
    move(layout.startY + 3 + size, layout.board2StartX);
    for (int i = 0; i < boardWidth; i++) printw("-");
    printw("\n");
    
    attron(A_UNDERLINE);
    mvprintw(1, 1, "instructions");
    attroff(A_UNDERLINE);
    
    mvprintw(2, 1, "w/up - up      a/left - left");
    mvprintw(3, 1, "s/down - down    d/right - right");
    mvprintw(4, 1, "space/enter - select target");
    mvprintw(5, 1, "f - fire all shots");
    mvprintw(6, 1, "q - quit game");
    
    refresh();

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (myBoard.boardArray[i][j] != 'w') {
                attron(COLOR_PAIR(2));
                move(layout.startY + 3 + i, layout.board1StartX + 5 + (4 * j));
                addch(toupper(myBoard.boardArray[i][j]));
            }
        }
    }
    attron(COLOR_PAIR(1));
    
    cursor.y = layout.startY + 3;
    cursor.x = layout.board2StartX + 9;
    int maxCursorX = cursor.x + (size - 1) * 4;
    int maxCursorY = cursor.y + size - 1;
    
    move(cursor.y, cursor.x);
    refresh();
    
    int playerHits = 0;
    int enemyHits = 0;
    bool playerTurn = isHost;
    
    int myShipsRemaining = totalShips;
    int enemyShipsRemaining = totalShips;
    
    std::vector<std::vector<char>> enemyKnownBoard(size, std::vector<char>(size, ' '));
    
    int grid_x = 0, grid_y = 0;
    
    struct PendingShot {
        int x, y;
        bool used;
    };
    
    std::vector<PendingShot> playerShots;
    playerShots.resize(shotsPerTurn);
    for (int i = 0; i < shotsPerTurn; i++) {
        playerShots[i].used = false;
    }
    
    int shotsSelected = 0;
    bool selectingMode = true;
    
    int animFrame = 0;
    int animStartY = maxY - 6; 
    
    int playerStatsY = layout.startY + 3 + size + 2;  
    int enemyStatsY = layout.startY + 3 + size + 5;   
    
    while (playerHits < maxHits && enemyHits < maxHits) {
        move(0, maxX - 35);
        clrtoeol();
        attron(COLOR_PAIR(5) | A_BOLD);
        printw("YOUR SHIPS: %d", myShipsRemaining);
        attroff(A_BOLD);
        
        move(0, maxX - 15);
        attron(COLOR_PAIR(6) | A_BOLD);
        printw("ENEMY: %d", enemyShipsRemaining);
        attroff(A_BOLD);
        attron(COLOR_PAIR(1));
        
        if (animStartY > layout.startY + 3 + size + 5) {
            int animationStartX = 10;
            int animationEndX = maxX - 10;
    
            for (int clearY = animStartY - 4; clearY <= animStartY + 6; clearY++) {
                move(clearY, animationStartX);
                for (int x = animationStartX; x < animationEndX; x++) {
                    addch(' ');
                }
            }   
            
            int cycleFrame = animFrame % 80;
            
            int yellowShipX = 15 + (cycleFrame / 2);
            int blueShipX = maxX - 25 - (cycleFrame / 2);
            
            attron(COLOR_PAIR(3));
            for (int i = 0; i < maxX; i += 2) {
                int waveY = animStartY + 4 + (int)(sin((i + animFrame * 0.5) * 0.2) * 1.5);
                if (waveY >= 0) {
                    mvaddch(waveY, i, '~');
                    if (i + 1 < maxX) {
                        mvaddch(waveY, i + 1, '~');
                    }
                }
            }
            attroff(COLOR_PAIR(3));
            
            int phase = (cycleFrame / 20);
            
            if (phase == 0 || phase == 1) {
                attron(COLOR_PAIR(5));
                mvprintw(animStartY - 2, yellowShipX, "    _~_");
                mvprintw(animStartY - 1, yellowShipX, "   /___\\");
                mvprintw(animStartY,     yellowShipX, "  |=====|>");
                mvprintw(animStartY + 1, yellowShipX, " /~~~~~~~\\");
                mvprintw(animStartY + 2, yellowShipX, "~~~~~~~~~~~");
                attroff(COLOR_PAIR(5));
                
                attron(COLOR_PAIR(6));
                mvprintw(animStartY - 2, blueShipX, " _~_");
                mvprintw(animStartY - 1, blueShipX, "/___\\");
                mvprintw(animStartY,     blueShipX, "<|=====|");
                mvprintw(animStartY + 1, blueShipX, "/~~~~~~~\\");
                mvprintw(animStartY + 2, blueShipX, "~~~~~~~~~~~");
                attroff(COLOR_PAIR(6));
                
            } else if (phase == 2) {
                attron(COLOR_PAIR(5));
                mvprintw(animStartY - 2, yellowShipX, "    _~_");
                mvprintw(animStartY - 1, yellowShipX, "   /___\\");
                mvprintw(animStartY,     yellowShipX, "  |=====|>");
                mvprintw(animStartY + 1, yellowShipX, " /~~~~~~~\\");
                mvprintw(animStartY + 2, yellowShipX, "~~~~~~~~~~~");
                attroff(COLOR_PAIR(5));
                
                int projectileProgress = cycleFrame % 20;
                int projectileX = yellowShipX + 11 + (projectileProgress * (blueShipX - yellowShipX - 15) / 20);
                if (projectileX < blueShipX - 2) {
                    attron(COLOR_PAIR(2));
                    mvprintw(animStartY, projectileX, "===>");
                    attroff(COLOR_PAIR(2));
                }
                
                attron(COLOR_PAIR(6));
                mvprintw(animStartY - 2, blueShipX, " _~_");
                mvprintw(animStartY - 1, blueShipX, "/___\\");
                mvprintw(animStartY,     blueShipX, "<|=====|");
                mvprintw(animStartY + 1, blueShipX, "/~~~~~~~\\");
                mvprintw(animStartY + 2, blueShipX, "~~~~~~~~~~~");
                attroff(COLOR_PAIR(6));
                
            } else if (phase == 3) {
                attron(COLOR_PAIR(5));
                mvprintw(animStartY - 2, yellowShipX, "    _~_");
                mvprintw(animStartY - 1, yellowShipX, "   /___\\");
                mvprintw(animStartY,     yellowShipX, "  |=====|>");
                mvprintw(animStartY + 1, yellowShipX, " /~~~~~~~\\");
                mvprintw(animStartY + 2, yellowShipX, "~~~~~~~~~~~");
                attroff(COLOR_PAIR(5));
                
                int explosionFrame = cycleFrame % 20;
                if (explosionFrame < 10) {
                    attron(COLOR_PAIR(4));
                    mvprintw(animStartY - 3, blueShipX - 2, "  * * *");
                    mvprintw(animStartY - 2, blueShipX - 2, " * * * *");
                    mvprintw(animStartY - 1, blueShipX - 2, "* BOOM *");
                    mvprintw(animStartY,     blueShipX - 2, "* * * * *");
                    mvprintw(animStartY + 1, blueShipX - 2, " * * * *");
                    mvprintw(animStartY + 2, blueShipX - 2, "  * * *");
                    attroff(COLOR_PAIR(4));
                } else {
                    attron(COLOR_PAIR(1));
                    mvprintw(animStartY - 1, blueShipX, " . . .");
                    mvprintw(animStartY,     blueShipX, ". . . .");
                    mvprintw(animStartY + 1, blueShipX, " . . .");
                    attroff(COLOR_PAIR(1));
                }
            }
        }
        
        if (playerTurn) {
            if (selectingMode) {
                move(1, 82);
                clrtoeol();
                attron(COLOR_PAIR(6));
                char msg[70];
                sprintf(msg, "Select %d (or less) targets (%d/%d) - F to fire", 
                        shotsPerTurn, shotsSelected, shotsPerTurn);
                printw("%s", msg);
                attron(COLOR_PAIR(1));
                move(cursor.y, cursor.x);
                refresh();
                
                #ifdef _WIN32
                    Sleep(50);
                #else
                    usleep(50000);
                #endif
                
                animFrame++;
                if (animFrame >= 80) animFrame = 0;
                
                nodelay(stdscr, TRUE);
                int key = getch();
                nodelay(stdscr, FALSE);
                
                if (key == ERR) continue;
                
                flushinp();
                
                switch (key) {
                    case KEY_LEFT:
                    case 'a':
                    case 'A':
                        if (cursor.x > layout.board2StartX + 9 && grid_x > 0) {
                            cursor.x -= 4;
                            grid_x--;
                        }
                        break;
                    case KEY_RIGHT:
                    case 'd':
                    case 'D':
                        if (cursor.x < maxCursorX && grid_x < size - 1) {
                            cursor.x += 4;
                            grid_x++;
                        }
                        break;
                    case KEY_UP:
                    case 'w':
                    case 'W':
                        if (cursor.y > layout.startY + 3 && grid_y > 0) {
                            cursor.y -= 1;
                            grid_y--;
                        }
                        break;
                    case KEY_DOWN:
                    case 's':
                    case 'S':
                        if (cursor.y < maxCursorY && grid_y < size - 1) {
                            cursor.y += 1;
                            grid_y++;
                        }
                        break;
                    case ' ':
                    case 10:
                        if (enemyKnownBoard[grid_y][grid_x] == ' ' && shotsSelected < shotsPerTurn) {
                            bool alreadySelected = false;
                            for (int i = 0; i < shotsSelected; i++) {
                                if (playerShots[i].x == grid_x && playerShots[i].y == grid_y) {
                                    alreadySelected = true;
                                    break;
                                }
                            }
                            
                            if (!alreadySelected) {
                                playerShots[shotsSelected].x = grid_x;
                                playerShots[shotsSelected].y = grid_y;
                                playerShots[shotsSelected].used = true;
                                shotsSelected++;
                                
                                attron(COLOR_PAIR(5) | A_BOLD);
                                move(cursor.y, cursor.x - 1);
                                addch('[');
                                move(cursor.y, cursor.x);
                                addch('+');
                                move(cursor.y, cursor.x + 1);
                                addch(']');
                                attron(COLOR_PAIR(1));
                            }
                        }
                        break;
                    case 'f':
                    case 'F':
                        if (shotsSelected > 0) {
                            selectingMode = false;
                        }
                        break;
                    case 'q':
                    case 'Q':
                        closesocket(client_socket);
                        return;
                }
                
                move(cursor.y, cursor.x);
                refresh();
                
            } else {
                move(1, 82);
                clrtoeol();
                attron(COLOR_PAIR(4) | A_BOLD);
                printw("                    FIRING!                                    ");
                attron(COLOR_PAIR(1));
                refresh();
                
                for (int i = 0; i < 3; i++) {
                    move(playerStatsY + i, layout.board1StartX);
                    clrtoeol();
                }
                
                attron(A_UNDERLINE | COLOR_PAIR(2));
                mvprintw(playerStatsY, layout.board1StartX, "Your volley:");
                attroff(A_UNDERLINE | COLOR_PAIR(2));
                attron(COLOR_PAIR(1));
                
                std::vector<std::string> volleyCoords;
                std::vector<coordinates> shotsFiredInVolley;
                int missInVolley = 0;
                int sunkInVolley = 0;
                
                send(client_socket, (const char*)&shotsSelected, sizeof(int), 0);

                for (int i = 0; i < shotsSelected; i++) {
                    coordinates shot;
                    shot.x = playerShots[i].x;
                    shot.y = playerShots[i].y;
                    shotsFiredInVolley.push_back(shot);
                    
                    char coordBuf[16];
                    sprintf(coordBuf, "%c%d", 'A' + shot.x, shot.y + 1);
                    volleyCoords.push_back(std::string(coordBuf));
                    
                    send(client_socket, (const char*)&shot, sizeof(shot), 0);
                    char answer;
                    if (recv(client_socket, &answer, sizeof(char), MSG_WAITALL) <= 0) {
                        attrset(A_NORMAL); 
                        clear(); 
                        mvprintw(5, 2, "Error: Connection lost!");
                        mvprintw(6, 2, "Press any key to exit...");
                        refresh();
                        flushinp(); 
                        getch();
                        clear(); 
                        closesocket(client_socket);
                        return;
                    }
                    
                    int shot_y = layout.startY + 3 + shot.y;
                    int shot_x = layout.board2StartX + 9 + (4 * shot.x);
                    
                    attron(COLOR_PAIR(1));
                    move(shot_y, shot_x - 1); addch(' ');
                    move(shot_y, shot_x + 1); addch(' ');
                    
                    if (answer == 'h') {
                        enemyBoard.boardArray[shot.y][shot.x] = 'x'; 
                        playerHits++;
                        
                        attron(COLOR_PAIR(4));
                        move(shot_y, shot_x); addch('X'); 
                        enemyKnownBoard[shot.y][shot.x] = 'h';
                    } else if (answer == 'm') {
                        enemyBoard.boardArray[shot.y][shot.x] = 'o'; 
                        missInVolley++;
                        
                        attron(COLOR_PAIR(3));
                        move(shot_y, shot_x); addch('O'); 
                        enemyKnownBoard[shot.y][shot.x] = 'm';
                    } else { 
                        enemyBoard.boardArray[shot.y][shot.x] = 'x'; 
                        playerHits++;
                        enemyShipsRemaining--;
                        sunkInVolley++;
                        
                        attron(COLOR_PAIR(4) | A_BOLD);
                        move(shot_y, shot_x); addch('S'); 
                        enemyBoard.boardArray[shot.y][shot.x] = 's';
                        enemyKnownBoard[shot.y][shot.x] = 's';
                        
                        std::queue<std::pair<int, int>> updateQueue;
                        updateQueue.push({shot.x, shot.y});
                        
                        while (!updateQueue.empty()) {
                            std::pair<int, int> current = updateQueue.front();
                            updateQueue.pop();
                            
                            int cx = current.first;
                            int cy = current.second;
                            
                            int dx[] = {0, 0, -1, 1};
                            int dy[] = {-1, 1, 0, 0};
                            
                            for (int k = 0; k < 4; k++) {
                                int nx = cx + dx[k];
                                int ny = cy + dy[k];
                                
                                if (nx >= 0 && nx < size && ny >= 0 && ny < size) {
                                    if (enemyBoard.boardArray[ny][nx] == 'x') {
                                        enemyBoard.boardArray[ny][nx] = 's'; 
                                        enemyKnownBoard[ny][nx] = 's';
                                        
                                        int update_y = layout.startY + 3 + ny;
                                        int update_x = layout.board2StartX + 9 + (4 * nx);
            
                                        move(update_y, update_x); 
                                        addch('S'); 
                                        updateQueue.push({nx, ny});
                                    }
                                }
                            }
                        }
                    }
                    attron(COLOR_PAIR(1));
                    refresh();
                    
                    #ifdef _WIN32
                        Sleep(400);
                    #else
                        usleep(400000);
                    #endif
                }

                int countWounded = 0;
                for (const auto& shot : shotsFiredInVolley) {
                    if (enemyBoard.boardArray[shot.y][shot.x] == 'x') {
                        countWounded++;
                    }
                }
                int countHits = countWounded;
                int countSunk = sunkInVolley; 
                int countMiss = missInVolley;

                std::string coordsStr = "";
                for (size_t i = 0; i < volleyCoords.size(); i++) {
                    coordsStr += volleyCoords[i];
                    if (i < volleyCoords.size() - 1) coordsStr += ",";
                }
                
                std::string statsStr = " - ";
                bool hasStats = false;
                
                if (countHits > 0) {
                    statsStr += std::to_string(countHits) + " wounded";
                    hasStats = true;
                }
                if (countSunk > 0) {
                    if (hasStats) statsStr += ", ";
                    statsStr += std::to_string(countSunk) + " sunk";
                    hasStats = true;
                }
                if (countMiss > 0) {
                    if (hasStats) statsStr += ", ";
                    statsStr += std::to_string(countMiss) + " miss";
                    hasStats = true;
                }
                
                mvprintw(playerStatsY + 1, layout.board1StartX, "%s%s", coordsStr.c_str(), statsStr.c_str());
                refresh();
                
                if (playerHits >= maxHits) {
                    drawFirework(true);        
                    closesocket(client_socket);
                    return;
                }
                
                shotsSelected = 0;
                selectingMode = true;
                playerTurn = false;
                
                for (int i = 0; i < size; i++) {
                    for (int j = 0; j < size; j++) {
                        if (enemyKnownBoard[i][j] == ' ') continue;
                        
                        int cell_y = layout.startY + 3 + i;
                        int cell_x = layout.board2StartX + 9 + (4 * j);
                        
                        if (enemyKnownBoard[i][j] == 's') {
                            attron(COLOR_PAIR(4) | A_BOLD);
                            move(cell_y, cell_x); addch('S');
                        } else if (enemyKnownBoard[i][j] == 'h') {
                            attron(COLOR_PAIR(4));
                            move(cell_y, cell_x); addch('X');
                        } else if (enemyKnownBoard[i][j] == 'm') {
                            attron(COLOR_PAIR(3));
                            move(cell_y, cell_x); addch('O');
                        }
                        attron(COLOR_PAIR(1));
                    }
                }
            }
        } else {
            move(1, 90);
            clrtoeol();
            attron(COLOR_PAIR(5));
            printw("         Enemy's turn...                     ");
            attron(COLOR_PAIR(1));
            refresh();
            
            #ifdef _WIN32
                Sleep(1000);
            #else
                usleep(1000000);
            #endif
            
            for (int i = 0; i < 3; i++) {
                move(enemyStatsY + i, layout.board1StartX);
                clrtoeol();
            }
            
            attron(A_UNDERLINE | COLOR_PAIR(4));
            mvprintw(enemyStatsY, layout.board1StartX, "Enemy volley:");
            attroff(A_UNDERLINE | COLOR_PAIR(4));
            attron(COLOR_PAIR(1));

            std::vector<std::string> enemyCoords;
            std::vector<coordinates> enemyShotsFired; 

            int sunkBefore = myBoard.getSunkCount();
            int missBefore = myBoard.getMissCount();
            
            int enemyShotsCount = 0;
            if (recv(client_socket, (char*)&enemyShotsCount, sizeof(int), MSG_WAITALL) <= 0) {
                 attrset(A_NORMAL); 
                 clear(); 
                 mvprintw(5, 2, "Error: Connection lost!");
                 mvprintw(6, 2, "Press any key to exit...");
                 refresh();
                 flushinp(); 
                 getch();
                 clear(); 
                 closesocket(client_socket);
                 return;
            }
            
            for (int i = 0; i < enemyShotsCount; i++) {
                coordinates shot;
                if (recv(client_socket, (char*)&shot, sizeof(shot), MSG_WAITALL) <= 0) {
                     break;
                }
                enemyShotsFired.push_back(shot);
                
                char coordBuf[16];
                sprintf(coordBuf, "%c%d", 'A' + shot.x, shot.y + 1);
                enemyCoords.push_back(std::string(coordBuf));
                
                int result = myBoard.receiveShot(shot.x, shot.y);
                
                char responseChar;
                if (result == 0) {
                    responseChar = 'm'; 
                } else if (result == 1) {
                    responseChar = 'h'; 
                    enemyHits++;
                } else { 
                    responseChar = 's'; 
                    enemyHits++;
                    myShipsRemaining--;
                }
                
                send(client_socket, &responseChar, sizeof(char), 0);
                
                int shot_y = layout.startY + 3 + shot.y;
                int shot_x = layout.board1StartX + 5 + (4 * shot.x);
                
                attron(COLOR_PAIR(1));
                move(shot_y, shot_x - 1); addch(' '); 
                move(shot_y, shot_x + 1); addch(' ');
                
                if (responseChar == 'm') {
                    attron(COLOR_PAIR(3)); 
                    move(shot_y, shot_x); addch('O'); 
                    myBoard.boardArray[shot.y][shot.x] = 'o';
                } else if (responseChar == 'h') {
                    attron(COLOR_PAIR(4)); 
                    move(shot_y, shot_x); addch('X'); 
                    myBoard.boardArray[shot.y][shot.x] = 'x'; 
                } else { 
                    for (int r = 0; r < size; r++) {
                        for (int c = 0; c < size; c++) {
                            if (myBoard.boardArray[r][c] == 'S' || myBoard.boardArray[r][c] == 's') {
                                myBoard.boardArray[r][c] = 's'; 
                                int s_y = layout.startY + 3 + r;
                                int s_x = layout.board1StartX + 5 + (4 * c);
                                attron(COLOR_PAIR(4) | A_BOLD);
                                move(s_y, s_x - 1); addch(' ');
                                move(s_y, s_x);     addch('S'); 
                                move(s_y, s_x + 1); addch(' ');
                                attroff(COLOR_PAIR(4) | A_BOLD);
                            }
                        }
                    }
                }
                attron(COLOR_PAIR(1));
                refresh();
                
                #ifdef _WIN32
                    Sleep(400);
                #else
                    usleep(400000);
                #endif
            }
            
            int countWounded = 0;
            for (const auto& shot : enemyShotsFired) {
                if (myBoard.boardArray[shot.y][shot.x] == 'x') {
                    countWounded++;
                }
            }
            int countHits = countWounded;
            int countSunk = myBoard.getSunkCount() - sunkBefore;
            int countMiss = myBoard.getMissCount() - missBefore;
            
            std::string eCoordsStr = "";
            for (size_t i = 0; i < enemyCoords.size(); i++) {
                eCoordsStr += enemyCoords[i];
                if (i < enemyCoords.size() - 1) eCoordsStr += ",";
            }
            
            std::string eStatsStr = " - ";
            bool eHasStats = false;
            
            if (countHits > 0) {
                eStatsStr += std::to_string(countHits) + " wounded";
                eHasStats = true;
            }
            if (countSunk > 0) {
                if (eHasStats) eStatsStr += ", ";
                eStatsStr += std::to_string(countSunk) + " sunk";
                eHasStats = true;
            }
            if (countMiss > 0) {
                if (eHasStats) eStatsStr += ", ";
                eStatsStr += std::to_string(countMiss) + " miss";
                eHasStats = true;
            }
            
            mvprintw(enemyStatsY + 1, layout.board1StartX, "%s%s", eCoordsStr.c_str(), eStatsStr.c_str());
            refresh();
            
            if (enemyHits >= maxHits) {
                drawFirework(false);
                closesocket(client_socket);
                return;
            }
            
            playerTurn = true; 
        }
    }
    closesocket(client_socket);
}

void User::displayBoard(Gameboard board) {
    int size = board.getBoardSize();
    BoardLayout layout = calculateBoardLayout(size);
    
    int boardWidth = size * 4 + 8;
    
    clear();
    
    const char *yourBoardTitle, *oppBoardTitle;
    if (size >= 20) {
        yourBoardTitle = "You";
        oppBoardTitle = "Opp";
    } else if (size >= 15) {
        yourBoardTitle = "Your";
        oppBoardTitle = "Opponent";
    } else {
        yourBoardTitle = "Your Board";
        oppBoardTitle = "Opp. Board";
    }
    
    int yourBoardLen = strlen(yourBoardTitle);
    int oppBoardLen = strlen(oppBoardTitle);
    
    int leftPad1 = (boardWidth - yourBoardLen) / 2;
    int rightPad1 = boardWidth - leftPad1 - yourBoardLen;
    int leftPad2 = (boardWidth - oppBoardLen) / 2;
    int rightPad2 = boardWidth - leftPad2 - oppBoardLen;
    
    move(layout.startY, layout.board1StartX);
    for (int i = 0; i < leftPad1; i++) printw("-");
    printw("%s", yourBoardTitle);
    for (int i = 0; i < rightPad1; i++) printw("-");
    
    move(layout.startY, layout.separatorX);
    printw("~~~~~");
    
    move(layout.startY, layout.board2StartX);
    for (int i = 0; i < leftPad2; i++) printw("-");
    printw("%s", oppBoardTitle);
    for (int i = 0; i < rightPad2; i++) printw("-");
    printw("\n");

    move(layout.startY + 1, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("_");
    move(layout.startY + 1, layout.separatorX);
    printw("~~~~~");
    move(layout.startY + 1, layout.board2StartX);
    for (int i = 0; i < boardWidth; i++) printw("_");
    printw("\n");

    move(layout.startY + 2, layout.board1StartX);
    printw("|  | A |");
    for (int i = 1; i < size; i++) {
        printw(" %c |", 'A' + i);
    }
    
    move(layout.startY + 2, layout.separatorX);
    printw("~~~~~");
    
    move(layout.startY + 2, layout.board2StartX);
    printw("|  | A |");
    for (int i = 1; i < size; i++) {
        printw(" %c |", 'A' + i);
    }
    printw("\n");

    for (int i = 0; i < size; i++) {
        move(layout.startY + 3 + i, layout.board1StartX);
        printw("|%2d|", i + 1);
        for (int j = 0; j < size; j++) {
            printw("   |");
        }
        
        move(layout.startY + 3 + i, layout.separatorX);
        printw("~~~~~");
        
        move(layout.startY + 3 + i, layout.board2StartX);
        printw("|%2d|", i + 1);
        for (int j = 0; j < size; j++) {
            printw("   |");
        }
    }

    move(layout.startY + 3 + size, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("-");
    move(layout.startY + 3 + size, layout.separatorX);
    printw("~~~~~");
    move(layout.startY + 3 + size, layout.board2StartX);
    for (int i = 0; i < boardWidth; i++) printw("-");
    printw("\n");
    
    attron(A_UNDERLINE);
    mvprintw(1, 1, "instructions");
    attroff(A_UNDERLINE);
    
    mvprintw(2, 1, "w/↑ - up      a/← - left");
    mvprintw(3, 1, "s/↓ - down    d/→ - right");
    mvprintw(4, 1, "space/enter - select target");
    mvprintw(5, 1, "F - fire all shots");
    mvprintw(6, 1, "q - quit game");
    
    attron(A_UNDERLINE);
    mvprintw(layout.instructionsY, layout.logStartX, "log");
    attroff(A_UNDERLINE);
    
    refresh();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board.boardArray[i][j] != 'w') {
                attron(COLOR_PAIR(2));
                move(layout.startY + 3 + i, layout.board1StartX + 4 + (4 * j));
                addch(' ');
                addch(toupper(board.boardArray[i][j]));
                addch(' ');
            } else {
                attron(COLOR_PAIR(3));
                move(layout.startY + 3 + i, layout.board1StartX + 4 + (4 * j));
                addch(' ');
                addch(' ');
                addch(' ');
            }
        }
    }
    attron(COLOR_PAIR(1));
    cursor.y = layout.startY + 3;
    cursor.x = layout.board2StartX + 5;
    move(cursor.y, cursor.x);
    refresh();
}

void User::printClientIP(struct sockaddr_in their_address) {
    char s[INET6_ADDRSTRLEN];
    inet_ntop(their_address.sin_family, &their_address.sin_addr, s, sizeof(s));
    cout << "Connection established with " << s << "\n";
}

void User::handleFullBoard(char_coordinates cc) {
    int size = getBoardSize();
    BoardLayout layout = calculateBoardLayout(size);
    
    int y_coor = layout.startY + 3 + cc.y;
    int x_coor = layout.board2StartX + 5 + (4 * cc.x);
    
    move(y_coor, x_coor);
    char c = inch() & A_CHARTEXT;
    if (c == 'm' || cc.c == 'w') {
        attron(COLOR_PAIR(3));
    } else {
        attron(COLOR_PAIR(2));
    }
    if (c == ' ') {
        if (cc.c == 'w') {
            addch(' ');
        } else {
            addch(cc.c);
        }
    }
    if (c == 'h') {
        attron(COLOR_PAIR(4));
        addch(cc.c);
        attron(COLOR_PAIR(2));
    }

    move(y_coor, x_coor - 1);
    addch(' ');
    move(y_coor, x_coor + 1);
    addch(' ');
    attron(COLOR_PAIR(1));
}

int User::handleAttack(SOCKET client_socket, Gameboard& myBoard) { 
    int shotX, shotY;
    
    if (recv(client_socket, (char*)&shotX, sizeof(int), 0) <= 0) return -1;
    if (recv(client_socket, (char*)&shotY, sizeof(int), 0) <= 0) return -1;
    
    int result = myBoard.receiveShot(shotX, shotY);
    string result_str = "";
    
    int size = getBoardSize();
    int startX, startY, width, height;
    
    UIHelper::getBoardDimensions(size, startX, startY, width, height);
    
    int x_pos = startX + 5 + (shotX * 4);
    int y_pos = startY + 2 + shotY;
    
    if (result == 1 || result == 2) {
        if (result == 2) {
            result_str = "Sunk";
            
            for(int r = 0; r < size; r++) {
                for(int c = 0; c < size; c++) {
                    if (myBoard.boardArray[r][c] == 'S' || myBoard.boardArray[r][c] == 's') {
                        myBoard.boardArray[r][c] = 's'; 
                        
                        int sX = startX + 5 + (c * 4);
                        int sY = startY + 2 + r;
                        
                        attron(COLOR_PAIR(3)); 
                        move(sY, sX - 1); addch('[');
                        move(sY, sX);     addch('S'); 
                        move(sY, sX + 1); addch(']');
                        attroff(COLOR_PAIR(3));
                    }
                }
            }
        } else {
            result_str = "Hit";
            attron(COLOR_PAIR(4));
            move(y_pos, x_pos - 1); addch(' ');
            move(y_pos, x_pos);     addch('X'); 
            move(y_pos, x_pos + 1); addch(' ');
            attroff(COLOR_PAIR(4));
        }
        
        send(client_socket, (const char*)&result, sizeof(int), 0);
    } else {
        result = 0; 
        result_str = "Miss";
        
        attron(COLOR_PAIR(3));
        move(y_pos, x_pos - 1); addch(' ');
        move(y_pos, x_pos);     addch('O'); 
        move(y_pos, x_pos + 1); addch(' ');
        attroff(COLOR_PAIR(3));
        
        send(client_socket, (const char*)&result, sizeof(int), 0);
    }
    
    string print_msg = "Opponent shot at " + to_string(shotY + 1) + ": "; 
    
    messageLog(print_msg + result_str);
    
    return result;
}

void User::messageLog(string message) {
    int size = getBoardSize();
    BoardLayout layout = calculateBoardLayout(size);
    
    logQueue.push(message);
    
    while (logQueue.size() > 10) {
        logQueue.pop();
    }

    int logY = layout.instructionsY + 1;
    for (int i = 0; i < 10; i++) {
        move(logY + i, layout.logStartX);
        clrtoeol();
    }
    
    std::queue<string> tempQueue = logQueue;
    int lineNum = 0;
    while (!tempQueue.empty() && lineNum < 10) {
        attron(COLOR_PAIR(1));
        mvprintw(logY + lineNum, layout.logStartX, "%s", tempQueue.front().c_str());
        attroff(COLOR_PAIR(1));
        tempQueue.pop();
        lineNum++;
    }
    
    refresh();
}
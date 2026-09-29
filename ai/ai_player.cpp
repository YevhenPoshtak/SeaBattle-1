#include "ai_player.hpp"
#include "../game/board_size_menu.hpp"
#include "../game/board_layout.hpp"
#include "../game/ship_config.hpp"
#include "../game/game_settings.hpp"
#include "../utils/game_logic_utils.hpp"
#include "../ui/animation.hpp"
#include "../ui/config.hpp" 
#include <algorithm>
#include <ctime>
#include <cmath>
#include <cstring>
#include <vector>
#include <string>
#include <deque>

#ifdef _WIN32
    #include <windows.h>
    #include <pdcurses.h>
#else
    #include <unistd.h>
    #include <ncurses.h>
#endif

extern GameSettings g_gameSettings;

static std::deque<AICoordinates> s_targetQueue;
static std::vector<AICoordinates> s_parityShots; 
static bool s_isSmartInitialized = false;

AIPlayer::AIPlayer(AIDifficulty diff) 
    : User(false), 
      difficulty(diff), 
      hunting(false), 
      huntDirection(0) {
    
    int size = getBoardSize();
    opponentBoard.resize(size, std::vector<char>(size, '?'));
    
    lastHit.x = -1;
    lastHit.y = -1;
    
    attackSeed = time(NULL) + 12345;
    
    s_targetQueue.clear();
    s_parityShots.clear();
    s_isSmartInitialized = false;
    
    initializeAvailableShots();
    setupBoard();
}

void AIPlayer::connect() {
}

void AIPlayer::setupBoard() {
    myBoard.generateRandomBoardAuto(false);
}

void AIPlayer::initializeAvailableShots() {
    availableShots.clear();
    s_parityShots.clear();
    
    int size = getBoardSize();
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            AICoordinates coord;
            coord.x = j;
            coord.y = i;
            availableShots.push_back(coord);
            if ((j + i) % 2 == 0) {
                s_parityShots.push_back(coord);
            }
        }
    }

    std::srand(std::time(0));
    std::random_shuffle(availableShots.begin(), availableShots.end());
    std::random_shuffle(s_parityShots.begin(), s_parityShots.end());
}

void addSmartNeighbors(int x, int y, int boardSize, const std::vector<std::vector<char>>& knownBoard) {

    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};
    
    for(int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        
        if(nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
            if(knownBoard[ny][nx] == '?' || knownBoard[ny][nx] == ' ') {
                AICoordinates c;
                c.x = nx;
                c.y = ny;
                s_targetQueue.push_back(c);
            }
        }
    }
}

AICoordinates AIPlayer::pickAttackCoordinates(int boardSize) {
    AICoordinates coord;
    coord.x = -1; coord.y = -1;

    if (availableShots.empty()) return coord;

    if (difficulty == EASY) {
        int index = rand() % availableShots.size();
        coord = availableShots[index];
        availableShots.erase(availableShots.begin() + index);
        return coord;
    }

    while (!s_targetQueue.empty()) {
        coord = s_targetQueue.front();
        s_targetQueue.pop_front();
        
        bool valid = false;
        int removeIndex = -1;
        
        for (size_t i = 0; i < availableShots.size(); i++) {
            if (availableShots[i].x == coord.x && availableShots[i].y == coord.y) {
                valid = true;
                removeIndex = i;
                break;
            }
        }
        
        if (valid) {
            availableShots.erase(availableShots.begin() + removeIndex);
            for(size_t i=0; i<s_parityShots.size(); ++i) {
                if(s_parityShots[i].x == coord.x && s_parityShots[i].y == coord.y) {
                    s_parityShots.erase(s_parityShots.begin() + i);
                    break;
                }
            }
            return coord;
        }
    }

    if (!s_parityShots.empty()) {
        coord = s_parityShots.back();
        s_parityShots.pop_back();
        
        for (size_t i = 0; i < availableShots.size(); i++) {
            if (availableShots[i].x == coord.x && availableShots[i].y == coord.y) {
                availableShots.erase(availableShots.begin() + i);
                break;
            }
        }
        return coord;
    }

    int index = rand() % availableShots.size();
    coord = availableShots[index];
    availableShots.erase(availableShots.begin() + index);
    return coord;
}

void AIPlayer::recordShotResult(int x, int y, bool isHit, bool isSunk) {
    int size = getBoardSize();
    if (x >= 0 && x < size && y >= 0 && y < size) {
        opponentBoard[y][x] = isHit ? 'X' : 'O';
        
        if (difficulty == SMART) {
            if (isHit && !isSunk) {
                addSmartNeighbors(x, y, size, opponentBoard);
            }
            
            if (isSunk) {
                hunting = false;
                lastHit.x = -1;
                lastHit.y = -1;
            }
        }

        if (isHit) {
            lastHit.x = x;
            lastHit.y = y;
            hunting = true;
        }
    }
}

bool AIPlayer::isValidCoordinate(int x, int y) {
    int size = getBoardSize();
    return (x >= 0 && x < size && y >= 0 && y < size);
}

void AIPlayer::clearTargetQueue() {
    s_targetQueue.clear();
}

void AIPlayer::reset() {
    int size = getBoardSize();
    opponentBoard.assign(size, std::vector<char>(size, '?'));
    
    lastHit.x = -1;
    lastHit.y = -1;
    
    hunting = false;
    huntDirection = 0;
    
    clearTargetQueue();
    initializeAvailableShots();
}

char performAttackOnAI(Gameboard& board, AICoordinates coord) {
    int res = board.receiveShot(coord.x, coord.y);
    
    if (res == 0) return 'm'; 
    if (res == 1) return 'h'; 
    if (res == 2) {
        for (const auto& ship : board.myShips) {
            if (ship.isSunk) {
                bool belongs = false;
                if (ship.orientation == 1) {
                    if (coord.x == ship.startCol && coord.y >= ship.startRow && coord.y < ship.startRow + ship.length) {
                        belongs = true;
                    }
                } else {
                    if (coord.y == ship.startRow && coord.x <= ship.startCol && coord.x > ship.startCol - ship.length) {
                        belongs = true;
                    }
                }
                if (belongs) {
                    return ship.symbol; 
                }
            }
        }
        return 's'; 
    }
    return 'm';
}

void AIPlayer::gameLoop(SOCKET client_socket) {
    (void)client_socket; 
    
    clear();
    
    int size = getBoardSize();
    int shots = g_gameSettings.shotsPerTurn;
    if (shots <= 0) shots = 1; 

    const char* diffName = (difficulty == EASY) ? "Easy" : "Smart";
    
    Gameboard playerBoard;
    playerBoard.setBoardSize(size); 

    int boardResult = 0;
    while (boardResult != 1) {
        boardResult = playerBoard.generateRandomBoard(true);  
        if (boardResult == 0) {
            boardResult = playerBoard.generateManualBoard();
        }
    }

    BoardLayout layout = calculateBoardLayout(size);
    
    int boardWidth = size * 4 + 8;
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    clear();
    
    char aiTitle[50];
    const char* yourBoardTitle;
    
    if (size >= 20) {
        sprintf(aiTitle, "AI-%s", diffName);
        yourBoardTitle = "You";
    } else if (size >= 15) {
        sprintf(aiTitle, "AI (%s)", diffName);
        yourBoardTitle = "Your";
    } else {
        sprintf(aiTitle, "AI Board (%s)", diffName);
        yourBoardTitle = "Your Board";
    }
    
    int yourBoardLen = strlen(yourBoardTitle);
    int aiTitleLen = strlen(aiTitle);
    
    int leftPad1 = (boardWidth - yourBoardLen) / 2;
    int rightPad1 = boardWidth - leftPad1 - yourBoardLen;
    int leftPad2 = (boardWidth - aiTitleLen) / 2;
    int rightPad2 = boardWidth - leftPad2 - aiTitleLen;
    
    move(layout.startY, layout.board1StartX);
    for (int i = 0; i < leftPad1; i++) printw("-");
    printw("%s", yourBoardTitle);
    for (int i = 0; i < rightPad1; i++) printw("-");
    
    move(layout.startY, layout.separatorX);
    printw("~~~~~");
    
    move(layout.startY, layout.board2StartX);
    for (int i = 0; i < leftPad2; i++) printw("-");
    printw("%s", aiTitle);
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
            if (playerBoard.boardArray[i][j] != 'w') {
                attron(COLOR_PAIR(2));
                move(layout.startY + 3 + i, layout.board1StartX + 5 + (4 * j));
                addch(playerBoard.boardArray[i][j]);
            }
        }
    }
    attron(COLOR_PAIR(1));
    
    AICoordinates cursor;
    cursor.y = layout.startY + 3;
    cursor.x = layout.board2StartX + 9;
    int maxCursorX = cursor.x + (size - 1) * 4;
    int maxCursorY = cursor.y + size - 1;
    
    move(cursor.y, cursor.x);
    refresh();
    
    int maxHits = getTotalShipCells(size);
    int totalShips = getTotalShips(size);
    int playerHits = 0;
    int aiHits = 0;
    bool playerTurn = true;
    
    int playerShipsRemaining = totalShips;
    int aiShipsRemaining = totalShips;
    
    std::vector<std::vector<char>> aiKnownBoard(size, std::vector<char>(size, ' '));
    
    int grid_x = 0, grid_y = 0;
    
    struct PendingShot {
        int x, y;
        bool used;
    };
    
    std::vector<PendingShot> playerShots;
    playerShots.resize(shots);
    for (int i = 0; i < shots; i++) {
        playerShots[i].used = false;
    }
    
    int shotsSelected = 0;
    bool selectingMode = true;
    
    int animFrame = 0;
    int animStartY = maxY - 6;  
    
    int playerStatsY = layout.startY + 3 + size + 2;  
    int aiStatsY = layout.startY + 3 + size + 5;      
    
    while (playerHits < maxHits && aiHits < maxHits) {
        move(0, maxX - 35);
        clrtoeol();
        attron(COLOR_PAIR(5) | A_BOLD);
        printw("YOUR SHIPS: %d", playerShipsRemaining);
        attroff(A_BOLD);
        
        move(0, maxX - 15);
        attron(COLOR_PAIR(6) | A_BOLD);
        printw("AI: %d", aiShipsRemaining);
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
                        shots, shotsSelected, shots);
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
                        if (aiKnownBoard[grid_y][grid_x] == ' ' && shotsSelected < shots) {
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
                        return;
                }
                
                move(cursor.y, cursor.x);
                refresh();
            }
            
            if (!selectingMode) {
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
                int prevSunk = myBoard.getSunkCount();
                int prevMiss = myBoard.getMissCount();
                std::vector<AICoordinates> shotsFiredInVolley;
                
                for (int i = 0; i < shotsSelected; i++) {
                    AICoordinates attack;
                    attack.x = playerShots[i].x;
                    attack.y = playerShots[i].y;
                    
                    shotsFiredInVolley.push_back(attack);
                    
                    char coordBuf[16];
                    sprintf(coordBuf, "%c%d", 'A' + attack.x, attack.y + 1);
                    volleyCoords.push_back(std::string(coordBuf));
                    
                    int shotRes = myBoard.receiveShot(attack.x, attack.y);
                    char resultChar = 'm';
                    
                    if (shotRes == 0) {
                        resultChar = 'm';
                        myBoard.boardArray[attack.y][attack.x] = 'o'; 
                    } 
                    else if (shotRes == 1) {
                        resultChar = 'h';
                        myBoard.boardArray[attack.y][attack.x] = 'x'; 
                        playerHits++;
                    } 
                    else if (shotRes == 2) {
                        resultChar = 's'; 
                        playerHits++;
                        aiShipsRemaining--;
                        
                        auto sunkCells = myBoard.getShipOccupiedCells(attack.x, attack.y);
                        
                        for (const auto& cell : sunkCells) {
                            myBoard.boardArray[cell.second][cell.first] = 's'; 
                            aiKnownBoard[cell.second][cell.first] = 's';
                            
                            int sY = layout.startY + 3 + cell.second;
                            int sX = layout.board2StartX + 9 + (4 * cell.first);
                            
                            attron(COLOR_PAIR(4) | A_BOLD);
                            move(sY, sX); addch('S');
                        }
                    }
                    
                    int shot_y = layout.startY + 3 + attack.y;
                    int shot_x = layout.board2StartX + 9 + (4 * attack.x);

                    attron(COLOR_PAIR(1)); 
                    move(shot_y, shot_x - 1); addch(' '); 
                    move(shot_y, shot_x + 1); addch(' '); 
                    
                    if (resultChar == 'h') {
                        attron(COLOR_PAIR(4));
                        move(shot_y, shot_x); addch('X'); 
                        aiKnownBoard[attack.y][attack.x] = 'h';
                    } else if (resultChar == 'm') {
                        attron(COLOR_PAIR(3));
                        move(shot_y, shot_x); addch('O'); 
                        aiKnownBoard[attack.y][attack.x] = 'm';
                    } 
                    
                    attron(COLOR_PAIR(1));
                    refresh();
                    
                    #ifdef _WIN32
                        Sleep(300);
                    #else
                        usleep(300000);
                    #endif
                }

                int countWounded = 0;
                for (const auto& shot : shotsFiredInVolley) {
                    if (myBoard.boardArray[shot.y][shot.x] == 'x') {
                        countWounded++;
                    }
                }
                
                int countHits = countWounded;
                int countSunk = myBoard.getSunkCount() - prevSunk;
                int countMiss = myBoard.getMissCount() - prevMiss;

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
                    return;
                }
                
                shotsSelected = 0;
                selectingMode = true;
                playerTurn = false; 
            }

        } else {
            move(1, 98);
            clrtoeol();
            attron(COLOR_PAIR(5));
            printw(" AI's turn...                           ");
            attron(COLOR_PAIR(1));
            refresh();
            
            #ifdef _WIN32
                Sleep(1000);
            #else
                usleep(1000000);
            #endif
            
            for (int i = 0; i < 3; i++) {
                move(aiStatsY + i, layout.board1StartX);
                clrtoeol();
            }
            
            attron(A_UNDERLINE | COLOR_PAIR(4));
            mvprintw(aiStatsY, layout.board1StartX, "Ai's volley:");
            attroff(A_UNDERLINE | COLOR_PAIR(4));
            attron(COLOR_PAIR(1));
            
            std::vector<std::string> aiCoords;
            int prevSunk = playerBoard.getSunkCount();
            int prevMiss = playerBoard.getMissCount();
            std::vector<AICoordinates> aiShotsFired;
            
            for (int i = 0; i < shots; i++) {
                AICoordinates shot = pickAttackCoordinates(size);
                if (shot.x == -1 || shot.y == -1) {
                    break; 
                }
                aiShotsFired.push_back(shot);
                
                char coordBuf[16];
                sprintf(coordBuf, "%c%d", 'A' + shot.x, shot.y + 1);
                aiCoords.push_back(std::string(coordBuf));
                
                int result = playerBoard.receiveShot(shot.x, shot.y);
                
                int shot_y = layout.startY + 3 + shot.y;
                int shot_x = layout.board1StartX + 5 + (4 * shot.x); 
                
                attron(COLOR_PAIR(1)); 
                move(shot_y, shot_x - 1); 
                addch(' '); 
                move(shot_y, shot_x + 1); 
                addch(' ');

                if (result == 0) {
                    attron(COLOR_PAIR(3));
                    move(shot_y, shot_x); addch('O');
                    recordShotResult(shot.x, shot.y, false, false);
                } else if (result == 1) {
                    attron(COLOR_PAIR(4));
                    move(shot_y, shot_x); addch('X'); 
                    aiHits++;
                    recordShotResult(shot.x, shot.y, true, false);
                } else {
                    recordShotResult(shot.x, shot.y, true, true);
                    aiHits++;
                    playerShipsRemaining--;
                    
                    for (int r = 0; r < size; r++) {
                        for (int c = 0; c < size; c++) {
                            if (playerBoard.boardArray[r][c] == 's') { 
                                int sY = layout.startY + 3 + r;
                                int sX = layout.board1StartX + 5 + (4 * c);
                                attron(COLOR_PAIR(4) | A_BOLD);
                                move(sY, sX); addch('S');
                            }
                        }
                    }
                }
                attron(COLOR_PAIR(1));
                refresh();
                #ifdef _WIN32
                    Sleep(300);
                #else
                    usleep(300000);
                #endif
            }
            
            int countWounded = 0;
            for (const auto& shot : aiShotsFired) {
                if (playerBoard.boardArray[shot.y][shot.x] == 'x') { 
                    countWounded++;
                }
            }
            int countHits = countWounded;
            int countSunk = playerBoard.getSunkCount() - prevSunk;
            int countMiss = playerBoard.getMissCount() - prevMiss;
            
            std::string aiCoordsStr = "";
            for (size_t i = 0; i < aiCoords.size(); i++) {
                aiCoordsStr += aiCoords[i];
                if (i < aiCoords.size() - 1) aiCoordsStr += ",";
            }
            
            std::string aiStatsStr = " - ";
            bool aiHasStats = false;
            if (countHits > 0) {
                aiStatsStr += std::to_string(countHits) + " wounded";
                aiHasStats = true;
            }
            if (countSunk > 0) {
                if (aiHasStats) aiStatsStr += ", ";
                aiStatsStr += std::to_string(countSunk) + " sunk";
                aiHasStats = true;
            }
            if (countMiss > 0) {
                if (aiHasStats) aiStatsStr += ", ";
                aiStatsStr += std::to_string(countMiss) + " miss";
                aiHasStats = true;
            }

            mvprintw(aiStatsY + 1, layout.board1StartX, "%s%s", aiCoordsStr.c_str(), aiStatsStr.c_str());
            refresh();
            
            if (aiHits >= maxHits) {
                drawFirework(false);
                return;
            }
            playerTurn = true; 
        }
    }
}

void AIPlayer::playGame(AIDifficulty difficulty) {
    clear();
    
    int size = getBoardSize();
    int shots = selectShotsPerTurn(size);
    g_gameSettings.shotsPerTurn = shots;
    
    clear();
    const char* diffName = (difficulty == EASY) ? "Easy" : "Smart";
    mvprintw(2, 2, "Playing against %s AI", diffName);
    mvprintw(3, 2, "Board: %dx%d | Shots: %d per turn", size, size, shots);
    refresh();
    
    #ifdef _WIN32
        Sleep(2000);
    #else
        usleep(2000000);
    #endif
    
    AIPlayer ai(difficulty);
    ai.gameLoop(0); 
}
/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: game_loop.cpp
 * Description: Main game loop implementation that handles turn-based gameplay.
 *              Manages player and enemy turns, shot selection, firing mechanics,
 *              board updates, and win/loss conditions for both AI and multiplayer modes.
 */

#include "game_loop.hpp"
#include "../ui/ui_renderer.hpp"
#include "../ui/ui_animation.hpp"
#include "../logic/ai_logic.hpp"
#include "../logic/network_logic.hpp"
#include "../logic/game_logic.hpp"
#include <string>
#include <deque>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(x) Sleep(x)
#else
    #include <unistd.h>
    #define SLEEP_MS(x) usleep((x) * 1000)
#endif

// Main game loop that alternates between player and opponent turns
// Handles shot selection, firing, board updates, and victory conditions
// playerBoard: player's board data
// enemyBoard: enemy's board data
// enemyKnownBoard: player's knowledge of enemy board (fog of war)
// size: board dimensions
// shots: number of shots per turn
// playerTurn: whether player goes first
// isAI: true for AI opponent, false for network multiplayer
// aiPtr: pointer to AI logic (if AI mode)
// socketPtr: pointer to network socket (if multiplayer mode)
// isHost: whether player is host in multiplayer
void GameLoop::runGameLoop(
    BoardData& playerBoard,
    BoardData& enemyBoard,
    std::vector<std::vector<char>>& enemyKnownBoard,
    int size,
    int shots,
    bool playerTurn,
    bool& isAI,
    void* aiPtr,
    void* socketPtr,
    bool isHost
) {
    // Calculate board layout for UI rendering
    BoardLayout layout = calculateBoardLayout(size);
    int totalShips = getTotalShips(size);
    
    // Initialize game statistics
    int playerHits = 0;
    int enemyHits = 0;
    int playerShipsRemaining = totalShips;
    int enemyShipsRemaining = totalShips;
    
    // Cast pointers based on game mode (AI or multiplayer)
    AILogic* ai = isAI ? static_cast<AILogic*>(aiPtr) : nullptr;
    SOCKET_TYPE* clientSocket = !isAI ? static_cast<SOCKET_TYPE*>(socketPtr) : nullptr;
    
    clear();
    
    // Set board titles based on size and opponent type
    const char* yourTitle = (size >= 20) ? "You" : (size >= 15) ? "Your" : "Your Board";
    const char* oppTitle;
    
    if (isAI) {
        AIDifficulty difficulty = ai->getDifficulty();
        if (size >= 20) {
            oppTitle = (difficulty == EASY) ? "AI-Easy" : "AI-Smart";
        } else if (size >= 15) {
            oppTitle = (difficulty == EASY) ? "AI (Easy)" : "AI (Smart)";
        } else {
            oppTitle = (difficulty == EASY) ? "AI Board (Easy)" : "AI Board (Smart)";
        }
    } else {
        oppTitle = (size >= 20) ? "Opp" : (size >= 15) ? "Opponent" : "Opp. Board";
    }
    
    // Draw initial game UI
    UIRenderer::drawGameBoards(layout, size, yourTitle, oppTitle);
    UIRenderer::drawInstructions(layout);
    UIRenderer::drawBoardState(layout, playerBoard, true);
    
    // Initialize cursor for shot selection on enemy board
    int cursorX = layout.board2StartX + 9;
    int cursorY = layout.startY + 3;
    int maxCursorX = cursorX + (size - 1) * 4;
    int maxCursorY = cursorY + size - 1;
    int gridX = 0, gridY = 0;
    
    // Initialize shot selection array
    std::vector<PendingShot> playerShots(shots);
    for (int i = 0; i < shots; i++) playerShots[i].used = false;
    
    int shotsSelected = 0;
    bool selectingMode = true;

    // Get terminal dimensions for animation positioning
    int maxY, maxX; 
    getmaxyx(stdscr, maxY, maxX);
    int animStartY = maxY - 6;
    int animFrame = 0;
    
    // Calculate positions for volley result display
    int playerStatsY = layout.startY + 3 + size + 2;
    int enemyStatsY = layout.startY + 3 + size + 5;
    
    // Main game loop - continues until one player loses all ships
    while (playerShipsRemaining > 0 && enemyShipsRemaining > 0) {
        // Update and display game statistics
        UIRenderer::drawGameStats(0, maxX - 35, playerShipsRemaining, enemyShipsRemaining);
        
        // Draw decorative ship animation at bottom if space available
        if (animStartY > layout.startY + 3 + size + 5) {
            UIAnimation::drawBottomShipAnimation(animFrame, animStartY, maxX);
        }
        
        if (playerTurn) {
            if (selectingMode) {
                // PLAYER TURN - SHOT SELECTION PHASE
                // Display instruction message
                char msg[70];
                sprintf(msg, "Select %d (or less) targets (%d/%d) - F to fire", 
                        shots, shotsSelected, shots);
                UIRenderer::showMessage(1, 82, msg, 6);
                UIRenderer::drawCursor(cursorY, cursorX);
                refresh();
                
                // Animate and handle input
                SLEEP_MS(50);
                animFrame++;
                if (animFrame >= 80) animFrame = 0;
                
                // Non-blocking input check
                nodelay(stdscr, TRUE);
                int key = getch();
                nodelay(stdscr, FALSE);
                
                if (key == ERR) continue;
                flushinp();
                
                // Handle cursor movement and shot selection
                switch (key) {
                    case KEY_LEFT:
                    case 'a':
                    case 'A':
                        // Move cursor left
                        if (cursorX > layout.board2StartX + 9 && gridX > 0) {
                            cursorX -= 4; gridX--;
                        }
                        break;
                    case KEY_RIGHT:
                    case 'd':
                    case 'D':
                        // Move cursor right
                        if (cursorX < maxCursorX && gridX < size - 1) {
                            cursorX += 4; gridX++;
                        }
                        break;
                    case KEY_UP:
                    case 'w':
                    case 'W':
                        // Move cursor up
                        if (cursorY > layout.startY + 3 && gridY > 0) {
                            cursorY -= 1; gridY--;
                        }
                        break;
                    case KEY_DOWN:
                    case 's':
                    case 'S':
                        // Move cursor down
                        if (cursorY < maxCursorY && gridY < size - 1) {
                            cursorY += 1; gridY++;
                        }
                        break;
                    case ' ':
                    case 10:
                        // Select/deselect shot at cursor position
                        if (enemyKnownBoard[gridY][gridX] == ' ' && shotsSelected < shots) {
                            // Check if position already selected
                            bool alreadySelected = false;
                            for (int i = 0; i < shotsSelected; i++) {
                                if (playerShots[i].x == gridX && playerShots[i].y == gridY) {
                                    alreadySelected = true;
                                    break;
                                }
                            }
                            if (!alreadySelected) {
                                // Add shot to selection
                                playerShots[shotsSelected].x = gridX;
                                playerShots[shotsSelected].y = gridY;
                                playerShots[shotsSelected].used = true;
                                shotsSelected++;
                                UIRenderer::drawShotIndicator(cursorY, cursorX, true);
                            }
                        }
                        break;
                    case 'f':
                    case 'F':
                        // Confirm selection and fire
                        if (shotsSelected > 0) {
                            selectingMode = false;
                        }
                        break;
                    case 'q':
                    case 'Q':
                        // Quit game
                        if (!isAI && clientSocket) {
                            closesocket(*clientSocket);
                        }
                        return;
                }
            } else {
                // PLAYER TURN - FIRING PHASE
                UIRenderer::showMessage(1, 82, "                    FIRING!                                    ", 4);
                refresh();
                
                // Track volley statistics
                std::vector<std::string> volleyCoords;
                int missInVolley = 0;
                int sunkInVolley = 0;
                
                // Send shot count to opponent in multiplayer mode
                if (!isAI && clientSocket) {
                    if (!NetworkLogic::sendShotCount(*clientSocket, shotsSelected)) {
                        clear();
                        mvprintw(5, 2, "Error: Connection lost!");
                        mvprintw(6, 2, "Press any key to exit...");
                        refresh();
                        getch();
                        closesocket(*clientSocket);
                        clear();
                        return;
                    }
                }
                
                // Prepare shot lists for AI and network modes
                std::vector<AICoordinates> shotsFired;
                std::vector<coordinates> netShotsFired;
                
                // Process each selected shot
                for (int i = 0; i < shotsSelected; i++) {
                    int shotX = playerShots[i].x;
                    int shotY = playerShots[i].y;
                    
                    // Format coordinate for display (e.g., "A5")
                    char coordBuf[16];
                    sprintf(coordBuf, "%c%d", 'A' + shotX, shotY + 1);
                    volleyCoords.push_back(std::string(coordBuf));
                    
                    int shotResult = 0;
                    
                    if (isAI) {
                        // Process shot against AI board
                        AICoordinates shot;
                        shot.x = shotX;
                        shot.y = shotY;
                        shotsFired.push_back(shot);
                        shotResult = ai->getBoard().receiveShot(shotX, shotY);
                    } else {
                        // Send shot to network opponent
                        coordinates shot;
                        shot.x = shotX;
                        shot.y = shotY;
                        netShotsFired.push_back(shot);
                        
                        if (!NetworkLogic::sendShot(*clientSocket, shot)) {
                            clear();
                            mvprintw(5, 2, "Error: Connection lost!");
                            mvprintw(6, 2, "Press any key to exit...");
                            refresh();
                            getch();
                            closesocket(*clientSocket);
                            clear();
                            return;
                        }
                        
                        // Receive shot result from opponent
                        char answer;
                        if (!NetworkLogic::receiveShotResult(*clientSocket, answer)) {
                            clear();
                            mvprintw(5, 2, "Error: Connection lost!");
                            mvprintw(6, 2, "Press any key to exit...");
                            refresh();
                            getch();
                            closesocket(*clientSocket);
                            clear();
                            return;
                        }
                        
                        // Convert network response to shot result
                        if (answer == 'm') shotResult = 0;      // miss
                        else if (answer == 'h') shotResult = 1; // hit
                        else shotResult = 2;                    // sunk
                    }
                    
                    // Calculate screen position for shot result display
                    int screenShotY = layout.startY + 3 + shotY;
                    int screenShotX = layout.board2StartX + 9 + (4 * shotX);
                    
                    // Clear shot indicator before displaying result
                    UIRenderer::clearShotIndicator(screenShotY, screenShotX);
                    
                    if (shotResult == 0) {
                        // MISS - update board with miss marker
                        missInVolley++;
                        enemyKnownBoard[shotY][shotX] = 'm';
                        enemyBoard.boardArray[shotY][shotX] = 'o';
                        UIRenderer::drawBoardCell(screenShotY, screenShotX, 'o', false);
                    } else if (shotResult == 1) {
                        // HIT - update board with hit marker
                        playerHits++;
                        enemyKnownBoard[shotY][shotX] = 'h';
                        enemyBoard.boardArray[shotY][shotX] = 'x';
                        UIRenderer::drawBoardCell(screenShotY, screenShotX, 'x', false);
                    } else if (shotResult == 2) {
                        // SUNK - mark entire ship as sunk
                        playerHits++;
                        enemyShipsRemaining--;
                        sunkInVolley++;
                        
                        if (isAI) {
                            // Get all cells of sunk ship from AI board
                            auto sunkCells = ai->getBoard().getShipOccupiedCells(shotX, shotY);
                            for (const auto& cell : sunkCells) {
                                enemyKnownBoard[cell.second][cell.first] = 's';
                                enemyBoard.boardArray[cell.second][cell.first] = 's';
                                int sY = layout.startY + 3 + cell.second;
                                int sX = layout.board2StartX + 9 + (4 * cell.first);
                                UIRenderer::drawBoardCell(sY, sX, 's', false);
                            }
                        } else {
                            // For network mode, use flood fill to mark entire ship as sunk
                            UIRenderer::clearShotIndicator(screenShotY, screenShotX);
                            
                            // BFS to find all connected hit cells (same ship)
                            std::deque<std::pair<int, int>> updateQueue;
                            updateQueue.push_back({shotX, shotY});
                            enemyKnownBoard[shotY][shotX] = 's';
                            enemyBoard.boardArray[shotY][shotX] = 's';
                            
                            while (!updateQueue.empty()) {
                                std::pair<int, int> current = updateQueue.front();
                                updateQueue.pop_front();
                                
                                int cx = current.first;
                                int cy = current.second;
                                
                                // Check all 4 adjacent cells
                                int dx[] = {0, 0, -1, 1};
                                int dy[] = {-1, 1, 0, 0};
                                
                                for (int k = 0; k < 4; k++) {
                                    int nx = cx + dx[k];
                                    int ny = cy + dy[k];
                                    
                                    if (nx >= 0 && nx < size && ny >= 0 && ny < size) {
                                        if (enemyBoard.boardArray[ny][nx] == 'x') {
                                            // Convert hit to sunk and add to queue
                                            enemyBoard.boardArray[ny][nx] = 's';
                                            enemyKnownBoard[ny][nx] = 's';
                                            
                                            int updateY = layout.startY + 3 + ny;
                                            int updateX = layout.board2StartX + 9 + (4 * nx);
                                            
                                            UIRenderer::clearShotIndicator(updateY, updateX);
                                            updateQueue.push_back({nx, ny});
                                        }
                                    }
                                }
                            }
                            
                            // Redraw all sunk cells
                            for (int r = 0; r < size; r++) {
                                for (int c = 0; c < size; c++) {
                                    if (enemyBoard.boardArray[r][c] == 's') {
                                        int sY = layout.startY + 3 + r;
                                        int sX = layout.board2StartX + 9 + (4 * c);
                                        UIRenderer::drawBoardCell(sY, sX, 's', false);
                                    }
                                }
                            }
                        }
                    }
                    
                    refresh();
                    SLEEP_MS(300);
                    
                    // Check for victory
                    if (enemyShipsRemaining <= 0) {
                        break;
                    }
                }
                
                // Count wounded ships (hit but not sunk)
                int countWounded = 0;
                if (isAI) {
                    for (const auto& shot : shotsFired) {
                        if (ai->getBoard().boardArray[shot.y][shot.x] == 'x') {
                            countWounded++;
                        }
                    }
                } else {
                    for (const auto& shot : netShotsFired) {
                        if (enemyBoard.boardArray[shot.y][shot.x] == 'x') {
                            countWounded++;
                        }
                    }
                }
                
                // Build coordinate string for display
                std::string coordsStr = "";
                for (size_t i = 0; i < volleyCoords.size(); i++) {
                    coordsStr += volleyCoords[i];
                    if (i < volleyCoords.size() - 1) coordsStr += ",";
                }
                
                // Build statistics string
                std::string statsStr = " - ";
                bool hasStats = false;
                if (countWounded > 0) {
                    statsStr += std::to_string(countWounded) + " wounded";
                    hasStats = true;
                }
                if (sunkInVolley > 0) {
                    if (hasStats) statsStr += ", ";
                    statsStr += std::to_string(sunkInVolley) + " sunk";
                    hasStats = true;
                }
                if (missInVolley > 0) {
                    if (hasStats) statsStr += ", ";
                    statsStr += std::to_string(missInVolley) + " miss";
                }
                
                // Display volley results
                UIRenderer::drawVolleyResult(playerStatsY, layout.board1StartX, coordsStr, statsStr, true);
                refresh();
                
                // Check for player victory
                if (enemyShipsRemaining <= 0) {
                    UIAnimation::drawFirework(true);
                    if (!isAI && clientSocket) {
                        closesocket(*clientSocket);
                    }
                    return;
                }
                
                // Reset for next turn
                shotsSelected = 0;
                selectingMode = true;
                playerTurn = false;
            }
        } else {
            // ENEMY TURN
            UIRenderer::showMessage(1, isAI ? 98 : 90, isAI ? " AI's turn...                           " : "         Enemy's turn...                     ", 5);
            refresh();
            SLEEP_MS(1000);
            
            // Track enemy volley statistics
            std::vector<std::string> enemyCoords;
            int missInVolley = 0;
            int sunkInVolley = 0;
            
            // Receive shot count in multiplayer mode
            int enemyShotsCount = shots;
            if (!isAI) {
                if (!NetworkLogic::receiveShotCount(*clientSocket, enemyShotsCount)) {
                    clear();
                    mvprintw(5, 2, "Error: Connection lost!");
                    mvprintw(6, 2, "Press any key to exit...");
                    refresh();
                    getch();
                    closesocket(*clientSocket);
                    clear();
                    return;
                }
            }
            
            // Prepare shot lists for AI and network modes
            std::vector<AICoordinates> aiShotsFired;
            std::vector<coordinates> netEnemyShotsFired;
            
            // Process enemy shots
            for (int i = 0; i < enemyShotsCount; i++) {
                int shotX, shotY;
                
                if (isAI) {
                    // Get AI's attack coordinates
                    AICoordinates shot = ai->pickAttackCoordinates();
                    if (shot.x == -1 || shot.y == -1) break;
                    aiShotsFired.push_back(shot);
                    shotX = shot.x;
                    shotY = shot.y;
                } else {
                    // Receive shot from network opponent
                    coordinates shot;
                    if (!NetworkLogic::receiveShot(*clientSocket, shot)) {
                        break;
                    }
                    netEnemyShotsFired.push_back(shot);
                    shotX = shot.x;
                    shotY = shot.y;
                }
                
                // Format coordinate for display
                char coordBuf[16];
                sprintf(coordBuf, "%c%d", 'A' + shotX, shotY + 1);
                enemyCoords.push_back(std::string(coordBuf));
                
                // Process shot on player's board
                int result = playerBoard.receiveShot(shotX, shotY);
                
                if (!isAI) {
                    // Send result back to network opponent
                    char responseChar;
                    if (result == 0) {
                        responseChar = 'm';
                    } else if (result == 1) {
                        responseChar = 'h';
                    } else {
                        responseChar = 's';
                    }
                    NetworkLogic::sendShotResult(*clientSocket, responseChar);
                }
                
                // Calculate screen position for display
                int screenShotY = layout.startY + 3 + shotY;
                int screenShotX = layout.board1StartX + 5 + (4 * shotX);
                
                if (result == 0) {
                    // MISS
                    missInVolley++;
                    if (isAI) {
                        ai->recordShotResult(shotX, shotY, false, false);
                    }
                    UIRenderer::drawBoardCell(screenShotY, screenShotX, 'o', true);
                } else if (result == 1) {
                    // HIT
                    enemyHits++;
                    if (isAI) {
                        ai->recordShotResult(shotX, shotY, true, false);
                    }
                    UIRenderer::drawBoardCell(screenShotY, screenShotX, 'x', true);
                } else {
                    // SUNK
                    enemyHits++;
                    playerShipsRemaining--;
                    sunkInVolley++;
                    if (isAI) {
                        ai->recordShotResult(shotX, shotY, true, true);
                    }
                    
                    // Redraw all sunk cells of the ship
                    for (int r = 0; r < size; r++) {
                        for (int c = 0; c < size; c++) {
                            if (playerBoard.boardArray[r][c] == 's') {
                                int sY = layout.startY + 3 + r;
                                int sX = layout.board1StartX + 5 + (4 * c);
                                UIRenderer::drawBoardCell(sY, sX, 's', true);
                            }
                        }
                    }
                }
                refresh();
                SLEEP_MS(300);
                
                // Check for enemy victory
                if (playerShipsRemaining <= 0) {
                    break;
                }
            }
            
            // Count wounded ships
            int countWounded = 0;
            if (isAI) {
                for (const auto& shot : aiShotsFired) {
                    if (playerBoard.boardArray[shot.y][shot.x] == 'x') {
                        countWounded++;
                    }
                }
            } else {
                for (const auto& shot : netEnemyShotsFired) {
                    if (playerBoard.boardArray[shot.y][shot.x] == 'x') {
                        countWounded++;
                    }
                }
            }
            
            // Build coordinate string
            std::string eCoordsStr = "";
            for (size_t i = 0; i < enemyCoords.size(); i++) {
                eCoordsStr += enemyCoords[i];
                if (i < enemyCoords.size() - 1) eCoordsStr += ",";
            }
            
            // Build statistics string
            std::string eStatsStr = " - ";
            bool eHasStats = false;
            if (countWounded > 0) {
                eStatsStr += std::to_string(countWounded) + " wounded";
                eHasStats = true;
            }
            if (sunkInVolley > 0) {
                if (eHasStats) eStatsStr += ", ";
                eStatsStr += std::to_string(sunkInVolley) + " sunk";
                eHasStats = true;
            }
            if (missInVolley > 0) {
                if (eHasStats) eStatsStr += ", ";
                eStatsStr += std::to_string(missInVolley) + " miss";
            }
            
            // Display enemy volley results
            UIRenderer::drawVolleyResult(enemyStatsY, layout.board1StartX, eCoordsStr, eStatsStr, false);
            refresh();
            
            // Check for enemy victory (player loss)
            if (playerShipsRemaining <= 0) {
                UIAnimation::drawFirework(false);
                if (!isAI && clientSocket) {
                    closesocket(*clientSocket);
                }
                return;
            }
            
            // Switch back to player turn
            playerTurn = true;
        }
    }
}
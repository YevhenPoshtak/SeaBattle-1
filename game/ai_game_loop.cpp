/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ai_game_loop.cpp
 * Description: Implementation of AI game mode. Handles the complete flow of playing
 *              against AI opponents (Easy or Smart difficulty), including board setup,
 *              manual ship placement, and game loop initialization.
 */

#include "ai_game_loop.hpp"
#include "game_loop.hpp"
#include "game_controller.hpp"
#include "../ui/ui_renderer.hpp"
#include "../ui/ui_config.hpp"
#include "../logic/game_logic.hpp"
#include "../data/ship_data.hpp"
#include <vector>
#include <cstring>

// Platform-specific sleep function
#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(x) Sleep(x)
#else
    #include <unistd.h>
    #define SLEEP_MS(x) usleep((x) * 1000)
#endif

extern GameSettings g_gameSettings;

// Main function to start and manage AI game mode
// difficulty: EASY or SMART AI opponent
void playAIGame(AIDifficulty difficulty) {
    clear();
    
    // Get board size and shots per turn from user
    int size = getBoardSize();
    int shots = UIRenderer::selectShotsPerTurn(size);
    g_gameSettings.shotsPerTurn = shots;
    
    // Display game settings
    clear();
    const char* diffName = (difficulty == EASY) ? "Easy" : "Smart";
    mvprintw(2, 2, "Playing against %s AI", diffName);
    mvprintw(3, 2, "Board: %dx%d | Shots: %d per turn", size, size, shots);
    refresh();
    SLEEP_MS(2000);
    
    // Initialize AI opponent and player board
    AILogic ai(difficulty, size);
    BoardData playerBoard(size);
    
    // Board setup loop - allows player to generate or manually place ships
    int boardResult = 0;
    while (boardResult != 1) {
        boardResult = setupPlayerBoard(playerBoard, size);
        
        // Manual placement mode
        if (boardResult == 0) {
            BoardLayout layout = calculateBoardLayout(size);
            std::vector<GamePiece> pieces;
            GameLogic::initializeGamePieces(playerBoard, pieces);
            
            // Manual ship placement variables
            int shipToPlace = 0;
            int orientation = 0;  // 0 = horizontal, 1 = vertical
            int cursorX = layout.board1StartX + 5;
            int cursorY = layout.startY + 3;
            int maxCursorX = layout.board1StartX + 5 + (size - 1) * 4;
            int maxCursorY = layout.startY + 3 + size - 1;
            
            // Ship placement loop
            while (shipToPlace < (int)pieces.size()) {
                UIRenderer::drawManualBoard(layout, playerBoard);
                
                GamePiece ship = pieces[shipToPlace];
                bool isValid = false;
                
                // Highlight current ship placement position
                UIRenderer::highlightShipPlacement(layout, cursorX, cursorY, 
                    ship.Get_Piece_Length(), orientation, ship.Get_Piece_Symbol(),
                    playerBoard, isValid);
                
                refresh();
                
                // Handle user input for ship placement
                int ch = getch();
                int gridX = (cursorX - layout.board1StartX - 5) / 4;
                int gridY = cursorY - layout.startY - 3;
                
                switch (ch) {
                    case KEY_LEFT:
                    case 'a':
                    case 'A':
                        // Move cursor left with boundary checking
                        if (orientation == 0) {
                            if ((cursorX - (4 * (ship.Get_Piece_Length() - 1))) > layout.board1StartX + 5) {
                                cursorX -= 4;
                            }
                        } else {
                            if (cursorX > layout.board1StartX + 5) {
                                cursorX -= 4;
                            }
                        }
                        break;
                    case KEY_RIGHT:
                    case 'd':
                    case 'D':
                        // Move cursor right
                        if (cursorX < maxCursorX) {
                            cursorX += 4;
                        }
                        break;
                    case KEY_UP:
                    case 'w':
                    case 'W':
                        // Move cursor up with boundary checking
                        if (orientation == 1) {
                            if ((cursorY - (ship.Get_Piece_Length() - 1)) > layout.startY + 3) {
                                cursorY -= 1;
                            }
                        } else {
                            if (cursorY > layout.startY + 3) {
                                cursorY -= 1;
                            }
                        }
                        break;
                    case KEY_DOWN:
                    case 's':
                    case 'S':
                        // Move cursor down
                        if (cursorY < maxCursorY) {
                            cursorY += 1;
                        }
                        break;
                    case 'r':
                    case 'R':
                        // Rotate ship orientation
                        if (orientation == 0) {
                            if ((cursorY - (ship.Get_Piece_Length() - 1)) >= layout.startY + 3) {
                                orientation = 1;
                            }
                        } else {
                            if ((cursorX - (4 * (ship.Get_Piece_Length() - 1))) >= layout.board1StartX + 5) {
                                orientation = 0;
                            }
                        }
                        break;
                    case 'g':
                    case 'G':
                        // Return to auto-generation mode
                        boardResult = -1;
                        shipToPlace = pieces.size();
                        break;
                    case ' ':
                    case 10:  // Enter key
                        // Place ship if position is valid
                        if (isValid) {
                            if (GameLogic::placeShip(playerBoard, gridX, gridY, orientation, 
                                ship.Get_Piece_Length(), ship.Get_Piece_Symbol())) {
                                shipToPlace++;
                                cursorY = layout.startY + 3;
                                cursorX = layout.board1StartX + 5;
                            }
                        }
                        break;
                }
            }
            
            // If user pressed 'G' to regenerate, skip confirmation
            if (boardResult == -1) {
                continue;
            }
            
            // Build ship cell map and confirm placement
            playerBoard.buildShipCellMap();
            if (UIRenderer::confirmBoardPlacement()) {
                boardResult = 1;
            }
        }
    }

    // Initialize game state for AI mode
    std::vector<std::vector<char>> aiKnownBoard(size, std::vector<char>(size, ' '));
    BoardData dummyEnemyBoard(size);
    
    bool isAI = true;
    void* aiPtr = &ai;
    void* socketPtr = nullptr;
    
    // Start the main game loop
    GameLoop::runGameLoop(
        playerBoard,
        dummyEnemyBoard,
        aiKnownBoard,
        size,
        shots,
        true,          // Player goes first
        isAI,
        aiPtr,
        socketPtr,
        true           // Player is host
    );
}
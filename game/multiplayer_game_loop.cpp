/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * * File: multiplayer_game_loop.cpp
 * Description: Handles multiplayer game setup and initialization for both
 * host and client modes. Manages network connection establishment,
 * game settings negotiation, and board setup before starting the main game loop.
 */

#include "multiplayer_game_loop.hpp"
#include "game_loop.hpp"
#include "game_controller.hpp"
#include "../ui/ui_renderer.hpp"
#include "../ui/ui_config.hpp"
#include "../logic/network_logic.hpp"
#include "../logic/game_logic.hpp"
#include "../data/ship_data.hpp"
#include <cstring>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(x) Sleep(x)
#else
    #include <unistd.h>
    #define SLEEP_MS(x) usleep((x) * 1000)
#endif

extern GameSettings g_gameSettings;

// Host multiplayer game - creates server socket, waits for client connection,
// selects game settings, and starts the game with host going first
void playMultiplayerHost() {
    clear();
    refresh();
    
    // Create and bind host socket
    SOCKET_TYPE hostSocket = NetworkLogic::createHostSocket();
    if (hostSocket == INVALID_SOCKET_VALUE) {
        mvprintw(7, 2, "Error creating host socket");
        mvprintw(9, 2, "Press any key to return...");
        refresh();
        getch();
        clear();
        return;
    }
    
    // Wait for client to connect
    mvprintw(8, 2, "Waiting for a client to connect...");
    refresh();
    
    bool accepted = false;
    SOCKET_TYPE clientSocket = NetworkLogic::acceptClientConnection(hostSocket, accepted);
    
    if (clientSocket == INVALID_SOCKET_VALUE || !accepted) {
        mvprintw(10, 2, "Error accepting client connection");
        mvprintw(12, 2, "Press any key to return...");
        refresh();
        getch();
        closesocket(hostSocket);
        clear();
        return;
    }
    
    // Prompt host to accept or decline connection
    mvprintw(9, 2, "\n  Would you like to accept the connection, Y/N?");
    refresh();
    
    while (true) {
        flushinp();
        char userInput = getch();
        if (userInput == 'Y' || userInput == 'y') {
            mvprintw(10, 2, "Accepting connection! Have fun!               ");
            refresh();
            SLEEP_MS(500);
            break;
        } else if (userInput == 'N' || userInput == 'n') {
            mvprintw(10, 2, "Connection refused...");
            refresh();
            SLEEP_MS(500);
            closesocket(clientSocket);
            closesocket(hostSocket);
            clear();
            return;
        }
    }
    
    // Host selects game settings
    int size = getBoardSize();
    int shots = UIRenderer::selectShotsPerTurn(size);
    
    // Send game settings to client
    if (!NetworkLogic::sendGameSettings(clientSocket, size, shots)) {
        clear();
        mvprintw(5, 2, "Error: Connection lost!");
        mvprintw(6, 2, "Press any key to exit...");
        refresh();
        getch();
        closesocket(clientSocket);
        closesocket(hostSocket);
        clear();
        return;
    }
    
    g_gameSettings.shotsPerTurn = shots;
    
    // Display game settings and wait for client board setup
    clear();
    mvprintw(2, 2, "Multiplayer Game (Host)");
    mvprintw(3, 2, "Board: %dx%d | Shots: %d per turn", size, size, shots);
    mvprintw(4, 2, "Waiting for client to setup board...");
    refresh();
    
    // Initialize boards
    BoardData playerBoard(size);
    BoardData enemyBoard(size);
    
    // Host sets up their board
    int boardResult = 0;
    while (boardResult != 1) {
        boardResult = setupPlayerBoard(playerBoard, size);

        // Manual placement mode handling
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
    
    // Initialize enemy board tracking (fog of war)
    std::vector<std::vector<char>> enemyKnownBoard(size, std::vector<char>(size, ' '));
    
    // Configure game mode parameters
    bool isAI = false;
    void* aiPtr = nullptr;
    void* socketPtr = &clientSocket;
    
    // Start main game loop (host goes first)
    GameLoop::runGameLoop(
        playerBoard,
        enemyBoard,
        enemyKnownBoard,
        size,
        shots,
        true,        // playerTurn = true (host starts)
        isAI,
        aiPtr,
        socketPtr,
        true         // isHost = true
    );
    
    // Cleanup
    closesocket(hostSocket);
    clear();
}

// Client multiplayer game - connects to host, receives game settings,
// sets up board, and starts the game with client going second
void playMultiplayerClient() {
    clear();
    // Get terminal dimensions
    int maxY, maxX; 
    getmaxyx(stdscr, maxY, maxX);
    (void)maxY;

    // Prompt for host IP address
    char hostname[100];
    echo();
    mvprintw(8, 2, "Enter host IP address: ");
    getnstr(hostname, 100);
    noecho();

    // Attempt connection to host
    clear();
    mvprintw(8, 2, "Connecting to %s...", hostname);
    refresh();

    SOCKET_TYPE clientSocket = NetworkLogic::createClientSocket(hostname);
    if (clientSocket == INVALID_SOCKET_VALUE) {
        mvprintw(8, 2, "Connection failed. Is the Host started?");
        mvprintw(10, 2, "Press any key to return...");
        refresh();
        getch();
        clear();
        return;
    }

    // Wait for host to send game settings
    mvprintw(8, 2, "Connected! Waiting for host to start game...");
    refresh();

    int size, shots;
    if (!NetworkLogic::receiveGameSettings(clientSocket, size, shots)) {
        clear();
        mvprintw(5, 2, "Error: Connection lost!");
        mvprintw(6, 2, "Press any key to exit...");
        refresh();
        getch();
        closesocket(clientSocket);
        clear();
        return;
    }

    g_gameSettings.shotsPerTurn = shots;

    // Display received game settings
    clear();
    mvprintw(2, 2, "Multiplayer Game (Client)");
    mvprintw(3, 2, "Board: %dx%d | Shots: %d per turn", size, size, shots);
    mvprintw(4, 2, "Host has chosen the settings!");
    refresh();
    SLEEP_MS(2000);

    // Set board size for client
    setBoardSize(size);

    // Initialize boards
    BoardData playerBoard(size);
    BoardData enemyBoard(size);

    // Client sets up their board
    int boardResult = 0;
    while (boardResult != 1) {
        boardResult = setupPlayerBoard(playerBoard, size);

        // Manual placement mode handling
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

    // Initialize enemy board tracking (fog of war)
    std::vector<std::vector<char>> enemyKnownBoard(size, std::vector<char>(size, ' '));

    // Configure game mode parameters
    bool isAI = false;
    void* aiPtr = nullptr;
    void* socketPtr = &clientSocket;

    // Start main game loop (client goes second)
    GameLoop::runGameLoop(
        playerBoard,
        enemyBoard,
        enemyKnownBoard,
        size,
        shots,
        false,       // playerTurn = false (client waits)
        isAI,
        aiPtr,
        socketPtr,
        false        // isHost = false
    );

    clear();
}
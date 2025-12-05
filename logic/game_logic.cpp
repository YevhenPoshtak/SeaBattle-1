/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: game_logic.cpp
 * Description: Core game logic implementation for Battleship.
 *              Handles game initialization, ship placement validation,
 *              board generation, shot processing, and game state management.
 */

#include "game_logic.hpp"
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <algorithm>

// Initialize game state with board size and settings
// state: game state object to initialize
// boardSize: dimensions of the board (NxN)
// shotsPerTurn: number of shots allowed per turn
// isHost: whether this player is the host (affects RNG seed)
void GameLogic::initializeGame(GameState& state, int boardSize, int shotsPerTurn, bool isHost) {
    state.initialize(boardSize, shotsPerTurn, isHost);
    state.totalShips = getTotalShips(boardSize);
    state.maxHits = getTotalShipCells(boardSize);
    state.playerShipsRemaining = state.totalShips;
    state.enemyShipsRemaining = state.totalShips;
}

// Initialize game pieces (ships) based on board configuration
// board: board to initialize pieces for
// pieces: vector to store created game pieces
void GameLogic::initializeGamePieces(BoardData& board, std::vector<GamePiece>& pieces) {
    pieces.clear();
    board.shipStatus.clear();
    board.myShips.clear();
    
    // Get ship configuration for this board size
    ShipConfiguration config = getShipConfig(board.boardSize);
    
    int shipCounter = 0;
    
    // Lambda to create ActiveShip structure
    auto createShip = [&](int len, char sym) -> ActiveShip {
        ActiveShip s;
        s.id = shipCounter;
        s.symbol = sym;
        s.length = len;
        s.hitCount = 0;
        s.isSunk = false;
        s.startRow = -1;
        s.startCol = -1;
        s.orientation = 0;
        return s;
    };

    // Create 4-deck ships (battleships)
    for (int i = 0; i < config.fourDeck; i++) {
        char symbol = 'A' + (shipCounter % 26);
        pieces.push_back(GamePiece(4, symbol));
        board.shipStatus[symbol] = createShip(4, symbol);
        shipCounter++;
    }
    
    // Create 3-deck ships (cruisers)
    for (int i = 0; i < config.threeDeck; i++) {
        char symbol = 'A' + (shipCounter % 26);
        pieces.push_back(GamePiece(3, symbol));
        board.shipStatus[symbol] = createShip(3, symbol);
        shipCounter++;
    }
    
    // Create 2-deck ships (destroyers)
    for (int i = 0; i < config.twoDeck; i++) {
        char symbol = 'A' + (shipCounter % 26);
        pieces.push_back(GamePiece(2, symbol));
        board.shipStatus[symbol] = createShip(2, symbol);
        shipCounter++;
    }
    
    // Create 1-deck ships (submarines)
    for (int i = 0; i < config.oneDeck; i++) {
        char symbol = 'A' + (shipCounter % 26);
        pieces.push_back(GamePiece(1, symbol));
        board.shipStatus[symbol] = createShip(1, symbol);
        shipCounter++;
    }
}

// Generate random placement for all ships on the board
// board: board to place ships on
// pieces: vector of ships to place
void GameLogic::generateBoardPlacement(BoardData& board, const std::vector<GamePiece>& pieces) {
    // Use high-resolution clock for better randomness
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    // Different seeds for host and client to ensure different boards
    if (board.isHost) {
        srand(static_cast<unsigned int>(seed) + 3000);
    } else {
        srand(static_cast<unsigned int>(seed));
    }

    // Attempt to place each ship
    for (size_t i = 0; i < pieces.size(); i++) {
        int starting_peg = (rand() % (board.boardSize * board.boardSize));
        int orientation = (rand() % 2) + 1;  // 1 = vertical, 2 = horizontal
        int piece_length = pieces[i].Get_Piece_Length();
        char piece_symbol = pieces[i].Get_Piece_Symbol();
        
        int ret = 0;
        int attempts = 0;
        const int MAX_ATTEMPTS = 1000;
        
        // Try to find valid placement
        while ((ret = checkStartingPeg(board, orientation, starting_peg, piece_length)) != 1) {
            if (ret == 2) {
                // Out of bounds - try flipping orientation
                orientation = (orientation == 1) ? 2 : 1;
                ret = checkStartingPeg(board, orientation, starting_peg, piece_length);
                if (ret == 1) {
                    break;
                }
            }

            // Try new random position and orientation
            starting_peg = (rand() % (board.boardSize * board.boardSize));
            orientation = (rand() % 2) + 1;
            attempts++;
            
            // If can't place after many attempts, restart entire board
            if (attempts > MAX_ATTEMPTS) {
                board.clear();
                i = -1;  // Restart from beginning
                break;
            }
        }
        
        // Place ship if valid position found
        if (i >= 0 && ret == 1) {
            board.addShip(orientation, starting_peg, piece_length, piece_symbol);
        }
    }
}

// Check if a ship can be placed starting at given position
// board: board to check placement on
// orientation: 1 = vertical (down), 2 = horizontal (left)
// starting_peg: starting position as linear index
// piece_length: length of ship to place
// Returns: 1 = valid, 2 = out of bounds, 3 = collides with existing ship
short GameLogic::checkStartingPeg(const BoardData& board, int orientation, int starting_peg, int piece_length) {
    if (orientation == 1) {
        // Vertical placement (going down)
        for (int j = 0; j < piece_length; j++) {
            if ((starting_peg / board.boardSize) + j >= board.boardSize) {
                return 2;  // Out of bounds
            } else if (board.boardArray[(starting_peg / board.boardSize) + j][(starting_peg % board.boardSize)] != 'w') {
                return 3;  // Collision with existing ship
            }
        }
        return 1;  // Valid placement
    } else {
        // Horizontal placement (going left)
        for (int j = 0; j < piece_length; j++) {
            if ((starting_peg % board.boardSize) - j < 0) {
                return 2;  // Out of bounds
            }
            if (board.boardArray[(starting_peg / board.boardSize)][(starting_peg % board.boardSize) - j] != 'w') {
                return 3;  // Collision with existing ship
            }
        }
        return 1;  // Valid placement
    }
}

// Place a ship manually at specified position
// board: board to place ship on
// gridX, gridY: starting position (grid coordinates)
// orientation: 0 = horizontal (left), 1 = vertical (up)
// length: length of ship
// symbol: character symbol for this ship
// Returns: true if placement successful, false otherwise
bool GameLogic::placeShip(BoardData& board, int gridX, int gridY, int orientation, int length, char symbol) {
    if (!isValidShipPlacement(board, gridX, gridY, orientation, length)) {
        return false;
    }
    
    if (orientation == 0) {
        // Horizontal placement (left from starting point)
        for (int i = 0; i < length; i++) {
            board.boardArray[gridY][gridX - i] = symbol;
        }
    } else {
        // Vertical placement (up from starting point)
        for (int i = 0; i < length; i++) {
            board.boardArray[gridY - i][gridX] = symbol;
        }
    }
    return true;
}

// Validate if a ship can be placed at specified position
// board: board to validate placement on
// gridX, gridY: starting position
// orientation: 0 = horizontal, 1 = vertical
// length: length of ship
// Returns: true if placement is valid, false otherwise
bool GameLogic::isValidShipPlacement(const BoardData& board, int gridX, int gridY, int orientation, int length) {
    if (orientation == 0) {
        // Check horizontal placement
        for (int i = 0; i < length; i++) {
            int checkX = gridX - i;
            if (checkX < 0 || checkX >= board.boardSize || board.boardArray[gridY][checkX] != 'w') {
                return false;
            }
        }
    } else {
        // Check vertical placement
        for (int i = 0; i < length; i++) {
            int checkY = gridY - i;
            if (checkY < 0 || checkY >= board.boardSize || board.boardArray[checkY][gridX] != 'w') {
                return false;
            }
        }
    }
    return true;
}

// Process a shot on the target board
// targetBoard: board to shoot at
// x, y: coordinates to shoot
// Returns: 0 = miss, 1 = hit, 2 = sunk
int GameLogic::processShot(BoardData& targetBoard, int x, int y) {
    return targetBoard.receiveShot(x, y);
}

// Update board to mark all cells of a sunk ship
// board: board to update
// x, y: coordinates of the sinking shot
void GameLogic::updateSunkShips(BoardData& board, int x, int y) {
    auto cells = board.getShipOccupiedCells(x, y);
    for (const auto& cell : cells) {
        board.boardArray[cell.second][cell.first] = 's';
    }
}

// Generate unique symbol for a ship based on its ID
// shipId: numerical ID of the ship
// Returns: string symbol (single or double character)
std::string GameLogic::generateShipSymbol(int shipId) {
    if (shipId < 26) {
        // Single character: a-z
        return std::string(1, 'a' + shipId);
    } else {
        // Double character: Aa, Ab, ..., Ba, Bb, ...
        int first = (shipId / 26) - 1;
        int second = shipId % 26;
        return std::string(1, 'A' + first) + std::string(1, 'a' + second);
    }
}

// Recursively mark all connected ship parts using DFS
// Used for counting remaining ships
// r, c: current position to check
// size: board size
// board: board array to check
// visited: tracking array for visited cells
void GameLogic::markShipParts(int r, int c, int size, const std::vector<std::vector<char>>& board, std::vector<std::vector<bool>>& visited) {
    // Check bounds
    if (r < 0 || r >= size || c < 0 || c >= size) return;
    
    char cell = board[r][c];
    bool isShip = (cell != 'w' && cell != 'o' && cell != 'x' && cell != ' ');
    
    // Check if already visited or not a ship cell
    if (visited[r][c] || !isShip) return;

    visited[r][c] = true;

    // Recursively mark all 4 neighbors
    markShipParts(r + 1, c, size, board, visited);
    markShipParts(r - 1, c, size, board, visited);
    markShipParts(r, c + 1, size, board, visited);
    markShipParts(r, c - 1, size, board, visited);
}

// Count number of remaining intact ships on the board
// boardArray: 2D array representing the board
// size: board size
// Returns: number of ships that are not sunk
int GameLogic::countRemainingShips(const std::vector<std::vector<char>>& boardArray, int size) {
    int count = 0;
    std::vector<std::vector<bool>> visited(size, std::vector<bool>(size, false));
    
    // Find all connected ship components
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            char cell = boardArray[i][j];
            bool isShip = (cell != 'w' && cell != 'o' && cell != 'x' && cell != ' ');
            
            if (isShip && !visited[i][j]) {
                count++;  // Found a new ship
                markShipParts(i, j, size, boardArray, visited);
            }
        }
    }   
    return count;
}
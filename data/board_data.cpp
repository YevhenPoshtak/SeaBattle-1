/*
 * Battleship-1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: board_data.cpp
 * Description: Implementation of the BoardData class which manages the game board state,
 *              ship placement, shot processing, and ship status tracking for the battleship game.
 */

#include "board_data.hpp"
#include <algorithm>

// Default constructor - initializes 10x10 board filled with water ('w')
BoardData::BoardData() : boardSize(10), missCount(0), isHost(true) {
    boardArray.resize(10, std::vector<char>(10, 'w'));
}

// Parameterized constructor - initializes board with custom size
BoardData::BoardData(int size) : boardSize(size), missCount(0), isHost(true) {
    boardArray.resize(size, std::vector<char>(size, 'w'));
}

// Initialize board with specified size and reset all data structures
void BoardData::initialize(int size) {
    boardSize = size;
    boardArray.clear();
    boardArray.resize(size, std::vector<char>(size, 'w'));
    myShips.clear();
    shipStatus.clear();
    shipCellMap.clear();
    missCount = 0;
}

// Clear the board and reset all ship-related data
void BoardData::clear() {
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            boardArray[i][j] = 'w';
        }
    }
    myShips.clear();
    shipStatus.clear();
    shipCellMap.clear();
    missCount = 0;
}

// Resize the board to new dimensions
void BoardData::resize(int newSize) {
    boardArray.clear();
    boardArray.resize(newSize, std::vector<char>(newSize, 'w'));
    boardSize = newSize;
}

// Process incoming shot at coordinates (x, y)
// Returns: 0 = miss, 1 = hit, 2 = ship sunk
int BoardData::receiveShot(int x, int y) {
    // Check bounds
    if (x < 0 || x >= boardSize || y < 0 || y >= boardSize) return 0;
    
    char cell = boardArray[y][x];
    
    // Already hit - no action needed
    if (cell == 'x' || cell == 'o' || cell == 's') return 0;

    // Water hit - mark as miss
    if (cell == 'w') { 
        boardArray[y][x] = 'o';  
        missCount++; 
        return 0; 
    }

    // Ship hit - process hit logic
    if (cell >= 'A' && cell <= 'Z') {
        char targetSymbol = cell;
        
        // Find the ship that was hit
        for (auto& ship : myShips) {
            if (ship.isSunk) continue;

            if (ship.symbol != targetSymbol) continue;
            
            bool isHit = false;

            // Check if coordinates belong to this ship
            if (ship.length == 1) {
                if (x == ship.startCol && y == ship.startRow) {
                    isHit = true;
                }
            }
            else if (ship.orientation == 1) { // Vertical orientation
                if (x == ship.startCol && y >= ship.startRow && y < ship.startRow + ship.length) {
                    isHit = true;
                }
            } else { // Horizontal orientation
                if (y == ship.startRow && x <= ship.startCol && x > ship.startCol - ship.length) {
                    isHit = true;
                }
            }

            if (isHit) {
                ship.hitCount++;
                
                // Check if ship is completely sunk
                if (ship.hitCount >= ship.length) {
                    ship.isSunk = true;

                    shipStatus[ship.symbol].isSunk = true;
                    shipStatus[ship.symbol].hitCount = ship.hitCount;
                    
                    // Mark all ship cells as sunk
                    for (int i = 0; i < ship.length; i++) {
                        int r = ship.startRow;
                        int c = ship.startCol;

                        if (ship.orientation == 1) {
                            r += i; 
                        } else {
                            c -= i; 
                        }
                        
                        if (r >= 0 && r < boardSize && c >= 0 && c < boardSize) {
                            boardArray[r][c] = 's'; 
                        }
                    }
                    return 2; // Ship sunk
                } else {
                    boardArray[y][x] = 'x';  // Mark as hit
                    
                    shipStatus[ship.symbol].hitCount = ship.hitCount;
                    
                    return 1; // Hit but not sunk
                }
            }
        }
    }

    boardArray[y][x] = 'x';
    return 1; 
}

// Check if a specific ship is completely sunk
bool BoardData::isShipSunk(char shipSymbol) {
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (boardArray[i][j] == shipSymbol) {
                return false;
            }
        }
    }
    return true;
}

// Mark a ship as hit and update its status
void BoardData::markShipAsHit(char shipSymbol) {
    if (shipStatus.find(shipSymbol) != shipStatus.end()) {
        shipStatus[shipSymbol].hitCount++;
        if (shipStatus[shipSymbol].hitCount >= shipStatus[shipSymbol].length) {
            shipStatus[shipSymbol].isSunk = true;
        }
    }
}

// Get count of ships that are still afloat
int BoardData::getRemainingShips() {
    int count = 0;
    for (auto& pair : shipStatus) {
        if (!pair.second.isSunk) {
            count++;
        }
    }
    return count;
}

// Get total count of hit cells on ships that are not yet sunk
int BoardData::getWoundedCount() {
    int count = 0;
    for (const auto& ship : myShips) {
        if (!ship.isSunk && ship.hitCount > 0) {
            count += ship.hitCount; 
        }
    }
    return count;
}

// Get count of completely sunk ships
int BoardData::getSunkCount() {
    int count = 0;
    for (const auto& ship : myShips) {
        if (ship.isSunk) count++;
    }
    return count;
}

// Get all coordinates occupied by a specific ship
std::vector<std::pair<int, int>> BoardData::getShipCoordinates(char shipSymbol) {
    std::vector<std::pair<int, int>> coords;
    for (const auto& pair : shipCellMap) {
        if (pair.second == shipSymbol) {
            coords.push_back(pair.first);
        }
    }
    return coords;
}

// Get all cells occupied by the ship at given coordinates
std::vector<std::pair<int, int>> BoardData::getShipOccupiedCells(int x, int y) {
    std::vector<std::pair<int, int>> cells;

    // Find which ship occupies this cell
    for (const auto& ship : myShips) {
        bool belongsToShip = false;
        if (ship.orientation == 1) { // Vertical
            if (x == ship.startCol && y >= ship.startRow && y < ship.startRow + ship.length) {
                belongsToShip = true;
            }
        } else { // Horizontal
            if (y == ship.startRow && x <= ship.startCol && x > ship.startCol - ship.length) {
                belongsToShip = true;
            }
        }

        if (belongsToShip) {
            // Collect all cells of this ship
            for (int i = 0; i < ship.length; i++) {
                int r = ship.startRow;
                int c = ship.startCol;

                if (ship.orientation == 1) {
                    r += i; 
                } else {
                    c -= i; 
                }
                cells.push_back({c, r}); 
            }
            return cells; 
        }
    }
    return cells; 
}

// Build mapping of coordinates to ship symbols
void BoardData::buildShipCellMap() {
    shipCellMap.clear();
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            char cell = boardArray[i][j];
            if (cell >= 'A' && cell <= 'Z') {
                shipCellMap[{j, i}] = cell;
            }
        }
    }
}

// Add a new ship to the board
// orientation: 1 = vertical, 0 = horizontal
// startPos: linear position on board (row * boardSize + col)
void BoardData::addShip(int orientation, int startPos, int length, char symbol) {
    int row = startPos / boardSize;
    int col = startPos % boardSize;

    // Create new ship object
    ActiveShip newShip;
    newShip.startRow = row;
    newShip.startCol = col;
    newShip.length = length;
    newShip.orientation = orientation;
    newShip.symbol = symbol;
    newShip.hitCount = 0;
    newShip.isSunk = false;
    newShip.id = (int)myShips.size();

    myShips.push_back(newShip);
    shipStatus[symbol] = newShip;

    // Place ship on board
    if (orientation == 1) { // Vertical
        for (int j = 0; j < length; j++) {
            boardArray[row + j][col] = symbol;
        }
    } else { // Horizontal
        for (int j = 0; j < length; j++) {
            boardArray[row][col - j] = symbol;
        }
    }
}
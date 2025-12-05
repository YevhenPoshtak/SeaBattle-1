/*
 * Battleship-1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: board_data.hpp
 * Description: Header file defining the BoardData class and ActiveShip structure.
 *              Contains declarations for board management, ship tracking, and game logic.
 */

#ifndef BOARD_DATA_HPP
#define BOARD_DATA_HPP

#include <vector>
#include <map>
#include <utility>

// Structure representing an active ship on the board
struct ActiveShip {
    int id;              // Unique identifier for the ship
    char symbol;         // Character symbol representing the ship (A-Z)
    int length;          // Length of the ship in cells
    int hitCount;        // Number of times ship has been hit
    bool isSunk;         // Flag indicating if ship is completely destroyed
    int startRow;        // Starting row position
    int startCol;        // Starting column position
    int orientation;     // 1 = vertical, 0 = horizontal
};

// Main class managing the game board state and operations
class BoardData {
public:
    std::vector<std::vector<char>> boardArray;           // 2D array representing board state
    std::vector<ActiveShip> myShips;                     // List of all ships on this board
    std::map<char, ActiveShip> shipStatus;               // Map of ship symbols to their status
    std::map<std::pair<int, int>, char> shipCellMap;     // Map of coordinates to ship symbols
    int boardSize;                                        // Size of the board (NxN)
    int missCount;                                        // Count of missed shots
    bool isHost;                                          // Flag indicating if this is host's board

    // Constructors
    BoardData();
    BoardData(int size);
    
    // Board initialization and management
    void initialize(int size);              // Initialize board with given size
    void clear();                           // Clear all board data
    void resize(int newSize);               // Resize board to new dimensions
    
    // Shot and hit processing
    int receiveShot(int x, int y);          // Process incoming shot, returns hit status
    bool isShipSunk(char shipSymbol);       // Check if specific ship is sunk
    void markShipAsHit(char shipSymbol);    // Mark ship as hit and update status
    
    // Statistics and status queries
    int getRemainingShips();                 // Get count of ships still afloat
    int getWoundedCount();                   // Get count of hit cells on unsunk ships
    int getSunkCount();                      // Get count of completely sunk ships
    int getMissCount() const { return missCount; }  // Get total miss count
    
    // Ship coordinate queries
    std::vector<std::pair<int, int>> getShipCoordinates(char shipSymbol);  // Get all coords for ship
    std::vector<std::pair<int, int>> getShipOccupiedCells(int x, int y);   // Get cells of ship at (x,y)
    
    // Ship management
    void buildShipCellMap();                // Build coordinate-to-ship mapping
    void addShip(int orientation, int startPos, int length, char symbol);  // Add ship to board
    
    // Getters and setters
    void setIsHost(bool host) { isHost = host; }     // Set host flag
    int getBoardSize() const { return boardSize; }   // Get board size
};

#endif
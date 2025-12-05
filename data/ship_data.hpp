/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 *
 * File: ship_data.hpp
 * Description: Header file defining ship-related structures and functions including
 *              ship configurations for different board sizes, ship status tracking,
 *              and the GamePiece class for ship representation.
 */

#ifndef SHIP_DATA_HPP
#define SHIP_DATA_HPP

// Configuration structure defining ship counts for a specific board size
struct ShipConfiguration {
    int boardSize;      // Size of the board (NxN)
    int fourDeck;       // Number of 4-cell ships
    int threeDeck;      // Number of 3-cell ships
    int twoDeck;        // Number of 2-cell ships
    int oneDeck;        // Number of 1-cell ships
    int shotsPerTurn;   // Shots allowed per turn for this configuration
};

// Structure representing a ship in the game
struct Ship {
    char symbol;        // Character representing the ship (A-Z)
    int length;         // Length of the ship in cells
    int hitsRemaining;  // Number of cells not yet hit
    bool isSunk;        // Flag indicating if ship is destroyed
    
    // Default constructor
    Ship() : symbol('\0'), length(0), hitsRemaining(0), isSunk(false) {}
    // Parameterized constructor
    Ship(char s, int l) : symbol(s), length(l), hitsRemaining(l), isSunk(false) {}
};

// Class representing a game piece (ship) with encapsulated properties
class GamePiece {
private:
    int piece_length;       // Length of the ship
    char piece_symbol;      // Symbol representing the ship
    
public:
    // Constructors
    GamePiece();
    GamePiece(int length, char symbol);
    
    // Getter methods
    int Get_Piece_Length() const { return piece_length; }
    char Get_Piece_Symbol() const { return piece_symbol; }
};

// Function to get ship configuration for a specific board size
// Returns the number of ships of each type and shots per turn
inline ShipConfiguration getShipConfig(int boardSize) {
    // Predefined configurations for board sizes 10-26
    static const ShipConfiguration configs[] = {
        {10, 1, 2, 3, 4, 5},    // 10x10: 1x4-deck, 2x3-deck, 3x2-deck, 4x1-deck, 5 shots
        {11, 1, 2, 4, 5, 5},
        {12, 1, 3, 4, 6, 5},
        {13, 1, 3, 5, 6, 5},
        {14, 2, 3, 5, 7, 6},
        {15, 2, 4, 6, 8, 6},
        {16, 2, 4, 6, 9, 6},
        {17, 2, 4, 7, 9, 6},
        {18, 2, 5, 7, 10, 7},
        {19, 3, 5, 8, 11, 7},
        {20, 3, 5, 8, 12, 7},
        {21, 3, 6, 9, 13, 7},
        {22, 3, 6, 9, 14, 7},
        {23, 4, 6, 10, 15, 8},
        {24, 4, 7, 10, 16, 8},
        {25, 4, 7, 11, 17, 8},
        {26, 4, 7, 11, 18, 9}   // 26x26: 4x4-deck, 7x3-deck, 11x2-deck, 18x1-deck, 9 shots
    };
    
    // Find configuration matching the requested board size
    for (const auto& config : configs) {
        if (config.boardSize == boardSize) {
            return config;
        }
    }
    
    // Default to 10x10 configuration if size not found
    return {10, 1, 2, 3, 4, 5};
}

// Calculate total number of ships for a given board size
inline int getTotalShips(int boardSize) {
    auto config = getShipConfig(boardSize);
    return config.fourDeck + config.threeDeck + config.twoDeck + config.oneDeck;
}

// Calculate total number of ship cells (max hits needed to win) for a given board size
inline int getTotalShipCells(int boardSize) {
    auto config = getShipConfig(boardSize);
    return config.fourDeck * 4 + config.threeDeck * 3 + 
           config.twoDeck * 2 + config.oneDeck * 1;
}

#endif
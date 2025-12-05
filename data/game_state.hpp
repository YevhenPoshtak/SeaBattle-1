/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: game_state.hpp
 * Description: Header file defining game state structures including GameState, GameSettings,
 *              and coordinate structures used throughout the game.
 */

#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "board_data.hpp"
#include "ship_data.hpp"
#include <vector>

// Structure for game configuration settings
struct GameSettings {
    int shotsPerTurn;               // Number of shots allowed per turn
    GameSettings() : shotsPerTurn(3) {}
};

// Main structure managing complete game state
struct GameState {
    BoardData playerBoard;          // Player's own board with ships
    BoardData enemyBoard;            // Enemy's board (hidden)
    
    int playerHits;                  // Total successful hits by player
    int enemyHits;                   // Total successful hits by enemy
    int maxHits;                     // Total hits needed to win
    
    int playerShipsRemaining;        // Number of player's ships still afloat
    int enemyShipsRemaining;         // Number of enemy's ships still afloat
    int totalShips;                  // Total number of ships per player
    
    bool playerTurn;                 // True if it's player's turn
    bool isHost;                     // True if this player is the host
    
    int boardSize;                   // Size of the game board (NxN)
    int shotsPerTurn;                // Shots allowed per turn
    
    std::vector<std::vector<char>> enemyKnownBoard;  // Fog of war for enemy board
    
    // Constructor and methods
    GameState();
    void initialize(int size, int shots, bool host);  // Initialize game with parameters
    void reset();                                      // Reset game to initial state
    bool isGameOver() const;                          // Check if game has ended
    bool hasPlayerWon() const;                        // Check if player won
};

// Structure for AI coordinate selection
struct AICoordinates {
    int x;      // X coordinate
    int y;      // Y coordinate
};

// General coordinate structure
struct coordinates {
    int x;      // X coordinate
    int y;      // Y coordinate
};

// Coordinate structure with character data
struct char_coordinates {
    int x;      // X coordinate
    int y;      // Y coordinate
    char c;     // Character at this position
};

#endif
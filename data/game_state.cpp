/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: game_state.cpp
 * Description: Implementation of the GameState class which manages the overall game state,
 *              including both player and enemy boards, turn management, and win conditions.
 */

#include "game_state.hpp"

// Default constructor - initializes game state with default values
GameState::GameState() 
    : playerHits(0), enemyHits(0), maxHits(0),
      playerShipsRemaining(0), enemyShipsRemaining(0), totalShips(0),
      playerTurn(true), isHost(true),
      boardSize(10), shotsPerTurn(3) {
}

// Initialize game state with specified parameters
// size: board dimensions (NxN)
// shots: number of shots allowed per turn
// host: whether this player is the host
void GameState::initialize(int size, int shots, bool host) {
    boardSize = size;
    shotsPerTurn = shots;
    isHost = host;
    
    // Initialize both boards
    playerBoard.initialize(size);
    enemyBoard.initialize(size);
    
    // Initialize enemy known board (fog of war)
    enemyKnownBoard.clear();
    enemyKnownBoard.resize(size, std::vector<char>(size, ' '));
    
    // Reset hit counters
    playerHits = 0;
    enemyHits = 0;
    
    // Host goes first
    playerTurn = host;
}

// Reset game state to start new game while keeping configuration
void GameState::reset() {
    playerHits = 0;
    enemyHits = 0;
    playerShipsRemaining = totalShips;
    enemyShipsRemaining = totalShips;
    playerTurn = isHost;
    
    // Clear both boards
    playerBoard.clear();
    enemyBoard.clear();
    
    // Reset fog of war
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            enemyKnownBoard[i][j] = ' ';
        }
    }
}

// Check if game has ended (either player reached max hits)
bool GameState::isGameOver() const {
    return playerHits >= maxHits || enemyHits >= maxHits;
}

// Check if player has won the game
bool GameState::hasPlayerWon() const {
    return playerHits >= maxHits;
}
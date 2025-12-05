/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ai_logic.hpp
 * Description: Header file for AI opponent logic. Defines the AILogic class
 *              with support for multiple difficulty levels and intelligent
 *              targeting algorithms for the Battleship game.
 */

#ifndef AI_LOGIC_HPP
#define AI_LOGIC_HPP

#include "../data/board_data.hpp"
#include "../data/game_state.hpp"
#include <vector>
#include <deque>

// AI difficulty levels
enum AIDifficulty { EASY, SMART };

class AILogic {
private:
    AIDifficulty difficulty;                        // Current AI difficulty level
    BoardData aiBoard;                              // AI's own board with ships
    std::vector<std::vector<char>> opponentBoard;   // AI's knowledge of player's board
    
    AICoordinates lastHit;                          // Last successful hit coordinates
    bool hunting;                                    // Whether AI is in hunt mode
    int huntDirection;                               // Current hunting direction
    
    std::vector<AICoordinates> availableShots;      // All remaining available shots
    std::deque<AICoordinates> targetQueue;          // Priority targets (neighbors of hits)
    std::vector<AICoordinates> parityShots;         // Checkerboard pattern shots
    
    unsigned int attackSeed;                         // Random seed for attacks
    int boardSize;                                   // Size of game board
    
    // Initialize all possible shot coordinates
    void initializeAvailableShots();
    
    // Add neighboring cells to target queue after a hit
    void addSmartNeighbors(int x, int y);
    
public:
    // Constructor - initializes AI with difficulty and board size
    AILogic(AIDifficulty diff, int size);
    
    // Generate AI's board with random ship placement
    void setupBoard();
    
    // Select next coordinates to attack based on AI strategy
    AICoordinates pickAttackCoordinates();
    
    // Record result of a shot and update AI strategy
    void recordShotResult(int x, int y, bool isHit, bool isSunk);
    
    // Check if coordinates are valid
    bool isValidCoordinate(int x, int y);
    
    // Clear the target queue
    void clearTargetQueue();
    
    // Reset AI state for new game
    void reset();
    
    // Getters
    BoardData& getBoard() { return aiBoard; }
    AIDifficulty getDifficulty() const { return difficulty; }
};

#endif
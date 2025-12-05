/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ai_logic.cpp
 * Description: Implementation of AI opponent logic for Battleship game.
 *              Supports two difficulty levels: EASY (random) and SMART (targeted).
 *              Smart AI uses parity targeting and hunt mode for efficient ship destruction.
 */

#include "ai_logic.hpp"
#include "game_logic.hpp"
#include <algorithm>
#include <ctime>
#include <cstdlib>

// Constructor - initializes AI with difficulty level and board size
// diff: AI difficulty (EASY or SMART)
// size: board dimensions (NxN)
AILogic::AILogic(AIDifficulty diff, int size) 
    : difficulty(diff), 
      aiBoard(size),
      hunting(false), 
      huntDirection(0),
      boardSize(size) {
    
    // Initialize opponent board tracking (AI's view of player board)
    opponentBoard.resize(size, std::vector<char>(size, '?'));
    
    // Initialize attack coordinates tracking
    lastHit.x = -1;
    lastHit.y = -1;
    
    // Seed random number generator
    attackSeed = time(NULL) + 12345;
    
    // Initialize targeting queues
    targetQueue.clear();
    parityShots.clear();
    
    // Generate all available shot coordinates
    initializeAvailableShots();
    
    // Generate AI's board with random ship placement
    setupBoard();
}

// Generate AI's board with random ship placement
void AILogic::setupBoard() {
    aiBoard.setIsHost(false);
    aiBoard.initialize(boardSize);
    
    // Initialize and place ships randomly
    std::vector<GamePiece> pieces;
    GameLogic::initializeGamePieces(aiBoard, pieces);
    GameLogic::generateBoardPlacement(aiBoard, pieces);
    aiBoard.buildShipCellMap();
}

// Initialize list of all possible shot coordinates
// For SMART AI, also creates parity shot list (checkerboard pattern)
void AILogic::initializeAvailableShots() {
    availableShots.clear();
    parityShots.clear();
    
    // Generate all coordinates
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            AICoordinates coord;
            coord.x = j;
            coord.y = i;
            availableShots.push_back(coord);
            
            // Add to parity shots (checkerboard pattern)
            // This helps find ships faster since ships occupy 2+ consecutive cells
            if ((j + i) % 2 == 0) {
                parityShots.push_back(coord);
            }
        }
    }

    // Randomize shot order for unpredictability
    std::srand(std::time(0));
    std::random_shuffle(availableShots.begin(), availableShots.end());
    std::random_shuffle(parityShots.begin(), parityShots.end());
}

// Add all valid neighboring cells to target queue (for smart AI)
// Called when a ship is hit but not yet sunk
// x, y: coordinates of confirmed hit
void AILogic::addSmartNeighbors(int x, int y) {
    // Check all 4 orthogonal neighbors (up, down, left, right)
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};
    
    for(int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        
        // Validate coordinates and check if cell hasn't been shot
        if(nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
            if(opponentBoard[ny][nx] == '?' || opponentBoard[ny][nx] == ' ') {
                AICoordinates c;
                c.x = nx;
                c.y = ny;
                targetQueue.push_back(c);
            }
        }
    }
}

// Select next attack coordinates based on AI difficulty
// Returns: coordinates to attack, or (-1, -1) if no valid shots remain
AICoordinates AILogic::pickAttackCoordinates() {
    AICoordinates coord;
    coord.x = -1; 
    coord.y = -1;

    // No shots available
    if (availableShots.empty()) return coord;

    // EASY MODE: completely random targeting
    if (difficulty == EASY) {
        int index = rand() % availableShots.size();
        coord = availableShots[index];
        availableShots.erase(availableShots.begin() + index);
        return coord;
    }

    // SMART MODE: prioritized targeting strategy
    
    // Priority 1: Target queued cells (neighbors of previous hits)
    while (!targetQueue.empty()) {
        coord = targetQueue.front();
        targetQueue.pop_front();
        
        // Verify coordinate is still available
        bool valid = false;
        int removeIndex = -1;
        
        for (size_t i = 0; i < availableShots.size(); i++) {
            if (availableShots[i].x == coord.x && availableShots[i].y == coord.y) {
                valid = true;
                removeIndex = i;
                break;
            }
        }
        
        if (valid) {
            // Remove from both available lists
            availableShots.erase(availableShots.begin() + removeIndex);
            for(size_t i=0; i<parityShots.size(); ++i) {
                if(parityShots[i].x == coord.x && parityShots[i].y == coord.y) {
                    parityShots.erase(parityShots.begin() + i);
                    break;
                }
            }
            return coord;
        }
    }

    // Priority 2: Use parity targeting (checkerboard pattern)
    // This finds ships more efficiently than pure random
    if (!parityShots.empty()) {
        coord = parityShots.back();
        parityShots.pop_back();
        
        // Remove from available shots list
        for (size_t i = 0; i < availableShots.size(); i++) {
            if (availableShots[i].x == coord.x && availableShots[i].y == coord.y) {
                availableShots.erase(availableShots.begin() + i);
                break;
            }
        }
        return coord;
    }

    // Priority 3: Random shot from remaining available coordinates
    int index = rand() % availableShots.size();
    coord = availableShots[index];
    availableShots.erase(availableShots.begin() + index);
    return coord;
}

// Record the result of a shot and update AI strategy
// x, y: coordinates that were attacked
// isHit: true if shot hit a ship
// isSunk: true if shot sunk a ship
void AILogic::recordShotResult(int x, int y, bool isHit, bool isSunk) {
    // Validate coordinates
    if (x >= 0 && x < boardSize && y >= 0 && y < boardSize) {
        // Update opponent board tracking
        opponentBoard[y][x] = isHit ? 'X' : 'O';
        
        // Smart AI adjusts strategy based on results
        if (difficulty == SMART) {
            if (isHit && !isSunk) {
                // Hit but not sunk - add neighbors to target queue
                addSmartNeighbors(x, y);
            }
            
            if (isSunk) {
                // Ship sunk - exit hunt mode
                hunting = false;
                lastHit.x = -1;
                lastHit.y = -1;
            }
        }

        // Track last hit for potential follow-up
        if (isHit) {
            lastHit.x = x;
            lastHit.y = y;
            hunting = true;
        }
    }
}

// Check if coordinates are within board bounds
// Returns: true if valid, false otherwise
bool AILogic::isValidCoordinate(int x, int y) {
    return (x >= 0 && x < boardSize && y >= 0 && y < boardSize);
}

// Clear the target queue (used when resetting or changing strategy)
void AILogic::clearTargetQueue() {
    targetQueue.clear();
}

// Reset AI state for a new game
void AILogic::reset() {
    // Reset opponent board tracking
    opponentBoard.assign(boardSize, std::vector<char>(boardSize, '?'));
    
    // Reset hunt mode tracking
    lastHit.x = -1;
    lastHit.y = -1;
    
    hunting = false;
    huntDirection = 0;
    
    // Clear targeting data
    clearTargetQueue();
    initializeAvailableShots();
}
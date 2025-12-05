/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: game_loop.hpp
 * Description: Header file for the main game loop. Declares the GameLoop class
 *              which manages the turn-based gameplay, shot processing, and
 *              communication with AI or network opponents.
 */

#ifndef GAME_LOOP_HPP
#define GAME_LOOP_HPP

#include "../data/board_data.hpp"
#include "../data/game_state.hpp"
#include "../ui/ui_config.hpp"
#include <vector>

// Main class managing the game loop
class GameLoop {
public:
    // Main game loop function that handles turn-based gameplay
    // playerBoard: player's own board with ships
    // enemyBoard: opponent's board (hidden from player)
    // enemyKnownBoard: player's view of enemy board (fog of war)
    // size: board dimensions (NxN)
    // shots: number of shots allowed per turn
    // playerTurn: true if it's player's turn
    // isAI: true if playing against AI, false for network game
    // aiPtr: pointer to AILogic object (if AI game)
    // socketPtr: pointer to socket (if network game)
    // isHost: true if player is host in network game
    static void runGameLoop(
        BoardData& playerBoard,
        BoardData& enemyBoard,
        std::vector<std::vector<char>>& enemyKnownBoard,
        int size,
        int shots,
        bool playerTurn,
        bool& isAI,
        void* aiPtr,
        void* socketPtr,
        bool isHost
    );

private:
    // Structure to track pending shot selections during player's turn
    struct PendingShot {
        int x, y;       // Coordinates of the shot
        bool used;      // Flag indicating if this slot is used
    };
};

#endif
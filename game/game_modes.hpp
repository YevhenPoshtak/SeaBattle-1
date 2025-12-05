/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: game_modes.hpp
 * Description: Enumeration defining available game modes in the main menu.
 *              Includes AI games, multiplayer modes, settings, and debug options.
 */

#ifndef GAME_MODES_HPP
#define GAME_MODES_HPP

// Enumeration of all available game modes
enum GameMode {
    MODE_AI_EASY = 0,              // Play against Easy AI
    MODE_AI_SMART = 1,             // Play against Smart AI
    MODE_MULTIPLAYER_HOST = 2,     // Host a multiplayer game
    MODE_MULTIPLAYER_CLIENT = 3,   // Join a multiplayer game as client
    MODE_BOARD_SIZE_SETTINGS = 4,  // Configure board size settings
    MODE_DEBUG_TESTS = 5,          // Run debug and test functions
    MODE_QUIT = 6                  // Exit the game
};

#endif
/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: game_controller.hpp
 * Description: Header file for game controller functions. Declares the main function
 *              for setting up player board with automatic generation options.
 */

#ifndef GAME_CONTROLLER_HPP
#define GAME_CONTROLLER_HPP

#include "../data/board_data.hpp"
#include "../data/game_state.hpp"

// Setup player board with automatic generation and user interaction
// board: reference to BoardData to be initialized and populated
// size: board dimensions (NxN)
// Returns: 1 = accepted, 0 = manual mode, -1 = regenerate
int setupPlayerBoard(BoardData& board, int size);

#endif
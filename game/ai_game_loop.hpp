/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ai_game_loop.hpp
 * Description: Header file for AI game mode functionality. Declares the main function
 *              to start a game against an AI opponent with specified difficulty level.
 */

#ifndef AI_GAME_LOOP_HPP
#define AI_GAME_LOOP_HPP

#include "../logic/ai_logic.hpp"

// Function to start and run a game against AI opponent
// difficulty: EASY or SMART AI difficulty level
void playAIGame(AIDifficulty difficulty);

#endif
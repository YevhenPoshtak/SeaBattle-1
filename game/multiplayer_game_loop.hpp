/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: multiplayer_game_loop.hpp
 * Description: Header file for multiplayer game functionality. Declares functions
 *              for hosting and joining network-based multiplayer games.
 */

#ifndef MULTIPLAYER_GAME_LOOP_HPP
#define MULTIPLAYER_GAME_LOOP_HPP

// Function to host a multiplayer game
// Creates a server socket, waits for client connection,
// negotiates game settings, and starts the game loop
void playMultiplayerHost();

// Function to join a multiplayer game as client
// Connects to host server, receives game settings,
// and starts the game loop
void playMultiplayerClient();

#endif
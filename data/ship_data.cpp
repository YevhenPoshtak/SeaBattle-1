/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 *
 * File: ship_data.cpp
 * Description: Implementation of the GamePiece class which represents individual ship pieces
 *              in the battleship game with their length and symbol properties.
 */

#include "ship_data.hpp"

// Default constructor - initializes empty game piece
GamePiece::GamePiece() {
    piece_length = 0;        // No length
    piece_symbol = '\0';     // Null character
}

// Parameterized constructor - creates game piece with specified properties
// length: length of the ship in cells
// symbol: character symbol representing this ship
GamePiece::GamePiece(int length, char symbol) {
    piece_length = length;
    piece_symbol = symbol;
}
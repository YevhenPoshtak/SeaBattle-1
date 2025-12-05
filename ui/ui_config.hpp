/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ui_config.hpp
 * Description: Configuration header for UI rendering. Defines board size limits,
 *              layout calculation functions, and cross-platform ncurses compatibility.
 */

#ifndef UI_CONFIG_HPP
#define UI_CONFIG_HPP

#ifdef _WIN32
    #include <pdcurses.h>
#else
    #include <ncurses.h>
#endif

// Board size constraints
const int MIN_BOARD_SIZE = 10;
const int MAX_BOARD_SIZE = 26;

// Set and get current board size
void setBoardSize(int size);
int getBoardSize();

// Check if current terminal can fit the game interface
bool canFitInterface(int boardSize, int maxY, int maxX);

// Calculate minimum terminal dimensions for given board size
void getRequiredTerminalSize(int boardSize, int &minY, int &minX);

// Structure to hold calculated positions for UI elements
struct BoardLayout {
    int startY;          // Starting Y position for boards
    int board1StartX;    // X position for player's board
    int board2StartX;    // X position for opponent's board
    int separatorX;      // X position for separator between boards
    int instructionsY;   // Y position for instruction text
    int logStartX;       // X position for game log/messages
    int statusY;         // Y position for status messages
};

// Calculate optimal layout based on board size and terminal dimensions
BoardLayout calculateBoardLayout(int boardSize);

// Convert column index to letter (0='A', 1='B', etc.)
char getColumnLetter(int index);

#endif
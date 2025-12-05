/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ui_config.cpp
 * Description: Implementation of UI configuration functions. Handles board size
 *              management, terminal size validation, and dynamic layout calculation
 *              to center game elements on screen.
 */

#include "ui_config.hpp"
#include <algorithm>

// Global board size variable
static int BOARD_SIZE = 10;

// Set board size with bounds checking
void setBoardSize(int size) {
    BOARD_SIZE = std::max(MIN_BOARD_SIZE, std::min(MAX_BOARD_SIZE, size));
}

// Get current board size
int getBoardSize() {
    return BOARD_SIZE;
}

// Check if terminal dimensions are sufficient for game interface
bool canFitInterface(int boardSize, int maxY, int maxX) {
    int minY = boardSize + 20;  // Vertical space needed
    int minX = 2 * (8 + boardSize * 4) + 5;  // Horizontal space for two boards
    return (maxY >= minY && maxX >= minX);
}

// Calculate minimum required terminal size for given board
void getRequiredTerminalSize(int boardSize, int &minY, int &minX) {
    minY = boardSize + 20;
    minX = 2 * (8 + boardSize * 4) + 5;
}

// Calculate optimal layout positions for all UI elements
BoardLayout calculateBoardLayout(int boardSize) {
    BoardLayout layout;
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    // Calculate widths
    int boardWidth = boardSize * 4 + 8;  // Each cell is 4 chars wide, plus labels
    int separatorWidth = 5;
    int totalWidth = boardWidth * 2 + separatorWidth;
    
    // Center boards horizontally
    layout.board1StartX = (maxX - totalWidth) / 2;
    if (layout.board1StartX < 3) layout.board1StartX = 3;
    
    layout.separatorX = layout.board1StartX + boardWidth;
    layout.board2StartX = layout.separatorX + separatorWidth;
    
    // Center boards vertically
    int boardHeight = boardSize + 6;
    layout.startY = (maxY - boardHeight - 12) / 2 + 4;
    if (layout.startY < 10) layout.startY = 10;
    
    // Position other UI elements relative to boards
    layout.instructionsY = layout.startY + boardHeight + 2;
    layout.statusY = layout.instructionsY + 3;
    layout.logStartX = layout.board2StartX + boardWidth + 3;
    
    // Ensure log doesn't go off screen
    if (layout.logStartX + 50 > maxX) {
        layout.logStartX = maxX - 50;
    }
    
    return layout;
}

// Convert column index to letter label
char getColumnLetter(int index) {
    if (index >= 0 && index < 26) {
        return 'A' + index;
    }
    return '?';
}
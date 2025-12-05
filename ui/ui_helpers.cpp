/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ui_helpers.cpp
 * Description: Implementation of UI helper functions for coordinate conversions.
 *              Handles translation between logical grid positions and physical
 *              screen coordinates for rendering.
 */

#include "ui_helpers.hpp"

namespace UIHelpers {
    
    // Convert column index to letter string
    std::string getColumnLetter(int index) {
        if (index >= 0 && index < 26) {
            return std::string(1, 'A' + index);
        }
        return "?";
    }

    // Convert grid position to screen coordinates
    void gridToScreen(int boardSize, int gridX, int gridY, int boardStartX, int boardStartY, int &screenX, int &screenY) {
        (void)boardSize;  // Unused parameter
        screenY = boardStartY + 2 + gridY;  // +2 for border and header
        screenX = boardStartX + 8 + (gridX * 4);  // +8 for row labels, 4 chars per cell
    }

    // Convert screen coordinates to grid position
    void screenToGrid(int boardSize, int screenX, int screenY, int boardStartX, int boardStartY, int &gridX, int &gridY) {
        (void)boardSize;  // Unused parameter
        gridY = screenY - boardStartY - 2;
        gridX = (screenX - boardStartX - 8) / 4;
    }
}
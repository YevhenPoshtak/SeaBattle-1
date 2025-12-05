/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ui_helpers.hpp
 * Description: Helper functions for UI coordinate conversions. Provides utilities
 *              to convert between grid coordinates and screen positions.
 */

#ifndef UI_HELPERS_HPP
#define UI_HELPERS_HPP

#include <string>

namespace UIHelpers {
    
    // Convert column index to letter string (0='A', 1='B', etc.)
    std::string getColumnLetter(int index);
    
    // Convert grid coordinates to screen coordinates
    void gridToScreen(int boardSize, int gridX, int gridY, int boardStartX, int boardStartY, int &screenX, int &screenY);
    
    // Convert screen coordinates to grid coordinates
    void screenToGrid(int boardSize, int screenX, int screenY, int boardStartX, int boardStartY, int &gridX, int &gridY);
}

#endif
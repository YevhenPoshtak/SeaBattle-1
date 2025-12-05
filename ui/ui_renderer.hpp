/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * * File: ui_renderer.hpp
 * Description: Header file for the main UI rendering class. Declares static methods
 * for drawing game boards, menus, animations, and handling visual 
 * feedback using the ncurses library.
 */

#ifndef UI_RENDERER_HPP
#define UI_RENDERER_HPP

#include "../data/game_state.hpp"
#include "../data/board_data.hpp"
#include "ui_config.hpp"
#include <string>
#include <vector>

/**
 * @brief Main class for handling all visual output using ncurses.
 * Contains static methods to draw UI elements, manage menus, and render game state.
 */
class UIRenderer {
public:
    /**
     * @brief Initializes ncurses environment, colors, and input modes.
     */
    static void setupWindow();

    /**
     * @brief Draws the main ASCII title art and menu borders.
     */
    static void drawTitle();

    /**
     * @brief Renders the dual-board layout (Player and Enemy grids).
     * @param layout Coordinate configuration structure.
     * @param boardSize Dimension of the board (e.g., 10).
     * @param leftTitle Title for the left board.
     * @param rightTitle Title for the right board.
     */
    static void drawGameBoards(const BoardLayout& layout, int boardSize, const char* leftTitle, const char* rightTitle);
    
    /**
     * @brief Renders a single cell character with appropriate color coding.
     * @param screenY Y coordinate on screen.
     * @param screenX X coordinate on screen.
     * @param cell Character to draw ('w', 's', 'x', 'o', etc.).
     * @param isPlayerBoard If true, reveals ships; otherwise hides them.
     */
    static void drawBoardCell(int screenY, int screenX, char cell, bool isPlayerBoard);

    /**
     * @brief Iterates through board data and draws the entire grid.
     */
    static void drawBoardState(const BoardLayout& layout, const BoardData& board, bool isPlayerBoard);
    
    /**
     * @brief Displays control instructions/keybindings on the screen.
     */
    static void drawInstructions(const BoardLayout& layout);

    /**
     * @brief Shows current statistics (remaining ships for both sides).
     */
    static void drawGameStats(int y, int x, int playerShips, int enemyShips);
    
    /**
     * @brief Draws the targeting brackets around the current cursor position.
     */
    static void drawShotIndicator(int y, int x, bool selected);

    /**
     * @brief Removes the targeting brackets from the screen.
     */
    static void clearShotIndicator(int y, int x);
    
    /**
     * @brief Displays text feedback for the last shot (Hit/Miss/Sunk).
     */
    static void drawVolleyResult(int startY, int startX, const std::string& coords, const std::string& stats, bool isPlayer);
    
    /**
     * @brief Displays a temporary status message at specific coordinates.
     */
    static void showMessage(int y, int x, const std::string& message, int colorPair);

    /**
     * @brief Clears a specific horizontal line on the terminal.
     */
    static void clearLine(int y);
    
    /**
     * @brief Moves the physical terminal cursor to the specified location.
     */
    static void drawCursor(int y, int x);
    
    /**
     * @brief Visualizes ship positioning during manual setup.
     * Colors the ship green if valid, red if invalid.
     * @param isValid Output parameter indicating if placement is legal.
     */
    static void highlightShipPlacement(const BoardLayout& layout, int cursorX, int cursorY, int shipLength, int orientation, char symbol, const BoardData& board, bool& isValid);
    
    /**
     * @brief Renders the board specifically for the Auto-Generation phase.
     */
    static void drawGeneratedBoard(const BoardLayout& layout, const BoardData& board);

    /**
     * @brief Renders the board specifically for the Manual Placement phase.
     */
    static void drawManualBoard(const BoardLayout& layout, const BoardData& board);
    
    /**
     * @brief Displays the menu to select board size (10-20).
     * @return Selected size integer.
     */
    static int selectBoardSize();

    /**
     * @brief Displays the menu to configure shots per turn (Salvo mode).
     * @return Number of shots selected.
     */
    static int selectShotsPerTurn(int boardSize);
    
    /**
     * @brief Checks if terminal window is large enough for the board.
     * Displays a warning if too small.
     */
    static bool showTerminalSizeWarning(int boardSize);
    
    /**
     * @brief Prompts user to confirm their board setup (Y/N).
     */
    static bool confirmBoardPlacement();
    
    /**
     * @brief Renders the main game menu with navigation.
     * @param selectedOption Reference to remember last cursor position.
     * @return Index of the selected menu item.
     */
    static int showMainMenu(int& selectedOption);
    
    /**
     * @brief Restores terminal settings and ends ncurses mode.
     */
    static void cleanup();
};

#endif
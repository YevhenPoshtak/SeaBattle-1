/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: game_controller.cpp
 * Description: Implementation of game controller functions for board setup.
 *              Handles automatic board generation and user interaction for
 *              accepting, regenerating, or switching to manual placement.
 */

#include "game_controller.hpp"
#include "../ui/ui_renderer.hpp"
#include "../ui/ui_config.hpp"
#include "../logic/game_logic.hpp"
#include "../data/ship_data.hpp"
#include <vector>

// Setup player board with automatic generation and user options
// board: reference to the board to be initialized
// size: dimensions of the board (NxN)
// Returns: 1 = accepted, 0 = switch to manual, -1 = regenerate
int setupPlayerBoard(BoardData& board, int size) {
    // Initialize board and set as host
    board.initialize(size);
    board.setIsHost(true);
    
    // Calculate board layout for UI rendering
    BoardLayout layout = calculateBoardLayout(size);
    
    // Initialize game pieces and generate random placement
    std::vector<GamePiece> pieces;
    GameLogic::initializeGamePieces(board, pieces);
    GameLogic::generateBoardPlacement(board, pieces);
    board.buildShipCellMap();
    
    // Draw the generated board for user review
    UIRenderer::drawGeneratedBoard(layout, board);
    
    // Wait for user decision
    bool deciding = true;
    while (deciding) {
        int ch = getch();
        switch (ch) {
            case 'y':
            case 'Y':
                // Accept current board placement
                return 1;
            case 'n':
            case 'N':
                // Regenerate new random placement
                board.clear();
                GameLogic::generateBoardPlacement(board, pieces);
                board.buildShipCellMap();
                UIRenderer::drawGeneratedBoard(layout, board);
                break;
            case 'm':
            case 'M':
                // Switch to manual placement mode
                board.clear();
                return 0; 
        }
    }
    return 1;
}
/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: game_logic.hpp
 * Description: Header file for core game logic. Defines the GameLogic class
 *              with static methods for game initialization, ship placement,
 *              shot processing, and board validation.
 */

#ifndef GAME_LOGIC_HPP
#define GAME_LOGIC_HPP

#include "../data/game_state.hpp"
#include "../data/board_data.hpp"
#include "../data/ship_data.hpp"
#include <vector>
#include <string>

class GameLogic {
public:
    // Initialize game state with settings
    static void initializeGame(GameState& state, int boardSize, int shotsPerTurn, bool isHost);
    
    // Board generation methods
    static bool generateRandomBoard(BoardData& board, bool isHost);
    static bool generateManualBoard(BoardData& board);
    
    // Ship initialization and placement
    static void initializeGamePieces(BoardData& board, std::vector<GamePiece>& pieces);
    static void generateBoardPlacement(BoardData& board, const std::vector<GamePiece>& pieces);
    
    // Placement validation
    static short checkStartingPeg(const BoardData& board, int orientation, int starting_peg, int piece_length);
    
    // Manual ship placement
    static bool placeShip(BoardData& board, int gridX, int gridY, int orientation, int length, char symbol);
    
    // Validate ship placement
    static bool isValidShipPlacement(const BoardData& board, int gridX, int gridY, int orientation, int length);
    
    // Shot processing
    static int processShot(BoardData& targetBoard, int x, int y);
    
    // Update sunk ships on board
    static void updateSunkShips(BoardData& board, int x, int y);
    
    // Generate unique ship symbol
    static std::string generateShipSymbol(int shipId);
    
    // Helper for counting ships (recursive marking)
    static void markShipParts(int r, int c, int size, const std::vector<std::vector<char>>& board, std::vector<std::vector<bool>>& visited);
    
    // Count remaining intact ships
    static int countRemainingShips(const std::vector<std::vector<char>>& boardArray, int size);
};

#endif
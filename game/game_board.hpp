#ifndef GAME_BOARD_HPP
#define GAME_BOARD_HPP

#include "game_piece.hpp"
#include "../ui/config.hpp"
#include "ship_config.hpp"   
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>  

#ifdef _WIN32
    #include <pdcurses.h> 
#else
    #include <ncurses.h> 
#endif

struct ActiveShip {
    int id;             
    int startRow;       
    int startCol;       
    int length;         
    int orientation;    
    int hitCount;       
    bool isSunk;        
    char symbol;        
};

class Gameboard {
    bool isHost;
    std::vector<Game_Piece> piece_array; 
    int boardSize;
    std::map<char, ActiveShip> shipStatus;
    int missCount;  

public:
    std::vector<std::vector<char>> boardArray;
    std::vector<ActiveShip> myShips;
    int receiveShot(int x, int y);
    int getWoundedCount();
    int getSunkCount();
    int getMissCount();

    Gameboard();
    Gameboard(int size);

    int generateRandomBoard(bool host);
    int generateRandomBoardAuto(bool host);
    int generateManualBoard();
    void printBoard();
    
    int getBoardSize() const { return boardSize; }
    void setBoardSize(int size);
    void resizeBoard(int newSize);
    
    bool isShipSunk(char shipSymbol);
    void markShipAsHit(char shipSymbol);
    int getRemainingShips();

    std::vector<std::pair<int, int>> getShipCoordinates(char shipSymbol);
    void buildShipCellMap();
    std::vector<std::pair<int, int>> getShipOccupiedCells(int x, int y);

private:
    void initializeBoard();
    void initializeGamePieces();
    void generateBoardPlacement();
    short checkStartingPeg(int orientation, int starting_peg, int piece_length);
    void addPieceToBoardArray(int orientation, int starting_peg, int piece_length, char piece_symbol);
    void setIsHost(bool host);
    void displayEmptyBoard();
    void displayGeneratedBoard();
    bool highlightShip(int cursor_x, int cursor_y, int ship_length, int orientation, char ship_symbol);
    void placeGamePiece(int cursor_x, int cursor_y, int orientation, int ship_length, char ship_symbol);
    bool acceptGameboard();
    std::map<std::pair<int,int>, char> shipCellMap;
};

#endif
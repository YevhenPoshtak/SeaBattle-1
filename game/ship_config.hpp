#ifndef SHIP_CONFIG_HPP
#define SHIP_CONFIG_HPP

#include <vector>
#include <map>

struct ShipConfiguration {
    int boardSize;
    int fourDeck;   
    int threeDeck;  
    int twoDeck;    
    int oneDeck;     
    int shotsPerTurn; 
};

inline ShipConfiguration getShipConfig(int boardSize) {
    static const ShipConfiguration configs[] = {
        {10, 1, 2, 3, 4, 5},
        {11, 1, 2, 4, 5, 5},
        {12, 1, 3, 4, 6, 5},
        {13, 1, 3, 5, 6, 5},
        {14, 2, 3, 5, 7, 6},
        {15, 2, 4, 6, 8, 6},
        {16, 2, 4, 6, 9, 6},
        {17, 2, 4, 7, 9, 6},
        {18, 2, 5, 7, 10, 7},
        {19, 3, 5, 8, 11, 7},
        {20, 3, 5, 8, 12, 7},
        {21, 3, 6, 9, 13, 7},
        {22, 3, 6, 9, 14, 7},
        {23, 4, 6, 10, 15, 8},
        {24, 4, 7, 10, 16, 8},
        {25, 4, 7, 11, 17, 8},
        {26, 4, 7, 11, 18, 9}
    };
    
    for (const auto& config : configs) {
        if (config.boardSize == boardSize) {
            return config;
        }
    }
    
    return {10, 1, 2, 3, 4, 5};
}

inline int getTotalShips(int boardSize) {
    auto config = getShipConfig(boardSize);
    return config.fourDeck + config.threeDeck + config.twoDeck + config.oneDeck;
}

inline int getTotalShipCells(int boardSize) {
    auto config = getShipConfig(boardSize);
    return config.fourDeck * 4 + config.threeDeck * 3 + 
           config.twoDeck * 2 + config.oneDeck * 1;
}

struct Ship {
    char symbol;        
    int length;         
    int hitsRemaining;  
    bool isSunk;        
    
    Ship() : symbol('\0'), length(0), hitsRemaining(0), isSunk(false) {}
    Ship(char s, int l) : symbol(s), length(l), hitsRemaining(l), isSunk(false) {}
};

#endif
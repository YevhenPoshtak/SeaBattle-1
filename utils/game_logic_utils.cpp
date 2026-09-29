#include "game_logic_utils.hpp"

void markShipParts(int r, int c, int size, 
                   const std::vector<std::vector<char>>& board, 
                   std::vector<std::vector<bool>>& visited) {
    if (r < 0 || r >= size || c < 0 || c >= size) return;
    
    char cell = board[r][c];
    bool isShip = (cell != 'w' && cell != 'o' && cell != 'x' && cell != ' ');
    
    if (visited[r][c] || !isShip) return;

    visited[r][c] = true;

    markShipParts(r + 1, c, size, board, visited);
    markShipParts(r - 1, c, size, board, visited);
    markShipParts(r, c + 1, size, board, visited);
    markShipParts(r, c - 1, size, board, visited);
}

int countRemainingShips(const std::vector<std::vector<char>>& boardArray, int size) {
    int count = 0;
    std::vector<std::vector<bool>> visited(size, std::vector<bool>(size, false));
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            char cell = boardArray[i][j];
            bool isShip = (cell != 'w' && cell != 'o' && cell != 'x' && cell != ' ');
            
            if (isShip && !visited[i][j]) {
                count++; 
                markShipParts(i, j, size, boardArray, visited);
            }
        }
    }   
    return count;
}
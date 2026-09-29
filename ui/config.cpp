#include "config.hpp"
#include <algorithm>

int BOARD_SIZE = 10;

void setBoardSize(int size) {
    BOARD_SIZE = std::max(MIN_BOARD_SIZE, std::min(MAX_BOARD_SIZE, size));
}

int getBoardSize() {
    return BOARD_SIZE;
}

bool canFitInterface(int boardSize, int maxY, int maxX) {
    int minY = boardSize + 20;
    int minX = 2 * (8 + boardSize * 4) + 5;
    return (maxY >= minY && maxX >= minX);
}

void getRequiredTerminalSize(int boardSize, int &minY, int &minX) {
    minY = boardSize + 20;
    minX = 2 * (8 + boardSize * 4) + 5;
}
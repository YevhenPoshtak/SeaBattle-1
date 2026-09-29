#ifndef GAME_LOGIC_UTILS_HPP
#define GAME_LOGIC_UTILS_HPP

#include <vector>
#include <string>

int countRemainingShips(const std::vector<std::vector<char>>& boardArray, int size);

void markShipParts(int r, int c, int size, const std::vector<std::vector<char>>& board, std::vector<std::vector<bool>>& visited);

#endif 
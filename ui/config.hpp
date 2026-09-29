#ifndef CONFIG_HPP
#define CONFIG_HPP

extern int BOARD_SIZE;
const int MIN_BOARD_SIZE = 10;
const int MAX_BOARD_SIZE = 26;

void setBoardSize(int size);
int getBoardSize();
bool canFitInterface(int boardSize, int maxY, int maxX);
void getRequiredTerminalSize(int boardSize, int &minY, int &minX);

#endif
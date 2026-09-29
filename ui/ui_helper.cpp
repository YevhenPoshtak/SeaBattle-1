#include "ui_helper.hpp"
#include <sstream>
#include <cstring>

void UIHelper::getBoardDimensions(int boardSize, int &startX, int &startY, int &width, int &height) {
    width = 3 + boardSize * 4;
    height = boardSize + 3;
    
    startY = 6;
    
    int maxX = getmaxx(stdscr);
    int totalWidth = width * 2 + 5; 
    startX = (maxX - totalWidth) / 2;
}

void UIHelper::drawBoardFrame(int startX, int startY, int boardSize, const char* title) {
    int titleLen = strlen(title);
    int titleX = startX + (3 + boardSize * 4 - titleLen) / 2;
    mvprintw(startY - 1, titleX, title);
    
    move(startY, startX);
    printw("   ");
    for (int i = 0; i < 3 + boardSize * 4; i++) {
        addch('_');
    }
    
    move(startY + 1, startX);
    printw("   |  |");
    for (int i = 0; i < boardSize; i++) {
        printw(" %c |", getColumnLetter(i));
    }
    
    for (int i = 0; i < boardSize; i++) {
        move(startY + 2 + i, startX);
        if (i + 1 < 10) {
            printw("   | %d|", i + 1);
        } else {
            printw("   |%d|", i + 1);
        }
        
        for (int j = 0; j < boardSize; j++) {
            printw("   |");
        }
    }
    
    move(startY + 2 + boardSize, startX);
    printw("   ");
    for (int i = 0; i < 3 + boardSize * 4; i++) {
        addch('-');
    }
}

void UIHelper::drawBoardSeparator(int startX, int startY, int height) {
    for (int i = 0; i < height; i++) {
        mvprintw(startY + i, startX, "~~~~~");
    }
}

bool UIHelper::checkTerminalSize(int boardSize) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    return canFitInterface(boardSize, maxY, maxX);
}

void UIHelper::showTerminalSizeError(int boardSize) {
    clear();
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    int minY, minX;
    getRequiredTerminalSize(boardSize, minY, minX);
    
    attron(COLOR_PAIR(4));
    mvprintw(maxY/2 - 2, 2, "WARNING: Terminal size too small!");
    attroff(COLOR_PAIR(4));
    
    mvprintw(maxY/2, 2, "Current size: %dx%d", maxX, maxY);
    mvprintw(maxY/2 + 1, 2, "Required size: %dx%d", minX, minY);
    mvprintw(maxY/2 + 3, 2, "Please increase terminal window size");
    mvprintw(maxY/2 + 4, 2, "or reduce board size (current: %dx%d)", boardSize, boardSize);
    mvprintw(maxY/2 + 6, 2, "Press any key to return...");
    refresh();
    getch();
}

void UIHelper::gridToScreen(int boardSize, int gridX, int gridY, int boardStartX, int boardStartY,
                           int &screenX, int &screenY) {
    screenY = boardStartY + 2 + gridY; 
    screenX = boardStartX + 8 + (gridX * 4); 
}

std::string UIHelper::getColumnHeader(int boardSize) {
    std::stringstream ss;
    ss << "|  |";
    for (int i = 0; i < boardSize; i++) {
        ss << " " << getColumnLetter(i) << " |";
    }
    return ss.str();
}
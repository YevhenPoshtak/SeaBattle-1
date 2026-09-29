#ifndef BOARD_LAYOUT_HPP
#define BOARD_LAYOUT_HPP

#ifdef _WIN32
    #include <curses.h>
#else
    #include <ncurses.h>
#endif

struct BoardLayout {
    int startY;
    int board1StartX;
    int board2StartX;
    int separatorX;
    int instructionsY;
    int logStartX;
    int statusY;
};

inline BoardLayout calculateBoardLayout(int boardSize) {
    BoardLayout layout;
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    int boardWidth = boardSize * 4 + 8;
    int separatorWidth = 5;
    int totalWidth = boardWidth * 2 + separatorWidth;
    
    layout.board1StartX = (maxX - totalWidth) / 2;
    if (layout.board1StartX < 3) layout.board1StartX = 3;
    
    layout.separatorX = layout.board1StartX + boardWidth;
    layout.board2StartX = layout.separatorX + separatorWidth;
    
    int boardHeight = boardSize + 6;
    layout.startY = (maxY - boardHeight - 12) / 2 + 4;
    if (layout.startY < 10) layout.startY = 10;
    
    layout.instructionsY = layout.startY + boardHeight + 2;
    layout.statusY = layout.instructionsY + 3;
    layout.logStartX = layout.board2StartX + boardWidth + 3;
    
    if (layout.logStartX + 50 > maxX) {
        layout.logStartX = maxX - 50;
    }
    
    return layout;
}

#endif  
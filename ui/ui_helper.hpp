#ifndef UI_HELPER_HPP
#define UI_HELPER_HPP

#include "config.hpp"
#include <string>

#ifdef _WIN32
    #include <pdcurses.h>
#else
    #include <ncurses.h>
#endif

class UIHelper {
public:

    static void getBoardDimensions(int boardSize, int &startX, int &startY, int &width, int &height);
    
    static void drawBoardFrame(int startX, int startY, int boardSize, const char* title);
    
    static void drawBoardSeparator(int startX, int startY, int height);

    static bool checkTerminalSize(int boardSize);

    static void showTerminalSizeError(int boardSize);

    static void gridToScreen(int boardSize, int gridX, int gridY, int boardStartX, int boardStartY,
                            int &screenX, int &screenY);

    static std::string getColumnHeader(int boardSize);
    
    static inline char getColumnLetter(int index) {
        return (index >= 0 && index < 26) ? ('A' + index) : '?';
    }
};

#endif 
#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#ifdef _WIN32
    #include <pdcurses.h>
#else
    #include <ncurses.h>
#endif

void drawFirework(bool playerWon);

void drawMenuAnimation(int frame);

void drawBottomShipAnimation(int frame, int startY, int maxX);

#endif 
#include "animation.hpp"
#include <cmath>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(x) Sleep(x)
#else
    #include <unistd.h>
    #define SLEEP_MS(x) usleep((x) * 1000)
#endif

void drawBottomShipAnimation(int frame, int startY, int maxX) {
    for (int clearY = startY - 4; clearY <= startY + 6; clearY++) {
        move(clearY, 0);
        clrtoeol();
    }
    
    int cycleFrame = frame % 80;
    
    int yellowShipX = 15 + (cycleFrame / 2);
    int blueShipX = maxX - 25 - (cycleFrame / 2);
    
    attron(COLOR_PAIR(3));
    for (int i = 0; i < maxX; i += 2) {
        int waveY = startY + 4 + (int)(sin((i + frame * 0.5) * 0.2) * 1.5);
        if (waveY >= 0) {
            mvaddch(waveY, i, '~');
            if (i + 1 < maxX) {
                mvaddch(waveY, i + 1, '~');
            }
        }
    }
    attroff(COLOR_PAIR(3));
    
    int phase = (cycleFrame / 20);
    
    if (phase == 0 || phase == 1) {
        attron(COLOR_PAIR(5));
        mvprintw(startY - 2, yellowShipX, "    _~_");
        mvprintw(startY - 1, yellowShipX, "   /___\\");
        mvprintw(startY,     yellowShipX, "  |=====|>");
        mvprintw(startY + 1, yellowShipX, " /~~~~~~~\\");
        mvprintw(startY + 2, yellowShipX, "~~~~~~~~~~~");
        attroff(COLOR_PAIR(5));
        
        attron(COLOR_PAIR(6));
        mvprintw(startY - 2, blueShipX, " _~_");
        mvprintw(startY - 1, blueShipX, "/___\\");
        mvprintw(startY,     blueShipX, "<|=====|");
        mvprintw(startY + 1, blueShipX, "/~~~~~~~\\");
        mvprintw(startY + 2, blueShipX, "~~~~~~~~~~~");
        attroff(COLOR_PAIR(6));
        
    } else if (phase == 2) {
        attron(COLOR_PAIR(5));
        mvprintw(startY - 2, yellowShipX, "    _~_");
        mvprintw(startY - 1, yellowShipX, "   /___\\");
        mvprintw(startY,     yellowShipX, "  |=====|>");
        mvprintw(startY + 1, yellowShipX, " /~~~~~~~\\");
        mvprintw(startY + 2, yellowShipX, "~~~~~~~~~~~");
        attroff(COLOR_PAIR(5));
        
        int projectileProgress = cycleFrame % 20;
        int projectileX = yellowShipX + 11 + (projectileProgress * (blueShipX - yellowShipX - 15) / 20);
        if (projectileX < blueShipX - 2) {
            attron(COLOR_PAIR(2));
            mvprintw(startY, projectileX, "===>");
            attroff(COLOR_PAIR(2));
        }
        
        attron(COLOR_PAIR(6));
        mvprintw(startY - 2, blueShipX, " _~_");
        mvprintw(startY - 1, blueShipX, "/___\\");
        mvprintw(startY,     blueShipX, "<|=====|");
        mvprintw(startY + 1, blueShipX, "/~~~~~~~\\");
        mvprintw(startY + 2, blueShipX, "~~~~~~~~~~~");
        attroff(COLOR_PAIR(6));
        
    } else if (phase == 3) {
        attron(COLOR_PAIR(5));
        mvprintw(startY - 2, yellowShipX, "    _~_");
        mvprintw(startY - 1, yellowShipX, "   /___\\");
        mvprintw(startY,     yellowShipX, "  |=====|>");
        mvprintw(startY + 1, yellowShipX, " /~~~~~~~\\");
        mvprintw(startY + 2, yellowShipX, "~~~~~~~~~~~");
        attroff(COLOR_PAIR(5));
        
        int explosionFrame = cycleFrame % 20;
        if (explosionFrame < 10) {
            attron(COLOR_PAIR(4));
            mvprintw(startY - 3, blueShipX - 2, "  * * *");
            mvprintw(startY - 2, blueShipX - 2, " * * * *");
            mvprintw(startY - 1, blueShipX - 2, "* BOOM *");
            mvprintw(startY,     blueShipX - 2, "* * * * *");
            mvprintw(startY + 1, blueShipX - 2, " * * * *");
            mvprintw(startY + 2, blueShipX - 2, "  * * *");
            attroff(COLOR_PAIR(4));
        } else {
            attron(COLOR_PAIR(1));
            mvprintw(startY - 1, blueShipX, " . . .");
            mvprintw(startY,     blueShipX, ". . . .");
            mvprintw(startY + 1, blueShipX, " . . .");
            attroff(COLOR_PAIR(1));
        }
    }
}

void drawFirework(bool playerWon) {
    clear();
    
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    nodelay(stdscr, TRUE);
    
    int frame = 0;
    bool running = true;
    
    int shipY = maxY - 6;
    (void)shipY;
    int numShips = 5;
    int shipPositions[5];
    int shipColors[5] = {2, 3, 4, 5, 6};
    
    int totalShipWidth = numShips * 8;
    int spacing = (maxX - totalShipWidth) / (numShips + 1);
    for (int s = 0; s < numShips; s++) {
        shipPositions[s] = spacing + s * (spacing + 8);
    }
    
    int sunkenShipPositions[3] = {15, maxX/2 - 5, maxX - 30};
    
    while (running) {
        clear();
        
        attron(COLOR_PAIR(1));
        mvprintw(0, (maxX - 50) / 2, "  ___            ___          _    _    _        ");
        mvprintw(1, (maxX - 50) / 2, " / __) ___  __ _| _ ) __ _ __| |_ | |_ | | ___   ");
        mvprintw(2, (maxX - 50) / 2, " \\__ \\/ -_)/ _` | _ \\/ _` (_-<  _||  _|| |/ -_)  ");
        mvprintw(3, (maxX - 50) / 2, " |___/\\___| \\__,_|___/\\__,_/__/\\__| \\__||_|\\___|  ");
        attroff(COLOR_PAIR(2));
        
        if (playerWon) {
        
            int seabedY = maxY - 2;
            int animationHeight = 20;
            int animationTop = seabedY - animationHeight;
            int sunkenShipY = seabedY - 4;
            int waveY = animationTop + 2;
            
            int winTextY = 7;
            attron(COLOR_PAIR(2) | A_BOLD);
            const char* msg = "YOU WON!";
            mvprintw(winTextY, (maxX - strlen(msg)) / 2, msg);
            attroff(A_BOLD);
            
            attron(COLOR_PAIR(1));
            const char* pressKey = "Press any key to return to menu...";
            mvprintw(winTextY + 2, (maxX - strlen(pressKey)) / 2, pressKey);
            
            attron(COLOR_PAIR(3));
            for (int i = 0; i < maxX; i++) {
                int wave = waveY + (int)(sin((i + frame * 0.3) * 0.15) * 1.2);
                if (wave >= animationTop && wave < seabedY) {
                    mvaddch(wave, i, '~');
                }
            }
            attroff(COLOR_PAIR(3));
            
            for (int s = 0; s < 3; s++) {
                int shipX = sunkenShipPositions[s];
                int tilt = (s == 1) ? 0 : (s == 0 ? 1 : -1);
                
                attron(COLOR_PAIR(1));
                
                if (tilt == 0) {
                    mvprintw(seabedY - 2, shipX, " /___\\");
                    mvprintw(seabedY - 1, shipX, "|xxxxx|");
                } else if (tilt == 1) {
                    mvprintw(seabedY - 2, shipX, "  /___\\_");
                    mvprintw(seabedY - 1, shipX, " |xxxxx|\\");
                } else {
                    mvprintw(seabedY - 2, shipX, "/___ \\");
                    mvprintw(seabedY - 1, shipX, "/|xxxxx|");
                }
                attroff(COLOR_PAIR(1));
            }
            
            attron(COLOR_PAIR(2));
            for (int i = 0; i < maxX; i++) {
                if (i % 3 == 0) mvaddch(seabedY, i, '^');
                else if (i % 2 == 0) mvaddch(seabedY, i, '=');
                else mvaddch(seabedY, i, '_');
            }
            attroff(COLOR_PAIR(2));
            
            for (int s = 0; s < numShips; s++) {
                int shipX = shipPositions[s];
                int bobbing = (int)(sin((frame + s * 20) * 0.1) * 0.5);
                int shipColor = shipColors[s];
                int raisedY = seabedY - 5;
                attron(COLOR_PAIR(shipColor));
                mvprintw(raisedY - 2 + bobbing, shipX, "  _~_");
                mvprintw(raisedY - 1 + bobbing, shipX, " /___\\");
                mvprintw(raisedY + bobbing,     shipX, "|=====|");
                mvprintw(raisedY + 1 + bobbing, shipX, "/~~~~~\\");
                attroff(COLOR_PAIR(shipColor));
            }
            
            for (int i = 0; i < numShips; i++) {
                int launchX = shipPositions[i] + 3;
                int fireDelay = i * 20;
                int localFrame = (frame - fireDelay + 80) % 80;
                
                if (localFrame < 60) {
                    int color = shipColors[i];
                    int raisedY = seabedY - 5;
                    
                    if (localFrame < 25) {
                        int rocketY = raisedY - 2 - localFrame;
                        if (rocketY >= 5 && rocketY < raisedY) {
                            attron(COLOR_PAIR(color));
                            mvaddch(rocketY, launchX, '|');
                            if (rocketY + 1 < raisedY) {
                                mvaddch(rocketY + 1, launchX, '.');
                            }
                            attroff(COLOR_PAIR(color));
                        }
                    } else if (localFrame < 60) {
                        int explosionFrame = localFrame - 25;
                        int explosionY = raisedY - 2 - 25;
                        
                        attron(COLOR_PAIR(color));
                        
                        if (explosionFrame < 8) {
                            mvaddch(explosionY, launchX, '*');
                            mvaddch(explosionY - 1, launchX, '*');
                            mvaddch(explosionY, launchX - 1, '*');
                            mvaddch(explosionY, launchX + 1, '*');
                            mvaddch(explosionY + 1, launchX, '*');
                        } else if (explosionFrame < 18) {
                            mvaddch(explosionY - 2, launchX, '*');
                            mvaddch(explosionY - 1, launchX - 1, '*');
                            mvaddch(explosionY - 1, launchX, '*');
                            mvaddch(explosionY - 1, launchX + 1, '*');
                            mvaddch(explosionY, launchX - 2, '*');
                            mvaddch(explosionY, launchX - 1, '*');
                            mvaddch(explosionY, launchX, '*');
                            mvaddch(explosionY, launchX + 1, '*');
                            mvaddch(explosionY, launchX + 2, '*');
                            mvaddch(explosionY + 1, launchX - 1, '*');
                            mvaddch(explosionY + 1, launchX, '*');
                            mvaddch(explosionY + 1, launchX + 1, '*');
                            mvaddch(explosionY + 2, launchX, '*');
                        } else if (explosionFrame < 28) {
                            mvaddch(explosionY - 3, launchX, '.');
                            mvaddch(explosionY - 2, launchX - 2, '.');
                            mvaddch(explosionY - 2, launchX, '*');
                            mvaddch(explosionY - 2, launchX + 2, '.');
                            mvaddch(explosionY - 1, launchX - 3, '.');
                            mvaddch(explosionY - 1, launchX - 1, '*');
                            mvaddch(explosionY - 1, launchX + 1, '*');
                            mvaddch(explosionY - 1, launchX + 3, '.');
                            mvaddch(explosionY, launchX - 3, '.');
                            mvaddch(explosionY, launchX - 1, '*');
                            mvaddch(explosionY, launchX, '*');
                            mvaddch(explosionY, launchX + 1, '*');
                            mvaddch(explosionY, launchX + 3, '.');
                            mvaddch(explosionY + 1, launchX - 3, '.');
                            mvaddch(explosionY + 1, launchX - 1, '*');
                            mvaddch(explosionY + 1, launchX + 1, '*');
                            mvaddch(explosionY + 1, launchX + 3, '.');
                            mvaddch(explosionY + 2, launchX - 2, '.');
                            mvaddch(explosionY + 2, launchX, '*');
                            mvaddch(explosionY + 2, launchX + 2, '.');
                            mvaddch(explosionY + 3, launchX, '.');
                        } else {
                            mvaddch(explosionY - 3, launchX - 1, '.');
                            mvaddch(explosionY - 3, launchX + 1, '.');
                            mvaddch(explosionY - 2, launchX - 3, '.');
                            mvaddch(explosionY - 2, launchX + 3, '.');
                            mvaddch(explosionY, launchX - 4, '.');
                            mvaddch(explosionY, launchX + 4, '.');
                            mvaddch(explosionY + 2, launchX - 3, '.');
                            mvaddch(explosionY + 2, launchX + 3, '.');
                            mvaddch(explosionY + 3, launchX - 1, '.');
                            mvaddch(explosionY + 3, launchX + 1, '.');
                        }
                        
                        attroff(COLOR_PAIR(color));
                    }
                }
            }
            
            for (int s = 0; s < 3; s++) {
                int bubbleX = sunkenShipPositions[s] + 4;
                for (int b = 0; b < 3; b++) {
                    int bubbleY = sunkenShipY - 2 - ((frame + b * 6 + s * 10) % 12);
                    int bubbleOffset = (int)(sin((frame + b) * 0.3) * 2);
                    if (bubbleY > waveY && bubbleY < sunkenShipY - 1) {
                        attron(COLOR_PAIR(3));
                        char bubble = (b % 3 == 0) ? 'o' : (b % 3 == 1) ? 'O' : '.';
                        mvaddch(bubbleY, bubbleX + bubbleOffset, bubble);
                        attroff(COLOR_PAIR(3));
                    }
                }
            }
            
            for (int fish = 0; fish < 4; fish++) {
                int fishX = ((frame * (2 + fish % 3)) / 3 + fish * 20) % (maxX + 10);
                int fishY = waveY + 3 + (fish % 2) * 3;
                int swimPattern = (int)(sin((frame + fish * 20) * 0.1) * 1);
                
                attron(COLOR_PAIR(2));
                if (fishX > 0 && fishX < maxX - 5 && fishY > waveY && fishY < sunkenShipY - 2) {
                    if (fish % 2 == 0) {
                        mvprintw(fishY + swimPattern, fishX, "><>");
                    } else {
                        mvprintw(fishY + swimPattern, maxX - fishX - 3, "<><");
                    }
                }
                attroff(COLOR_PAIR(2));
            }
            
            for (int j = 0; j < 2; j++) {
                int jellyX = 25 + j * 40 + (int)(sin((frame + j * 40) * 0.08) * 6);
                int jellyY = waveY + 5 + j * 4 + (int)(sin((frame + j * 30) * 0.06) * 1);
                
                attron(COLOR_PAIR(6));
                if (jellyX > 0 && jellyX < maxX - 6 && jellyY > waveY + 1 && jellyY < sunkenShipY - 2) {
                    mvprintw(jellyY, jellyX, " _-_");
                    mvprintw(jellyY + 1, jellyX, "(o.o)");
                }
                attroff(COLOR_PAIR(6));
            }
            
            attron(COLOR_PAIR(2));
            for (int w = 0; w < maxX; w += 12) {
                int seaweedHeight = 2 + (w % 2);
                for (int h = 0; h < seaweedHeight; h++) {
                    int swayX = w + (int)(sin((frame * 0.08 + h + w) * 0.4) * 1.5);
                    int swayY = seabedY - 1 - h;
                    if (swayX >= 0 && swayX < maxX && swayY > waveY + 2 && swayY < seabedY) {
                        mvaddch(swayY, swayX, '|');
                    }
                }
            }
            attroff(COLOR_PAIR(2));
            
            int crabX = (frame / 2) % (maxX - 10);
            attron(COLOR_PAIR(4));
            mvprintw(seabedY - 1, crabX, "(V)o.o(V)");
            attroff(COLOR_PAIR(4));
            
            attron(COLOR_PAIR(5));
            for (int s = 0; s < 3; s++) {
                int treasureX = sunkenShipPositions[s] + 9;
                if (s == 1) {
                    mvprintw(seabedY - 1, treasureX, "[$]");
                } else {
                    mvprintw(seabedY - 1, treasureX, "[#]");
                }
            }
            attroff(COLOR_PAIR(5));
            
        } else {
            
            int seabedY = maxY - 2;  
            int animationHeight = 20;  
            int animationTop = seabedY - animationHeight;  
            
            int sunkenShipY = seabedY - 4;  
            int waveY = animationTop + 2;  
            
            int winTextY = 7;
            attron(COLOR_PAIR(4) | A_BOLD);
            const char* msg = "OPPONENT WON!";
            mvprintw(winTextY, (maxX - strlen(msg)) / 2, msg);
            attroff(A_BOLD);
            
            attron(COLOR_PAIR(1));
            const char* pressKey = "Press any key to return to menu...";
            mvprintw(winTextY + 2, (maxX - strlen(pressKey)) / 2, pressKey);
            
            attron(COLOR_PAIR(3));
            for (int i = 0; i < maxX; i++) {
                int wave = waveY + (int)(sin((i + frame * 0.3) * 0.15) * 1.2);
                if (wave >= animationTop && wave < seabedY) {
                    mvaddch(wave, i, '~');
                }
            }
            attroff(COLOR_PAIR(3));
            
            for (int s = 0; s < 3; s++) {
                int shipX = sunkenShipPositions[s];
                int tilt = (s == 1) ? 0 : (s == 0 ? 1 : -1);
                
                attron(COLOR_PAIR(1));
                
                if (tilt == 0) {
                    mvprintw(seabedY - 2, shipX, " /___\\");
                    mvprintw(seabedY - 1, shipX, "|xxxxx|");
                } else if (tilt == 1) {
                    mvprintw(seabedY - 2, shipX, "  /___\\_");
                    mvprintw(seabedY - 1, shipX, " |xxxxx|\\");
                } else {
                    mvprintw(seabedY - 2, shipX, "/___ \\");
                    mvprintw(seabedY - 1, shipX, "/|xxxxx|");
                }
                attroff(COLOR_PAIR(1));
            }
            
            attron(COLOR_PAIR(2));
            for (int i = 0; i < maxX; i++) {
                if (i % 3 == 0) mvaddch(seabedY, i, '^');
                else if (i % 2 == 0) mvaddch(seabedY, i, '=');
                else mvaddch(seabedY, i, '_');
            }
            attroff(COLOR_PAIR(2));
            
            for (int s = 0; s < 3; s++) {
                int bubbleX = sunkenShipPositions[s] + 4;
                for (int b = 0; b < 3; b++) {
                    int bubbleY = sunkenShipY - 2 - ((frame + b * 6 + s * 10) % 12);
                    int bubbleOffset = (int)(sin((frame + b) * 0.3) * 2);
                    if (bubbleY > waveY && bubbleY < sunkenShipY - 1) {
                        attron(COLOR_PAIR(3));
                        char bubble = (b % 3 == 0) ? 'o' : (b % 3 == 1) ? 'O' : '.';
                        mvaddch(bubbleY, bubbleX + bubbleOffset, bubble);
                        attroff(COLOR_PAIR(3));
                    }
                }
            }
            
            for (int fish = 0; fish < 4; fish++) {
                int fishX = ((frame * (2 + fish % 3)) / 3 + fish * 20) % (maxX + 10);
                int fishY = waveY + 3 + (fish % 2) * 3;
                int swimPattern = (int)(sin((frame + fish * 20) * 0.1) * 1);
                
                attron(COLOR_PAIR(2));
                if (fishX > 0 && fishX < maxX - 5 && fishY > waveY && fishY < sunkenShipY - 2) {
                    if (fish % 2 == 0) {
                        mvprintw(fishY + swimPattern, fishX, "><>");
                    } else {
                        mvprintw(fishY + swimPattern, maxX - fishX - 3, "<><");
                    }
                }
                attroff(COLOR_PAIR(2));
            }
            
            for (int j = 0; j < 2; j++) {
                int jellyX = 25 + j * 40 + (int)(sin((frame + j * 40) * 0.08) * 6);
                int jellyY = waveY + 5 + j * 4 + (int)(sin((frame + j * 30) * 0.06) * 1);
                
                attron(COLOR_PAIR(6));
                if (jellyX > 0 && jellyX < maxX - 6 && jellyY > waveY + 1 && jellyY < sunkenShipY - 2) {
                    mvprintw(jellyY, jellyX, " _-_");
                    mvprintw(jellyY + 1, jellyX, "(o.o)");
                }
                attroff(COLOR_PAIR(6));
            }
            
            attron(COLOR_PAIR(2));
            for (int w = 0; w < maxX; w += 12) {
                int seaweedHeight = 2 + (w % 2);
                for (int h = 0; h < seaweedHeight; h++) {
                    int swayX = w + (int)(sin((frame * 0.08 + h + w) * 0.4) * 1.5);
                    int swayY = seabedY - 1 - h;
                    if (swayX >= 0 && swayX < maxX && swayY > waveY + 2 && swayY < seabedY) {
                        mvaddch(swayY, swayX, '|');
                    }
                }
            }
            attroff(COLOR_PAIR(2));
            
            int crabX = (frame / 2) % (maxX - 10);
            attron(COLOR_PAIR(4));
            mvprintw(seabedY - 1, crabX, "(V)o.o(V)");
            attroff(COLOR_PAIR(4));
            
            attron(COLOR_PAIR(5));
            for (int s = 0; s < 3; s++) {
                int treasureX = sunkenShipPositions[s] + 9;
                if (s == 1) {
                    mvprintw(seabedY - 1, treasureX, "[$]");
                } else {
                    mvprintw(seabedY - 1, treasureX, "[#]");
                }
            }
            attroff(COLOR_PAIR(5));
        }
        
        refresh();
        
        int ch = getch();
        if (ch != ERR) {
            running = false;
        }
        
        #ifdef _WIN32
            Sleep(80);
        #else
            usleep(80000);
        #endif
        
        frame++;
    }
    
    nodelay(stdscr, FALSE);
    clear();
    refresh();
}

void drawMenuAnimation(int frame) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    int animY = maxY - 8;
    if (animY < 10) return; 

    for (int clearY = animY - 13; clearY <= animY + 8; clearY++) {
        if (clearY >= 0 && clearY < maxY) {
            move(clearY, 0);
            clrtoeol();
        }
    }

    int cycleFrame = frame % 180;
    
    if (cycleFrame == 0) {
        for (int clearY = animY - 13; clearY <= animY + 8; clearY++) {
             if (clearY >= 0 && clearY < maxY) {
                move(clearY, 0);
                clrtoeol();
             }
        }
        refresh();
        return;
    }

    attron(COLOR_PAIR(3));
    for (int i = 0; i < maxX; i++) {
        int waveY = animY - 11 + (int)(sin((i + frame * 0.3) * 0.15) * 1.2);
        if (waveY >= 0 && waveY < maxY) {
            mvaddch(waveY, i, '~');
        }
    }
    attroff(COLOR_PAIR(3));

    attron(COLOR_PAIR(2));
    for (int i = 0; i < maxX; i++) {
        int floorY = animY + 7;
        if (floorY < maxY) {
            if (i % 5 == 0) mvaddch(floorY, i, '^');
            else if (i % 3 == 0) mvaddch(floorY, i, '_');
            else mvaddch(floorY, i, '=');
        }
    }
    attroff(COLOR_PAIR(2));

    int subX = 10 + (cycleFrame / 4);
    int subY = animY + 1;
    
    int minePositions[3] = {34, 59, 84};
    int hitMine = -1;
    
    if (cycleFrame > 10) {
        for (int m = 0; m < 3; m++) {
            int mineX = minePositions[m];
            int mineY = animY + 2;
            if (subX + 14 >= mineX && subX + 14 <= mineX + 3 && 
                subY >= mineY - 2 && subY + 2 <= mineY + 2) {
                hitMine = m;
                break;
            }
        }
    }
    int wasHit = -1;
    if (cycleFrame > 10 && hitMine < 0) {
        for (int m = 0; m < 3; m++) {
            int mineX = minePositions[m];
            int frameAtMine = (mineX - 14 - 10) * 4;
            if (cycleFrame > frameAtMine && frameAtMine > 10) {
                int checkX = 10 + (frameAtMine / 4);
                int mineY = animY + 2;
                if (checkX + 14 >= mineX && checkX + 14 <= mineX + 3 && 
                    subY >= mineY - 2 && subY + 2 <= mineY + 2) {
                    wasHit = m;
                    break;
                }
            }
        }
    }
    
    if (cycleFrame > 10) {
        for (int m = 0; m < 3; m++) {
            if (hitMine == m || wasHit == m) continue; 
            int mineX = minePositions[m];
            int mineY = animY + 2;
            
            if (mineY + 1 < maxY) {
                attron(COLOR_PAIR(4));
                mvprintw(mineY - 1, mineX, " |");
                mvprintw(mineY, mineX, "[@]");
                mvprintw(mineY + 1, mineX, "/*\\");
                attroff(COLOR_PAIR(4));
            }
        }
        if ((frame / 8) % 2 == 0 && hitMine < 0 && wasHit < 0) {
            attron(COLOR_PAIR(4));
            mvprintw(animY - 5, maxX / 2 - 15, "!!! DANGER: MINES DETECTED !!!");
            attroff(COLOR_PAIR(4));
        }
    }
    
    int explodingMine = (hitMine >= 0) ? hitMine : wasHit;
    
    if (explodingMine >= 0) {
        int explX = minePositions[explodingMine];
        int explY = animY + 2;
        int frameAtMine = (minePositions[explodingMine] - 14 - 10) * 4;
        
        int t = cycleFrame - frameAtMine; 
        
        int duration = 10;
        int stopT = (t > duration) ? duration : t;

        if (t < 3) {
            attron(COLOR_PAIR(5) | A_BOLD);
            if (explY - 3 >= 0) mvprintw(explY - 3, explX - 4, "    * * * ");
            if (explY - 2 >= 0) mvprintw(explY - 2, explX - 4, "  * * * * * ");
            if (explY - 1 >= 0) mvprintw(explY - 1, explX - 4, "* * BOOM! * *");
            if (explY < maxY)   mvprintw(explY, explX - 4,     " * * * * * ");
            if (explY + 1 < maxY) mvprintw(explY + 1, explX - 4, "    * * * ");
            attroff(COLOR_PAIR(5) | A_BOLD);
        }

        int shipBaseX = 10 + (frameAtMine / 4);
        int shipBaseY = subY;
        
        int floorLimit = animY + 6; 

        attron(COLOR_PAIR(1));

        int cabX = shipBaseX + 2 + (stopT / 2); 
        int cabY = shipBaseY - 1 + (stopT / 2); 
        
        if (cabY > floorLimit - 1) cabY = floorLimit - 1;
        
        if (cabY < maxY && cabX < maxX) {
            mvprintw(cabY, cabX, "__");
            mvprintw(cabY + 1, cabX, "/  |");
        }

        int tailX = shipBaseX - (stopT / 3); 
        int tailY = shipBaseY + 1 + (stopT / 2);
        
        if (tailY > floorLimit) tailY = floorLimit; 
        
        if (tailY < maxY && tailX > 0) {
             mvprintw(tailY, tailX, "|____");
             mvaddch(tailY, tailX - 1, '+'); 
        }

        int noseX = shipBaseX + 7 + stopT;
        int noseY = shipBaseY + (stopT / 2);
        
        if (noseY > floorLimit) noseY = floorLimit;

        if (noseY < maxY && noseX < maxX) {
            mvprintw(noseY, noseX, "\\___");
            mvprintw(noseY + 1, noseX, "_|>");
        }
        
        if (t >= duration) {
            int debrisY = floorLimit + 1;
            if (debrisY < maxY) {
                mvaddch(debrisY, shipBaseX + 5, 'o'); 
                mvaddch(debrisY, shipBaseX + 8, '#'); 
            }
        }

        attroff(COLOR_PAIR(1));

    } else {
        attron(COLOR_PAIR(5));
        if (subY + 2 < maxY) {
            mvprintw(subY - 2, subX, "   __");
            mvprintw(subY - 1, subX, "  /  |");
            mvprintw(subY, subX, " |o   \\___");
            mvprintw(subY + 1, subX, "|__________|>");
            mvprintw(subY + 2, subX, "  o  o  o");
        }
        attroff(COLOR_PAIR(5));

        attron(COLOR_PAIR(1));
        char propeller = (frame % 4 == 0) ? '|' : (frame % 4 == 1) ? '/' : (frame % 4 == 2) ? '-' : '\\';
        if (subY + 1 < maxY) mvaddch(subY + 1, subX - 1, propeller);
        attroff(COLOR_PAIR(1));
    }

    if (cycleFrame % 25 < 22 && explodingMine < 0) {
        int sonarPhase = cycleFrame % 25;
        attron(COLOR_PAIR(6));
        for (int wave = 0; wave < 3; wave++) {
            int waveStart = wave * 6;
            if (sonarPhase >= waveStart) {
                int radius = (sonarPhase - waveStart) * 2;
                for (int angle = 0; angle < 360; angle += 20) {
                    double rad = angle * 3.14159 / 180.0;
                    int px = subX + 6 + (int)(radius * cos(rad) * 1.5);
                    int py = subY + (int)(radius * sin(rad) * 0.8);
                    if (px >= 0 && px < maxX && py > animY - 10 && py < maxY) {
                        char sonarChar = (wave == 0) ? '.' : (wave == 1) ? 'o' : 'O';
                        mvaddch(py, px, sonarChar);
                    }
                }
            }
        }
        attroff(COLOR_PAIR(6));
    }

    for (int school = 0; school < 4; school++) {
        int fishX = maxX - 10 - ((cycleFrame * (2 + school)) / 2) % (maxX + 20);
        int fishY = animY - 8 + school * 2;
        
        attron(COLOR_PAIR(2));
        if (fishX > -10 && fishX < maxX - 5 && fishY < maxY && fishY > 0) {
            for (int f = 0; f < 3; f++) {
                if (fishX + f * 5 < maxX - 5) {
                    mvprintw(fishY, fishX + f * 5, "<><");
                }
            }
        }
        attroff(COLOR_PAIR(2));
    }

    for (int j = 0; j < 3; j++) {
        int jellyX = 20 + j * 30 + (int)(sin((frame + j * 50) * 0.1) * 10);
        int jellyY = animY - 7 + (int)(sin((frame + j * 30) * 0.05) * 2);
        
        attron(COLOR_PAIR(6));
        if (jellyX > 0 && jellyX < maxX - 5 && jellyY > 0 && jellyY + 2 < maxY) {
            mvprintw(jellyY, jellyX, " _-_");
            mvprintw(jellyY + 1, jellyX, "(o.o)");
            mvprintw(jellyY + 2, jellyX, " | |");
        }
        attroff(COLOR_PAIR(6));
    }

    for (int b = 0; b < 15; b++) {
        int bubbleX = subX + (b % 5) * 3 + (int)(sin((frame + b) * 0.2) * 2);
        int bubbleY = subY - 1 - ((frame + b * 8) % 35) / 6;
        if (bubbleY >= animY - 10 && bubbleY < subY && explodingMine < 0) {
            attron(COLOR_PAIR(3));
            char bubble = ((frame + b) % 3 == 0) ? 'o' : ((frame + b) % 3 == 1) ? 'O' : '0';
            mvaddch(bubbleY, bubbleX, bubble);
            attroff(COLOR_PAIR(3));
        }
    }

    if (cycleFrame > 30 && cycleFrame < 150) {
        int enemyX = maxX - 25 - ((cycleFrame - 30) / 5);
        int enemyY = animY + 3;
        attron(COLOR_PAIR(4));
        if (enemyY + 1 < maxY) {
            mvprintw(enemyY - 1, enemyX, "  __|__");
            mvprintw(enemyY, enemyX, "<|______|");
            mvprintw(enemyY + 1, enemyX, " o  o  o");
        }
        attroff(COLOR_PAIR(4));
    }

    for (int s = 0; s < maxX; s += 15) {
        attron(COLOR_PAIR(2));
        int seaweedHeight = 2 + (s % 3);
        for (int h = 0; h < seaweedHeight; h++) {
            int swayX = s + (int)(sin((frame * 0.1 + h) * 0.5) * 1);
            int drawY = animY + 6 - h;
            if (swayX >= 0 && swayX < maxX && drawY < maxY) {
                mvaddch(drawY, swayX, '|');
            }
        }
        attroff(COLOR_PAIR(2));
    }

    if (cycleFrame > 90) {
        int diverX = 5 + ((cycleFrame - 90) / 3);
        int diverY = animY - 1 + (int)(sin((frame - 90) * 0.1) * 1);
        attron(COLOR_PAIR(5));
        if (diverX < maxX - 10 && diverY < maxY) {
            mvprintw(diverY, diverX, "O-<");
        }
        attroff(COLOR_PAIR(5));
    }

    attron(COLOR_PAIR(1));
}
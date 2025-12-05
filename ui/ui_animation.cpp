/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ui_animation.cpp
 * Description: Implementation of UI animation effects. Creates immersive animated
 *              sequences including underwater scenes with marine life, submarine
 *              navigation, combat sequences, and victory/defeat celebrations.
 *              Uses ncurses for terminal-based ASCII art animations.
 */

#include "ui_animation.hpp"
#include "ui_config.hpp"
#include <cmath>
#include <cstring>

// Cross-platform sleep implementation to control animation speed (FPS).
#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(x) Sleep(x)
#else
    #include <unistd.h>
    #define SLEEP_MS(x) usleep((x) * 1000)
#endif

/**
 * @brief Draws the small ship battle animation at the bottom of the screen.
 * @param frame The current frame number used for movement calculations.
 * @param startY The Y-coordinate where drawing begins.
 * @param maxX The width of the screen.
 */
void UIAnimation::drawBottomShipAnimation(int frame, int startY, int maxX) {
    // Clear the specific animation area to prevent artifacts
    for (int clearY = startY - 4; clearY <= startY + 6; clearY++) {
        move(clearY, 0);
        clrtoeol();
    }
    
    // The animation cycle repeats every 80 frames
    int cycleFrame = frame % 80;
    
    // Calculate ship positions: they move towards each other
    int yellowShipX = 15 + (cycleFrame / 2);
    int blueShipX = maxX - 25 - (cycleFrame / 2);
    
    // Draw waves using a sine wave function for fluid movement
    attron(COLOR_PAIR(3)); // Water color
    for (int i = 0; i < maxX; i += 2) {
        // Calculate wave height based on X position and frame
        int waveY = startY + 4 + (int)(sin((i + frame * 0.5) * 0.2) * 1.5);
        if (waveY >= 0) {
            mvaddch(waveY, i, '~');
            if (i + 1 < maxX) {
                mvaddch(waveY, i + 1, '~');
            }
        }
    }
    attroff(COLOR_PAIR(3));
    
    // Determine the battle phase based on the current frame
    int phase = (cycleFrame / 20);
    
    if (phase == 0 || phase == 1) {
        // Phase 1: Ships are sailing towards each other
        
        // Draw Yellow Ship (Left)
        attron(COLOR_PAIR(5));
        mvprintw(startY - 2, yellowShipX, "    _~_");
        mvprintw(startY - 1, yellowShipX, "   /___\\");
        mvprintw(startY,     yellowShipX, "  |=====|>");
        mvprintw(startY + 1, yellowShipX, " /~~~~~~~\\");
        mvprintw(startY + 2, yellowShipX, "~~~~~~~~~~~");
        attroff(COLOR_PAIR(5));
        
        // Draw Blue Ship (Right)
        attron(COLOR_PAIR(6));
        mvprintw(startY - 2, blueShipX, " _~_");
        mvprintw(startY - 1, blueShipX, "/___\\");
        mvprintw(startY,     blueShipX, "<|=====|");
        mvprintw(startY + 1, blueShipX, "/~~~~~~~\\");
        mvprintw(startY + 2, blueShipX, "~~~~~~~~~~~");
        attroff(COLOR_PAIR(6));
        
    } else if (phase == 2) {
        // Phase 2: The Yellow ship fires a projectile
        
        // Redraw Yellow Ship
        attron(COLOR_PAIR(5));
        mvprintw(startY - 2, yellowShipX, "    _~_");
        mvprintw(startY - 1, yellowShipX, "   /___\\");
        mvprintw(startY,     yellowShipX, "  |=====|>");
        mvprintw(startY + 1, yellowShipX, " /~~~~~~~\\");
        mvprintw(startY + 2, yellowShipX, "~~~~~~~~~~~");
        attroff(COLOR_PAIR(5));
        
        // Calculate projectile position based on progress within the phase
        int projectileProgress = cycleFrame % 20;
        int projectileX = yellowShipX + 11 + (projectileProgress * (blueShipX - yellowShipX - 15) / 20);
        
        // Draw projectile only if it hasn't hit the target yet
        if (projectileX < blueShipX - 2) {
            attron(COLOR_PAIR(2));
            mvprintw(startY, projectileX, "===>");
            attroff(COLOR_PAIR(2));
        }
        
        // Redraw Blue Ship (Target)
        attron(COLOR_PAIR(6));
        mvprintw(startY - 2, blueShipX, " _~_");
        mvprintw(startY - 1, blueShipX, "/___\\");
        mvprintw(startY,     blueShipX, "<|=====|");
        mvprintw(startY + 1, blueShipX, "/~~~~~~~\\");
        mvprintw(startY + 2, blueShipX, "~~~~~~~~~~~");
        attroff(COLOR_PAIR(6));
        
    } else if (phase == 3) {
        // Phase 3: Impact and Explosion
        
        // Yellow Ship remains
        attron(COLOR_PAIR(5));
        mvprintw(startY - 2, yellowShipX, "    _~_");
        mvprintw(startY - 1, yellowShipX, "   /___\\");
        mvprintw(startY,     yellowShipX, "  |=====|>");
        mvprintw(startY + 1, yellowShipX, " /~~~~~~~\\");
        mvprintw(startY + 2, yellowShipX, "~~~~~~~~~~~");
        attroff(COLOR_PAIR(5));
        
        int explosionFrame = cycleFrame % 20;
        if (explosionFrame < 10) {
            // Active explosion animation
            attron(COLOR_PAIR(4)); // Red color for fire
            mvprintw(startY - 3, blueShipX - 2, "  * * *");
            mvprintw(startY - 2, blueShipX - 2, " * * * *");
            mvprintw(startY - 1, blueShipX - 2, "* BOOM *");
            mvprintw(startY,     blueShipX - 2, "* * * * *");
            mvprintw(startY + 1, blueShipX - 2, " * * * *");
            mvprintw(startY + 2, blueShipX - 2, "  * * *");
            attroff(COLOR_PAIR(4));
        } else {
            // Sinking debris / smoke
            attron(COLOR_PAIR(1));
            mvprintw(startY - 1, blueShipX, " . . .");
            mvprintw(startY,     blueShipX, ". . . .");
            mvprintw(startY + 1, blueShipX, " . . .");
            attroff(COLOR_PAIR(1));
        }
    }
}

/**
 * @brief Displays the final "Game Over" screen with appropriate animations.
 * @param playerWon True if the player won (fireworks), False if lost (sunken ships).
 */
void UIAnimation::drawFirework(bool playerWon) {
    clear();
    
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    // Enable non-blocking input so animation keeps running until keypress
    nodelay(stdscr, TRUE);
    
    int frame = 0;
    bool running = true;
    
    // Setup for the celebration flotilla (used in Win scenario)
    int shipY = maxY - 6;
    (void)shipY; // Suppress unused variable warning
    int numShips = 5;
    int shipPositions[5];
    int shipColors[5] = {2, 3, 4, 5, 6};
    
    // Calculate evenly spaced positions for celebration ships
    int totalShipWidth = numShips * 8;
    int spacing = (maxX - totalShipWidth) / (numShips + 1);
    for (int s = 0; s < numShips; s++) {
        shipPositions[s] = spacing + s * (spacing + 8);
    }
    
    // Positions for sunken ships (decoration)
    int sunkenShipPositions[3] = {15, maxX/2 - 5, maxX - 30};
    
    // Main animation loop
    while (running) {
        clear();
        
        // Draw the static title header
        attron(COLOR_PAIR(1));
        mvprintw(0, (maxX - 50) / 2, "  ___            ___          _    _    _        ");
        mvprintw(1, (maxX - 50) / 2, " / __) ___  __ _| _ ) __ _ __| |_ | |_ | | ___   ");
        mvprintw(2, (maxX - 50) / 2, " \\__ \\/ -_)/ _` | _ \\/ _` (_-<  _||  _|| |/ -_)  ");
        mvprintw(3, (maxX - 50) / 2, " |___/\\___| \\__,_|___/\\__,_/__/\\__| \\__||_|\\___|  ");
        attroff(COLOR_PAIR(2));
        
        if (playerWon) {
            // ================== WIN SCENARIO ==================
            
            // Environment variables
            int seabedY = maxY - 2;
            int animationHeight = 20;
            int animationTop = seabedY - animationHeight;
            int sunkenShipY = seabedY - 4;
            int waveY = animationTop + 2;
            
            // Draw text
            int winTextY = 7;
            attron(COLOR_PAIR(2) | A_BOLD);
            const char* msg = "YOU WON!";
            mvprintw(winTextY, (maxX - strlen(msg)) / 2, msg);
            attroff(A_BOLD);
            
            attron(COLOR_PAIR(1));
            const char* pressKey = "Press any key to return to menu...";
            mvprintw(winTextY + 2, (maxX - strlen(pressKey)) / 2, pressKey);
            
            // Draw surface waves
            attron(COLOR_PAIR(3));
            for (int i = 0; i < maxX; i++) {
                int wave = waveY + (int)(sin((i + frame * 0.3) * 0.15) * 1.2);
                if (wave >= animationTop && wave < seabedY) {
                    mvaddch(wave, i, '~');
                }
            }
            attroff(COLOR_PAIR(3));
            
            // Draw sunken ships on the seabed (decor)
            for (int s = 0; s < 3; s++) {
                int shipX = sunkenShipPositions[s];
                int tilt = (s == 1) ? 0 : (s == 0 ? 1 : -1); // Different angles
                
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
            
            // Draw the seabed floor
            attron(COLOR_PAIR(2));
            for (int i = 0; i < maxX; i++) {
                if (i % 3 == 0) mvaddch(seabedY, i, '^');
                else if (i % 2 == 0) mvaddch(seabedY, i, '=');
                else mvaddch(seabedY, i, '_');
            }
            attroff(COLOR_PAIR(2));
            
            // Draw bobbing celebration ships on the surface
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
            
            // Handle Fireworks logic (Launch -> Ascend -> Explode)
            for (int i = 0; i < numShips; i++) {
                int launchX = shipPositions[i] + 3;
                int fireDelay = i * 20; // Staggered firing
                int localFrame = (frame - fireDelay + 80) % 80;
                
                if (localFrame < 60) {
                    int color = shipColors[i];
                    int raisedY = seabedY - 5;
                    
                    // Rocket ascent phase
                    if (localFrame < 25) {
                        int rocketY = raisedY - 2 - localFrame;
                        if (rocketY >= 5 && rocketY < raisedY) {
                            attron(COLOR_PAIR(color));
                            mvaddch(rocketY, launchX, '|');
                            if (rocketY + 1 < raisedY) {
                                mvaddch(rocketY + 1, launchX, '.'); // Rocket trail
                            }
                            attroff(COLOR_PAIR(color));
                        }
                    } else if (localFrame < 60) {
                        // Explosion phase
                        int explosionFrame = localFrame - 25;
                        int explosionY = raisedY - 2 - 25;
                        
                        attron(COLOR_PAIR(color));
                        
                        // Expanding explosion patterns based on time
                        if (explosionFrame < 8) {
                            // Small burst
                            mvaddch(explosionY, launchX, '*');
                            mvaddch(explosionY - 1, launchX, '*');
                            mvaddch(explosionY, launchX - 1, '*');
                            mvaddch(explosionY, launchX + 1, '*');
                            mvaddch(explosionY + 1, launchX, '*');
                        } else if (explosionFrame < 18) {
                            // Medium burst
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
                            // Large dissipating burst
                            mvaddch(explosionY - 3, launchX, '.');
                            mvaddch(explosionY - 2, launchX - 2, '.');
                            mvaddch(explosionY - 2, launchX, '*');
                            mvaddch(explosionY - 2, launchX + 2, '.');
                            // ... (more explosion particles)
                            mvaddch(explosionY + 3, launchX, '.');
                        } else {
                            // Fading particles
                            mvaddch(explosionY - 3, launchX - 1, '.');
                            mvaddch(explosionY - 3, launchX + 1, '.');
                            // ...
                            mvaddch(explosionY + 3, launchX + 1, '.');
                        }
                        
                        attroff(COLOR_PAIR(color));
                    }
                }
            }
            
            // Draw ambient bubbles from sunken ships
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
            
            // Draw swimming fish
            for (int fish = 0; fish < 4; fish++) {
                // Calculate fish movement
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
            
            // Draw jellyfish
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
            
            // Draw swaying seaweed
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
            
            // Draw walking crab
            int crabX = (frame / 2) % (maxX - 10);
            attron(COLOR_PAIR(4));
            mvprintw(seabedY - 1, crabX, "(V)o.o(V)");
            attroff(COLOR_PAIR(4));
            
            // Draw treasures near sunken ships (Winner's loot!)
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
            // ================== LOSS SCENARIO ==================
            // Similar to Win scenario but without fireworks/treasures
            
            int seabedY = maxY - 2;  
            int animationHeight = 20;  
            int animationTop = seabedY - animationHeight;  
            int sunkenShipY = seabedY - 4;  
            int waveY = animationTop + 2;  
            
            // Draw "Opponent Won" text
            int winTextY = 7;
            attron(COLOR_PAIR(4) | A_BOLD);
            const char* msg = "OPPONENT WON!";
            mvprintw(winTextY, (maxX - strlen(msg)) / 2, msg);
            attroff(A_BOLD);
            
            attron(COLOR_PAIR(1));
            const char* pressKey = "Press any key to return to menu...";
            mvprintw(winTextY + 2, (maxX - strlen(pressKey)) / 2, pressKey);
            
            // Draw background elements (Waves, seabed, sunken ships, fish, etc.)
            // [Code for environment rendering is identical to the Win block, omitted for brevity in comments]
            
            // ... (Waves)
            attron(COLOR_PAIR(3));
            for (int i = 0; i < maxX; i++) {
                int wave = waveY + (int)(sin((i + frame * 0.3) * 0.15) * 1.2);
                if (wave >= animationTop && wave < seabedY) {
                    mvaddch(wave, i, '~');
                }
            }
            attroff(COLOR_PAIR(3));
            
            // ... (Sunken Ships)
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
            
            // ... (Seabed Floor)
            attron(COLOR_PAIR(2));
            for (int i = 0; i < maxX; i++) {
                if (i % 3 == 0) mvaddch(seabedY, i, '^');
                else if (i % 2 == 0) mvaddch(seabedY, i, '=');
                else mvaddch(seabedY, i, '_');
            }
            attroff(COLOR_PAIR(2));
            
            // ... (Bubbles)
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
            
            // ... (Fish)
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
            
            // ... (Jellyfish)
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
            
            // ... (Seaweed)
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
            
            // ... (Crab)
            int crabX = (frame / 2) % (maxX - 10);
            attron(COLOR_PAIR(4));
            mvprintw(seabedY - 1, crabX, "(V)o.o(V)");
            attroff(COLOR_PAIR(4));
            
            // Treasures (different for loser - optional, currently same logic as winner in code structure)
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
        
        // Input check
        int ch = getch();
        if (ch != ERR) {
            running = false; // Exit loop on keypress
        }
        
        // Cap framerate
        #ifdef _WIN32
            Sleep(80);
        #else
            usleep(80000);
        #endif
        
        frame++;
    }
    
    // Cleanup
    nodelay(stdscr, FALSE);
    clear();
    refresh();
}

/**
 * @brief Draws the main menu background animation (Submarine, mines, sea life).
 * @param frame Current animation frame.
 */
void UIAnimation::drawMenuAnimation(int frame) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    // Define the base Y coordinate for the animation area
    int animY = maxY - 8;
    if (animY < 10) return; // Don't draw if screen is too small

    // Clear the animation zone area
    for (int clearY = animY - 13; clearY <= animY + 8; clearY++) {
        if (clearY >= 0 && clearY < maxY) {
            move(clearY, 0);
            clrtoeol();
        }
    }

    int cycleFrame = frame % 180;
    
    // Reset frame check for clean loop restart
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

    // Draw water surface waves
    attron(COLOR_PAIR(3));
    for (int i = 0; i < maxX; i++) {
        int waveY = animY - 11 + (int)(sin((i + frame * 0.3) * 0.15) * 1.2);
        if (waveY >= 0 && waveY < maxY) {
            mvaddch(waveY, i, '~');
        }
    }
    attroff(COLOR_PAIR(3));

    // Draw seabed
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

    // Calculate Submarine position
    int subX = 10 + (cycleFrame / 4);
    int subY = animY + 1;
    
    // Define Mine locations
    int minePositions[3] = {34, 59, 84};
    int hitMine = -1;
    
    // Collision Detection: Check if submarine hits a mine
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
    
    // Logic to remember which mine was hit for the explosion animation
    int wasHit = -1;
    if (cycleFrame > 10 && hitMine < 0) {
        for (int m = 0; m < 3; m++) {
            int mineX = minePositions[m];
            int frameAtMine = (mineX - 14 - 10) * 4;
            if (cycleFrame > frameAtMine && frameAtMine > 10) {
                // If we passed a mine recently, check if we triggered it
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
    
    // Draw active mines and danger warning
    if (cycleFrame > 10) {
        for (int m = 0; m < 3; m++) {
            if (hitMine == m || wasHit == m) continue; // Don't draw exploded mine here
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
        // Blinking warning text
        if ((frame / 8) % 2 == 0 && hitMine < 0 && wasHit < 0) {
            attron(COLOR_PAIR(4));
            mvprintw(animY - 5, maxX / 2 - 15, "!!! DANGER: MINES DETECTED !!!");
            attroff(COLOR_PAIR(4));
        }
    }
    
    int explodingMine = (hitMine >= 0) ? hitMine : wasHit;
    
    // Handle Submarine Drawing (Normal vs Exploding)
    if (explodingMine >= 0) {
        // --- EXPLOSION ANIMATION ---
        int explX = minePositions[explodingMine];
        int explY = animY + 2;
        int frameAtMine = (minePositions[explodingMine] - 14 - 10) * 4;
        
        int t = cycleFrame - frameAtMine; // Time since impact
        
        int duration = 10;
        int stopT = (t > duration) ? duration : t;

        // Draw Explosion Flash
        if (t < 3) {
            attron(COLOR_PAIR(5) | A_BOLD);
            if (explY - 3 >= 0) mvprintw(explY - 3, explX - 4, "    * * * ");
            if (explY - 2 >= 0) mvprintw(explY - 2, explX - 4, "  * * * * * ");
            if (explY - 1 >= 0) mvprintw(explY - 1, explX - 4, "* * BOOM! * *");
            if (explY < maxY)   mvprintw(explY, explX - 4,     " * * * * * ");
            if (explY + 1 < maxY) mvprintw(explY + 1, explX - 4, "    * * * ");
            attroff(COLOR_PAIR(5) | A_BOLD);
        }

        // Break the submarine into pieces
        int shipBaseX = 10 + (frameAtMine / 4);
        int shipBaseY = subY;
        int floorLimit = animY + 6; 

        attron(COLOR_PAIR(1));

        // Piece 1: Cabin flies up and forward
        int cabX = shipBaseX + 2 + (stopT / 2); 
        int cabY = shipBaseY - 1 + (stopT / 2); 
        if (cabY > floorLimit - 1) cabY = floorLimit - 1;
        
        if (cabY < maxY && cabX < maxX) {
            mvprintw(cabY, cabX, "__");
            mvprintw(cabY + 1, cabX, "/  |");
        }

        // Piece 2: Tail falls back
        int tailX = shipBaseX - (stopT / 3); 
        int tailY = shipBaseY + 1 + (stopT / 2);
        if (tailY > floorLimit) tailY = floorLimit; 
        
        if (tailY < maxY && tailX > 0) {
             mvprintw(tailY, tailX, "|____");
             mvaddch(tailY, tailX - 1, '+'); 
        }

        // Piece 3: Nose flies forward
        int noseX = shipBaseX + 7 + stopT;
        int noseY = shipBaseY + (stopT / 2);
        if (noseY > floorLimit) noseY = floorLimit;

        if (noseY < maxY && noseX < maxX) {
            mvprintw(noseY, noseX, "\\___");
            mvprintw(noseY + 1, noseX, "_|>");
        }
        
        // Final debris resting on floor
        if (t >= duration) {
            int debrisY = floorLimit + 1;
            if (debrisY < maxY) {
                mvaddch(debrisY, shipBaseX + 5, 'o'); 
                mvaddch(debrisY, shipBaseX + 8, '#'); 
            }
        }
        attroff(COLOR_PAIR(1));

    } else {
        // --- NORMAL SUBMARINE DRAWING ---
        attron(COLOR_PAIR(5));
        if (subY + 2 < maxY) {
            mvprintw(subY - 2, subX, "   __");
            mvprintw(subY - 1, subX, "  /  |");
            mvprintw(subY, subX, " |o   \\___");
            mvprintw(subY + 1, subX, "|__________|>");
            mvprintw(subY + 2, subX, "  o  o  o");
        }
        attroff(COLOR_PAIR(5));

        // Animated Propeller
        attron(COLOR_PAIR(1));
        char propeller = (frame % 4 == 0) ? '|' : (frame % 4 == 1) ? '/' : (frame % 4 == 2) ? '-' : '\\';
        if (subY + 1 < maxY) mvaddch(subY + 1, subX - 1, propeller);
        attroff(COLOR_PAIR(1));
    }

    // Draw active Sonar Pings (if not exploded)
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

    // Draw Schools of Fish
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

    // Draw Jellyfish
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

    // Draw Bubbles trailing from submarine
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

    // Draw Enemy Submarine passing by in background
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

    // Draw Swaying Seaweed
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

    // Draw Scuba Diver
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
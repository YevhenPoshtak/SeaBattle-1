/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ui_renderer.cpp
 * Description: Implementation of UI rendering functions. Handles all visual output
 *              for the game including board displays, menus, status indicators,
 *              and user interface elements using ncurses library.
 */

#include "ui_renderer.hpp"
#include "ui_animation.hpp"
#include <cstring>
#include <cstdio>
#include <signal.h>
#include <locale.h>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(x) Sleep(x)
#else
    #include <unistd.h>
    #define SLEEP_MS(x) usleep((x) * 1000)
#endif

/**
 * @brief Signal handler for graceful program termination.
 * @param sig The signal number received (typically SIGINT for Ctrl+C).
 * 
 * Ensures ncurses is properly cleaned up before exit to restore terminal state.
 */
static void signal_handler(int sig) {
    endwin();
    exit(0);
}

/**
 * @brief Initializes the ncurses window and sets up color pairs.
 * 
 * Creates the main window, enables colors, defines 6 color pairs for game elements,
 * sets up signal handling, and configures input modes (cbreak, noecho, keypad).
 */
void UIRenderer::setupWindow() {
    setlocale(LC_ALL, "");
    
    WINDOW *win = initscr();
    
    if (win == NULL) {
        printf("ERROR: initscr() failed!\n");
        exit(1);
    }
    
    // Initialize color pairs for different game elements
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);   // Default text
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Ships, success messages
    init_pair(3, COLOR_BLUE, COLOR_BLACK);    // Water, misses
    init_pair(4, COLOR_RED, COLOR_BLACK);     // Hits, explosions, warnings
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);  // Highlights, player indicators
    init_pair(6, COLOR_CYAN, COLOR_BLACK);    // Enemy indicators, special effects
    
    signal(SIGINT, signal_handler);
    attron(COLOR_PAIR(1));
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    drawTitle();
    refresh();
}

/**
 * @brief Draws the main "SeaBattle" ASCII art title at the top of the screen.
 * 
 * Centers the title horizontally and adds decorative separator lines below it.
 */
void UIRenderer::drawTitle() {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    (void)maxY; // Suppress unused variable warning
    
    // ASCII art title lines
    const char* title[] = {
        "  ___            ___          _    _    _        ",
        " / __) ___  __ _| _ ) __ _ __| |_ | |_ | | ___   ",
        " \\__ \\/ -_)/ _` | _ \\/ _` (_-<  _||  _|| |/ -_)  ",
        " |___/\\___| \\__,_|___/\\__,_/__/\\__| \\__||_|\\___|  "
    };
    
    int titleWidth = strlen(title[0]);
    int startX = (maxX - titleWidth) / 2;
    
    // Draw each line of the title
    for (int i = 0; i < 4; i++) {
        mvprintw(i, startX, title[i]);
    }
    
    // Draw decorative separator
    mvprintw(5, (maxX - 50) / 2 - 1, "  ----------------------------------------------- ");
    mvprintw(6, (maxX - 50) / 2 - 1, " |          Choose your game mode below          |");
    mvprintw(7, (maxX - 50) / 2 - 1, "  ----------------------------------------------- ");
    
    refresh();
}

/**
 * @brief Draws the two game boards side by side with headers and grid structure.
 * @param layout Board layout configuration containing position coordinates.
 * @param boardSize Dimension of the board (NxN).
 * @param leftTitle Title text for the left board (player's board).
 * @param rightTitle Title text for the right board (opponent's board).
 * 
 * Creates the complete board structure including:
 * - Title headers with centered text
 * - Column labels (A, B, C, ...)
 * - Row numbers (1, 2, 3, ...)
 * - Grid lines and separators
 */
void UIRenderer::drawGameBoards(const BoardLayout& layout, int boardSize, const char* leftTitle, const char* rightTitle) {
    int boardWidth = boardSize * 4 + 8;
    
    int leftTitleLen = strlen(leftTitle);
    int rightTitleLen = strlen(rightTitle);
    
    // Calculate padding for centered titles
    int leftPad = (boardWidth - leftTitleLen) / 2;
    int rightPad = boardWidth - leftPad - leftTitleLen;
    int leftPad2 = (boardWidth - rightTitleLen) / 2;
    int rightPad2 = boardWidth - leftPad2 - rightTitleLen;
    
    // Draw left board title with dashes
    move(layout.startY, layout.board1StartX);
    for (int i = 0; i < leftPad; i++) printw("-");
    printw("%s", leftTitle);
    for (int i = 0; i < rightPad; i++) printw("-");
    
    // Draw separator between boards
    move(layout.startY, layout.separatorX);
    printw("~~~~~");
    
    // Draw right board title with dashes
    move(layout.startY, layout.board2StartX);
    for (int i = 0; i < leftPad2; i++) printw("-");
    printw("%s", rightTitle);
    for (int i = 0; i < rightPad2; i++) printw("-");
    printw("\n");

    // Draw underline for titles
    move(layout.startY + 1, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("_");
    move(layout.startY + 1, layout.separatorX);
    printw("~~~~~");
    move(layout.startY + 1, layout.board2StartX);
    for (int i = 0; i < boardWidth; i++) printw("_");
    printw("\n");

    // Draw column headers (A, B, C, ...)
    move(layout.startY + 2, layout.board1StartX);
    printw("|  | A |");
    for (int i = 1; i < boardSize; i++) {
        printw(" %c |", 'A' + i);
    }
    
    move(layout.startY + 2, layout.separatorX);
    printw("~~~~~");
    
    move(layout.startY + 2, layout.board2StartX + 4);
    printw("|  | A |");
    for (int i = 1; i < boardSize; i++) {
        printw(" %c |", 'A' + i);
    }
    printw("\n");

    // Draw grid rows with row numbers
    for (int i = 0; i < boardSize; i++) {
        // Left board row
        move(layout.startY + 3 + i, layout.board1StartX);
        printw("|%2d|", i + 1);
        for (int j = 0; j < boardSize; j++) {
            printw("   |");
        }
        
        // Separator
        move(layout.startY + 3 + i, layout.separatorX);
        printw("~~~~~");
        
        // Right board row
        move(layout.startY + 3 + i, layout.board2StartX + 4);
        printw("|%2d|", i + 1);
        for (int j = 0; j < boardSize; j++) {
            printw("   |");
        }
    }

    // Draw bottom borders
    move(layout.startY + 3 + boardSize, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("-");
    move(layout.startY + 3 + boardSize, layout.separatorX);
    printw("~~~~~");
    move(layout.startY + 3 + boardSize, layout.board2StartX);
    for (int i = 0; i < boardWidth; i++) printw("-");
    printw("\n");
}

/**
 * @brief Renders a single board cell with appropriate color and character.
 * @param screenY The Y coordinate on screen where to draw.
 * @param screenX The X coordinate on screen where to draw.
 * @param cell The character representing the cell state.
 * @param isPlayerBoard Whether this is the player's board (shows ships) or enemy's (hides ships).
 * 
 * Cell states:
 * - 'w' or ' ': Empty water
 * - 'o': Miss (blue O)
 * - 'x': Hit (red X)
 * - 's': Sunk ship (red bold S)
 * - 'A'-'Z': Ship segments (green on player board, hidden on enemy board)
 */
void UIRenderer::drawBoardCell(int screenY, int screenX, char cell, bool isPlayerBoard) {
    move(screenY, screenX);
    
    if (cell == 'w' || cell == ' ') {
        addch(' ');
    } else if (cell == 'o') {
        attron(COLOR_PAIR(3)); // Blue for misses
        addch('O');
        attroff(COLOR_PAIR(3));
    } else if (cell == 'x') {
        attron(COLOR_PAIR(4)); // Red for hits
        addch('X');
        attroff(COLOR_PAIR(4));
    } else if (cell == 's') {
        attron(COLOR_PAIR(4) | A_BOLD); // Bold red for sunk ships
        addch('S');
        attroff(COLOR_PAIR(4) | A_BOLD);
    } else if (cell >= 'A' && cell <= 'Z') {
        if (isPlayerBoard) {
            attron(COLOR_PAIR(2)); // Green for player's ships
            addch(cell);
            attroff(COLOR_PAIR(2));
        } else {
            addch(' '); // Hide enemy ships
        }
    } else {
        addch(cell);
    }
    
    attron(COLOR_PAIR(1));
}

/**
 * @brief Updates the visual state of an entire board by drawing all cells.
 * @param layout Board layout configuration.
 * @param board Board data structure containing the 2D array of cell states.
 * @param isPlayerBoard True for player's board, false for enemy's board.
 */
void UIRenderer::drawBoardState(const BoardLayout& layout, const BoardData& board, bool isPlayerBoard) {
    // Calculate the starting X offset for the appropriate board
    int xOffset = isPlayerBoard ? layout.board1StartX + 5 : layout.board2StartX + 9;
    
    // Draw each cell in the board
    for (int i = 0; i < board.boardSize; i++) {
        for (int j = 0; j < board.boardSize; j++) {
            int screenY = layout.startY + 3 + i;
            int screenX = xOffset + (4 * j);
            drawBoardCell(screenY, screenX, board.boardArray[i][j], isPlayerBoard);
        }
    }
}

/**
 * @brief Displays control instructions in the top-left corner.
 * @param layout Board layout configuration (unused but kept for consistency).
 */
void UIRenderer::drawInstructions(const BoardLayout& layout) {
    attron(A_UNDERLINE);
    mvprintw(1, 1, "instructions");
    attroff(A_UNDERLINE);
    
    mvprintw(2, 1, "w/up - up      a/left - left");
    mvprintw(3, 1, "s/down - down    d/right - right");
    mvprintw(4, 1, "space/enter - select target");
    mvprintw(5, 1, "f - fire all shots");
    mvprintw(6, 1, "q - quit game");
}

/**
 * @brief Displays the current ship count for both player and enemy.
 * @param y Y coordinate where to display stats.
 * @param x X coordinate where to display stats.
 * @param playerShips Number of player's remaining ships.
 * @param enemyShips Number of enemy's remaining ships.
 */
void UIRenderer::drawGameStats(int y, int x, int playerShips, int enemyShips) {
    move(y, x);
    clrtoeol();
    attron(COLOR_PAIR(5) | A_BOLD);
    printw("YOUR SHIPS: %d", playerShips);
    attroff(A_BOLD);
    
    move(y, x + 20);
    attron(COLOR_PAIR(6) | A_BOLD);
    printw("ENEMY: %d", enemyShips);
    attroff(A_BOLD);
    attron(COLOR_PAIR(1));
}

/**
 * @brief Draws a visual indicator around a selected shot target.
 * @param y Y coordinate of the target cell.
 * @param x X coordinate of the target cell.
 * @param selected Whether this shot is currently selected/highlighted.
 */
void UIRenderer::drawShotIndicator(int y, int x, bool selected) {
    if (selected) {
        attron(COLOR_PAIR(5) | A_BOLD);
        move(y, x - 1); addch('[');
        move(y, x);     addch('+');
        move(y, x + 1); addch(']');
        attroff(COLOR_PAIR(5) | A_BOLD);
    }
    attron(COLOR_PAIR(1));
}

/**
 * @brief Clears a previously drawn shot indicator.
 * @param y Y coordinate of the indicator.
 * @param x X coordinate of the indicator.
 */
void UIRenderer::clearShotIndicator(int y, int x) {
    attron(COLOR_PAIR(1));
    move(y, x - 1); addch(' ');  
    move(y, x);     addch(' ');  
    move(y, x + 1); addch(' ');  
}

/**
 * @brief Displays the results of a volley (multiple shots).
 * @param startY Y coordinate where to start drawing the result.
 * @param startX X coordinate where to start drawing the result.
 * @param coords String containing the coordinates of shots fired.
 * @param stats String containing statistics (hits, misses, sinks).
 * @param isPlayer True if this is the player's volley, false for enemy's.
 */
void UIRenderer::drawVolleyResult(int startY, int startX, const std::string& coords, const std::string& stats, bool isPlayer) {
    // Clear the display area
    for (int i = 0; i < 3; i++) {
        move(startY + i, startX);
        clrtoeol();
    }
    
    // Draw header with appropriate color
    if (isPlayer) {
        attron(A_UNDERLINE | COLOR_PAIR(2));
        mvprintw(startY, startX, "Your volley:");
        attroff(A_UNDERLINE | COLOR_PAIR(2));
    } else {
        attron(A_UNDERLINE | COLOR_PAIR(4));
        mvprintw(startY, startX, "Enemy volley:");
        attroff(A_UNDERLINE | COLOR_PAIR(4));
    }
    
    // Draw coordinates and statistics
    attron(COLOR_PAIR(1));
    mvprintw(startY + 1, startX, "%s%s", coords.c_str(), stats.c_str());
}

/**
 * @brief Displays a message at a specific location with color.
 * @param y Y coordinate.
 * @param x X coordinate.
 * @param message The text to display.
 * @param colorPair Color pair number to use (1-6).
 */
void UIRenderer::showMessage(int y, int x, const std::string& message, int colorPair) {
    move(y, x);
    clrtoeol();
    attron(COLOR_PAIR(colorPair));
    printw("%s", message.c_str());
    attron(COLOR_PAIR(1));
}

/**
 * @brief Clears an entire line on the screen.
 * @param y The line number to clear.
 */
void UIRenderer::clearLine(int y) {
    move(y, 0);
    clrtoeol();
}

/**
 * @brief Moves the cursor to a specific position.
 * @param y Y coordinate.
 * @param x X coordinate.
 */
void UIRenderer::drawCursor(int y, int x) {
    move(y, x);
}

/**
 * @brief Cleans up ncurses and restores terminal state.
 */
void UIRenderer::cleanup() {
    endwin();
}

/**
 * @brief Interactive menu for selecting board size.
 * @return The selected board size, or -1 if cancelled.
 * 
 * Features:
 * - Visual slider bar showing current size
 * - Preview grid representation
 * - Terminal size validation
 * - Keyboard controls: W/S for size adjustment, Enter to confirm, ESC to cancel
 */
int UIRenderer::selectBoardSize() {
    clear();
    
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    // Draw title
    attron(COLOR_PAIR(1) | A_BOLD);     
    const char* title = "Select Board Size";
    mvprintw(2, (maxX - strlen(title)) / 2, "%s", title);
    attroff(A_BOLD);
    
    // Draw instructions
    const char* hint1 = "Use UP/DOWN arrows or W/S to change size";
    const char* hint2 = "Press ENTER to confirm or ESC to cancel";
    mvprintw(4, (maxX - strlen(hint1)) / 2, "%s", hint1);
    mvprintw(5, (maxX - strlen(hint2)) / 2, "%s", hint2);
    
    int currentSize = getBoardSize(); 
    bool selecting = true;
    
    while (selecting) {
        // Display current size
        attron(COLOR_PAIR(2) | A_BOLD);
        char sizeText[50];
        sprintf(sizeText, "Size: %dx%d", currentSize, currentSize);
        int centerX = (maxX - strlen(sizeText)) / 2;
        
        move(8, centerX - 5);
        for (int i = 0; i < 30; i++) printw(" ");
        
        mvprintw(8, centerX, "%s", sizeText);
        attroff(A_BOLD);
        
        // Draw progress bar
        attron(COLOR_PAIR(3));
        int barY = 10;
        int barStartX = (maxX - 40) / 2;
        int barWidth = 40;
        
        mvprintw(barY, barStartX, "[");
        mvprintw(barY, barStartX + barWidth - 1, "]");
        
        // Calculate filled portion of bar
        int filledWidth = ((currentSize - MIN_BOARD_SIZE) * (barWidth - 2)) / (MAX_BOARD_SIZE - MIN_BOARD_SIZE);
        for (int i = 0; i < barWidth - 2; i++) {
            if (i < filledWidth) {
                attron(COLOR_PAIR(2));
                mvprintw(barY, barStartX + 1 + i, "=");
            } else {
                attron(COLOR_PAIR(1));
                mvprintw(barY, barStartX + 1 + i, "-");
            }
        }
        attroff(COLOR_PAIR(3));
        
        // Draw min/max labels
        attron(COLOR_PAIR(1));
        mvprintw(barY + 1, barStartX, "%d", MIN_BOARD_SIZE);
        char maxText[10];
        sprintf(maxText, "%d", MAX_BOARD_SIZE);
        mvprintw(barY + 1, barStartX + barWidth - strlen(maxText), "%s", maxText);
        
        // Draw preview grid
        int previewSize = 12;
        int previewY = 13;
        int previewX = (maxX - (previewSize * 2)) / 2;
        
        attron(COLOR_PAIR(3));
        mvprintw(previewY - 1, previewX + previewSize - 5, " Preview:");
        
        for (int i = 0; i < previewSize; i++) {
            move(previewY + i, previewX);
            for (int j = 0; j < previewSize; j++) {
                if ((i + j) % 2 == 0) {
                    printw("::");
                } else {
                    printw("..");
                }
            }
        }
        attroff(COLOR_PAIR(3));
        
        // Calculate and display terminal size requirements
        int reqY, reqX;
        getRequiredTerminalSize(currentSize, reqY, reqX);

        // Clear previous size info
        for (int i = 0; i < 5; i++) {
            move(previewY + previewSize + 2 + i, 0);
            clrtoeol();
        }

        attron(COLOR_PAIR(1));
        mvprintw(previewY + previewSize + 2, 2, "Required terminal size: %dx%d", reqX, reqY);
        mvprintw(previewY + previewSize + 3, 2, "Current terminal size:  %dx%d", maxX, maxY);
        
        // Display fit status
        if (canFitInterface(currentSize, maxY, maxX)) {
            attron(COLOR_PAIR(2));
            mvprintw(previewY + previewSize + 4, 2, "[OK] Interface will fit in terminal");
            attroff(COLOR_PAIR(2));
        } else {
            attron(COLOR_PAIR(4));
            mvprintw(previewY + previewSize + 4, 2, "[X] Interface will NOT fit - increase terminal size!");
            attroff(COLOR_PAIR(4));
        }
        
        refresh();
        
        // Handle user input
        flushinp();
        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'w':
            case 'W':
                if (currentSize < MAX_BOARD_SIZE) {
                    currentSize++;
                }
                break;
                
            case KEY_DOWN:
            case 's':
            case 'S':
                if (currentSize > MIN_BOARD_SIZE) {
                    currentSize--;
                }
                break;
                
            case KEY_RIGHT:
            case 'd':
            case 'D':
                if (currentSize < MAX_BOARD_SIZE) {
                    currentSize++;
                }
                break;
                
            case KEY_LEFT:
            case 'a':
            case 'A':
                if (currentSize > MIN_BOARD_SIZE) {
                    currentSize--;
                }
                break;
                
            case 10:  // Enter key
            case ' ':
                if (canFitInterface(currentSize, maxY, maxX)) {
                    setBoardSize(currentSize);
                    clear();
                    return currentSize;
                } else {
                    // Show warning if terminal is too small
                    attron(COLOR_PAIR(4) | A_BOLD);
                    const char* warning = "Please increase terminal window size!";
                    mvprintw(previewY + previewSize + 6, (maxX - strlen(warning)) / 2, "%s", warning);
                    attroff(A_BOLD);
                    refresh();
                    SLEEP_MS(1500);
                    move(previewY + previewSize + 6, 0);
                    clrtoeol();
                }
                break;
                
            case 27:  // ESC key
            case 'q':
            case 'Q':
                clear();
                return -1; 
                
            default:
                break;
        }
    }
    
    return currentSize;
}

/**
 * @brief Interactive menu for selecting number of shots per turn.
 * @param boardSize Current board size (used to calculate recommended shots).
 * @return The selected number of shots per turn.
 * 
 * Features:
 * - Recommended value based on board size
 * - Visual slider bar
 * - Fine adjustment (±1) with W/S keys
 * - Coarse adjustment (±5) with A/D keys
 * - Manual input option with 'm' key
 */
int UIRenderer::selectShotsPerTurn(int boardSize) {
    auto config = getShipConfig(boardSize);
    int recommended = config.shotsPerTurn;
    
    clear();
    
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    (void)maxY;
        
    // Draw title and board info
    attron(COLOR_PAIR(1) | A_BOLD);
    const char* title = "Select Shots Per Turn";
    mvprintw(2, (maxX - strlen(title)) / 2, "%s", title);
    attroff(A_BOLD);
    
    char info[100];
    sprintf(info, "Board size: %dx%d", boardSize, boardSize);
    mvprintw(4, (maxX - strlen(info)) / 2, "%s", info);
    
    sprintf(info, "Recommended: %d shots", recommended);
    attron(COLOR_PAIR(2));
    mvprintw(5, (maxX - strlen(info)) / 2, "%s", info);
    attroff(COLOR_PAIR(2));
    
    // Draw instructions
    const char* hint1 = "Use UP/DOWN or W/S to change by 1";
    const char* hint2 = "Use LEFT/RIGHT or A/D to change by 5";
    const char* hint3 = "Press 'm' to enter custom value (1-26)";
    const char* hint4 = "Press ENTER to confirm";
    mvprintw(7, (maxX - strlen(hint1)) / 2, "%s", hint1);
    mvprintw(8, (maxX - strlen(hint2)) / 2, "%s", hint2);
    mvprintw(9, (maxX - strlen(hint3)) / 2, "%s", hint3);
    mvprintw(10, (maxX - strlen(hint4)) / 2, "%s", hint4);
    
    int currentShots = 3;
    int minShots = 1;
    int maxShots = 26;
    
    bool selecting = true;
    
    while (selecting) {
        // Display current shot count
        attron(COLOR_PAIR(2) | A_BOLD);
        char shotsText[50];
        sprintf(shotsText, "Shots: %d", currentShots);
        int centerX = (maxX - strlen(shotsText)) / 2;
        
        move(13, centerX - 10);
        for (int i = 0; i < 40; i++) printw(" ");
            
        mvprintw(13, centerX, "%s", shotsText);
        attroff(A_BOLD);
            
        // Draw progress bar
        int barY = 15;
        int barWidth = 50;
        int barStartX = (maxX - barWidth) / 2;
            
        attron(COLOR_PAIR(3));
        mvprintw(barY, barStartX, "[");
        mvprintw(barY, barStartX + barWidth - 1, "]");
        attroff(COLOR_PAIR(3));
        
        // Calculate filled portion
        int innerWidth = barWidth - 2;
        int filledWidth = 0;
        if (maxShots > minShots) {
            filledWidth = ((currentShots - minShots) * innerWidth) / (maxShots - minShots);
            if (currentShots == maxShots) filledWidth = innerWidth;
        }

        for (int i = 0; i < innerWidth; i++) {
            if (i < filledWidth) {
                attron(COLOR_PAIR(2));
                mvprintw(barY, barStartX + 1 + i, "=");
            } else {
                attron(COLOR_PAIR(1));
                mvprintw(barY, barStartX + 1 + i, "-");
            }
        }
        
        // Draw min/max labels
        attron(COLOR_PAIR(1));
        mvprintw(barY + 1, barStartX, "%d", minShots);
        
        char maxText[10];
        sprintf(maxText, "%d", maxShots);
        mvprintw(barY + 1, barStartX + barWidth - strlen(maxText), "%s", maxText);
        
        // Display gameplay tips
        const char* hint5 = "More shots = faster game, easier gameplay";
        const char* hint6 = "Fewer shots = longer game, more strategic";
        mvprintw(18, (maxX - strlen(hint5)) / 2, "%s", hint5);
        mvprintw(19, (maxX - strlen(hint6)) / 2, "%s", hint6);
        
        refresh();
        
        // Handle user input
        flushinp();
        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'w':
            case 'W':
                if (currentShots < maxShots) currentShots++;
                break;
                
            case KEY_DOWN:
            case 's':
            case 'S':
                if (currentShots > minShots) currentShots--;
                break;
                
            case KEY_RIGHT:
            case 'd':
            case 'D':
                currentShots = (currentShots + 5 > maxShots) ? maxShots : currentShots + 5;
                break;
                
            case KEY_LEFT:
            case 'a':
            case 'A':
                currentShots = (currentShots - 5 < minShots) ? minShots : currentShots - 5;
                break;
                
            case 'm':
            case 'M':
                {
                    // Manual input mode - allow user to type exact number
                    move(13, centerX - 10);
                    for (int i = 0; i < 40; i++) printw(" ");
                    
                    attron(COLOR_PAIR(5));
                    mvprintw(13, centerX - 5, "Enter shots (1-26): ");
                    attroff(COLOR_PAIR(5));
                    
                    // Enable echo and cursor for text input
                    echo();
                    curs_set(1);
                    
                    char input[10];
                    memset(input, 0, sizeof(input));
                    mvgetnstr(13, centerX + 15, input, 9);
                    
                    // Disable echo and cursor after input
                    noecho();
                    curs_set(0);
                    
                    // Parse and validate input
                    int value = atoi(input);
                    if (value >= minShots && value <= maxShots) {
                        currentShots = value;
                    } else if (value > maxShots) {
                        currentShots = maxShots;
                    } else if (value < minShots) {
                        currentShots = minShots;
                    }
                    
                    // Clear input area
                    move(13, centerX - 15);
                    for (int i = 0; i < 40; i++) printw(" ");
                }
                break;
                
            case 10:  // Enter key
            case ' ':
                clear();
                return currentShots;
                
            case 27:  // ESC key
            case 'q':
            case 'Q':
                clear();
                return 3;  // Default value
        }
    }
    
    return 3;
}

/**
 * @brief Displays a warning when terminal size is insufficient for the selected board size.
 * @param boardSize The board size that doesn't fit.
 * @return Always returns false.
 * 
 * Shows current vs required terminal dimensions and waits for keypress.
 */
bool UIRenderer::showTerminalSizeWarning(int boardSize) {
    clear();
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    int minY, minX;
    getRequiredTerminalSize(boardSize, minY, minX);
    
    // Display warning message in red
    attron(COLOR_PAIR(4));
    mvprintw(maxY/2 - 2, 2, "WARNING: Terminal size too small!");
    attroff(COLOR_PAIR(4));
    
    // Show size comparison
    mvprintw(maxY/2, 2, "Current size: %dx%d", maxX, maxY);
    mvprintw(maxY/2 + 1, 2, "Required size: %dx%d", minX, minY);
    mvprintw(maxY/2 + 3, 2, "Please increase terminal window size");
    mvprintw(maxY/2 + 4, 2, "or reduce board size (current: %dx%d)", boardSize, boardSize);
    mvprintw(maxY/2 + 6, 2, "Press any key to return...");
    refresh();
    getch();
    
    return false;
}

/**
 * @brief Asks the player to confirm their board placement.
 * @return True if player confirms (y/Y), false if they decline (n/N).
 * 
 * Loops until valid input (y or n) is received.
 */
bool UIRenderer::confirmBoardPlacement() {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    (void)maxX;
    
    move(maxY - 3, 1);
    printw("Do you want to use this board? Enter y/n");
    refresh();
    
    while (true) {
        flushinp();
        switch (getch()) {
            case 'y':
            case 'Y':
                return true;
            case 'n':
            case 'N':
                return false;
            default:
                move(maxY - 3, 1);
                printw("Invalid Input please enter y/n to accept board\n");
                refresh();
        }
    }
}

/**
 * @brief Highlights the ship being placed during manual board setup.
 * @param layout Board layout configuration.
 * @param cursorX Current cursor X position on screen.
 * @param cursorY Current cursor Y position on screen.
 * @param shipLength Length of the ship being placed.
 * @param orientation 0 for horizontal, 1 for vertical.
 * @param symbol Character representing the ship.
 * @param board Current board state.
 * @param isValid Output parameter set to true if placement is valid, false otherwise.
 * 
 * Shows the ship in highlight (standout) if placement is valid, or in red if invalid
 * (colliding with another ship or out of bounds).
 */
void UIRenderer::highlightShipPlacement(const BoardLayout& layout, int cursorX, int cursorY, int shipLength, int orientation, char symbol, const BoardData& board, bool& isValid) {
    isValid = true;
    
    // Convert screen coordinates to grid coordinates
    int gridX = (cursorX - layout.board1StartX - 5) / 4;
    int gridY = cursorY - layout.startY - 3;
    
    if (orientation == 0) {
        // Horizontal placement (extends left from cursor)
        for (int i = 0; i < shipLength; i++) {
            int checkX = gridX - i;
            
            // Check if position is valid (in bounds and on water)
            if (checkX < 0 || checkX >= board.boardSize || board.boardArray[gridY][checkX] != 'w') {
                // Invalid placement - show in red
                move(cursorY, cursorX - (4 * i));
                char c = inch() & A_CHARTEXT;
                attron(COLOR_PAIR(4));
                addch(c);
                attroff(COLOR_PAIR(4));
                isValid = false;
            } else {
                // Valid placement - show highlighted
                move(cursorY, cursorX - (4 * i));
                attron(A_STANDOUT);
                addch(symbol);
                attroff(A_STANDOUT);
            }
        }
    } else {
        // Vertical placement (extends up from cursor)
        for (int i = 0; i < shipLength; i++) {
            int checkY = gridY - i;
            
            // Check if position is valid
            if (checkY < 0 || checkY >= board.boardSize || board.boardArray[checkY][gridX] != 'w') {
                // Invalid placement - show in red
                move(cursorY - i, cursorX);
                char c = inch() & A_CHARTEXT;
                attron(COLOR_PAIR(4));
                addch(c);
                attroff(COLOR_PAIR(4));
                isValid = false;
            } else {
                // Valid placement - show highlighted
                move(cursorY - i, cursorX);
                attron(A_STANDOUT);
                addch(symbol);
                attroff(A_STANDOUT);
            }
        }
    }
    
    // Return cursor to original position
    move(cursorY, cursorX);
}

/**
 * @brief Draws the board during random/generated board placement mode.
 * @param layout Board layout configuration.
 * @param board Current board state with placed ships.
 * 
 * Shows the complete board with all ship placements and control instructions.
 * Title size adapts to board dimensions to ensure proper fit.
 */
void UIRenderer::drawGeneratedBoard(const BoardLayout& layout, const BoardData& board) {
    int size = board.boardSize;
    int boardWidth = size * 4 + 8;
    
    clear();
    move(layout.startY, layout.board1StartX);
    
    // Choose title based on board size to fit properly
    const char* title = (size >= 20) ? "You" : (size >= 15) ? "Your" : "Your Board";
    int titleLen = strlen(title);
    int leftPad = (boardWidth - titleLen) / 2;
    int rightPad = boardWidth - leftPad - titleLen;
    
    // Draw title with dashes
    for (int i = 0; i < leftPad; i++) printw("-");
    printw("%s", title);
    for (int i = 0; i < rightPad; i++) printw("-");
    printw("\n");
    
    // Draw board border
    move(layout.startY + 1, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("_");
    printw("\n");
    
    // Draw column headers (A, B, C, ...)
    move(layout.startY + 2, layout.board1StartX);
    printw("|  | A |");
    for (int i = 1; i < size; i++) {
        printw(" %c |", 'A' + i);
    }
    printw("\n");

    // Draw grid with row numbers
    for (int i = 0; i < size; i++) {
        move(layout.startY + 3 + i, layout.board1StartX);
        printw("|%2d|", i + 1);
        for (int j = 0; j < size; j++) {
            printw("   |");
        }
    }
    
    // Draw bottom border
    move(layout.startY + 3 + size, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("-");
    printw("\n");
    
    // Draw instructions for generated board mode
    attron(A_UNDERLINE);
    mvprintw(1, 1, "instructions");
    attroff(A_UNDERLINE);
    
    mvprintw(2, 1, "y - accept board placement");
    mvprintw(3, 1, "n - randomize board");
    mvprintw(4, 1, "m - switch to manual mode");

    refresh();
    
    // Draw ship placements on the board
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board.boardArray[i][j] != 'w') {
                move(layout.startY + 3 + i, layout.board1StartX + 5 + (j * 4));
                addch(board.boardArray[i][j]);
            }
        }
    }
    refresh();
}

/**
 * @brief Draws the board during manual ship placement mode.
 * @param layout Board layout configuration.
 * @param board Current board state with placed ships.
 * 
 * Shows the board with instructions for manual placement controls.
 * Already placed ships are drawn in bold to distinguish them.
 */
void UIRenderer::drawManualBoard(const BoardLayout& layout, const BoardData& board) {
    int size = board.boardSize;
    int boardWidth = size * 4 + 8;
    
    clear();
    move(layout.startY, layout.board1StartX);
    
    // Choose title based on board size
    const char* title = (size >= 20) ? "You" : (size >= 15) ? "Your" : "Your Board";
    int titleLen = strlen(title);
    int leftPad = (boardWidth - titleLen) / 2;
    int rightPad = boardWidth - leftPad - titleLen;
    
    // Draw title
    for (int i = 0; i < leftPad; i++) printw("-");
    printw("%s", title);
    for (int i = 0; i < rightPad; i++) printw("-");
    printw("\n");
    
    // Draw board border
    move(layout.startY + 1, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("_");
    printw("\n");
    
    // Draw column headers
    move(layout.startY + 2, layout.board1StartX);
    printw("|  |");
    for (int i = 0; i < size; i++) {
        printw(" %c |", 'A' + i);
    }
    printw("\n");
    
    // Draw grid rows with row numbers
    for (int i = 0; i < size; i++) {
        move(layout.startY + 3 + i, layout.board1StartX);
        // Format row number (single digit gets space padding)
        if (i + 1 < 10) {
            printw("| %d|", i + 1);
        } else {
            printw("|%d|", i + 1);
        }
        for (int j = 0; j < size; j++) {
            printw("   |");
        }
    }
    
    // Draw bottom border
    move(layout.startY + 3 + size, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("-");
    printw("\n");
    
    // Draw manual placement instructions
    attron(A_UNDERLINE);
    mvprintw(1, 1, "instructions");
    attroff(A_UNDERLINE);
    
    mvprintw(2, 1, "w/u - up      a/<- - left");
    mvprintw(3, 1, "s/d - down    d/-> - right");
    mvprintw(4, 1, "r - rotate piece");
    mvprintw(5, 1, "space/enter - place piece");
    mvprintw(6, 1, "g - switch to random mode");

    // Draw already placed ships in bold
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board.boardArray[i][j] != 'w') {
                move(layout.startY + 3 + i, layout.board1StartX + 5 + (j * 4));
                attron(A_BOLD);
                addch(board.boardArray[i][j]);
                attroff(A_BOLD);
            }
        }
    }
    
    refresh();
}

/**
 * @brief Displays and handles the main menu with animated background.
 * @param selectedOption Reference to the currently selected menu option (modified).
 * @return The index of the selected option (0-6), or -1 if cancelled.
 * 
 * Menu options:
 * 0. vs Easy AI
 * 1. vs Smart AI
 * 2. Host (Multiplayer)
 * 3. Client (Multiplayer)
 * 4. Board Size Settings
 * 5. Debug Tests
 * 6. Quit
 * 
 * Features:
 * - Animated submarine background (drawMenuAnimation)
 * - Keyboard navigation with W/S or arrow keys
 * - Current board size displayed above menu
 * - Non-blocking input for smooth animation at ~10 FPS
 * - Visual cursor (>>>) and highlighted selection
 */
int UIRenderer::showMainMenu(int& selectedOption) {
    drawTitle();

    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    (void)maxY;
    
    // Define menu options
    std::string options[7] = {
        "1) vs Easy AI", 
        "2) vs Smart AI", 
        "3) Host (Multiplayer)", 
        "4) Client (Multiplayer)",
        "5) Board Size Settings",
        "6) Debug Tests",
        "7) Quit"
    };
    
    int i = selectedOption;
    int prev = 0;
    curs_set(0);  // Hide cursor
    
    // Calculate menu positioning
    int menuStartY = 10;
    int longestOption = 0;
    for (int j = 0; j < 7; j++) {
        if ((int)options[j].length() > longestOption) {
            longestOption = options[j].length();
        }
    }
    int menuStartX = (maxX - longestOption) / 2 + 3;
    int cursorX = menuStartX - 4;
    
    // Display current board size info
    char sizeInfo[50];
    sprintf(sizeInfo, "Current board: %dx%d", getBoardSize(), getBoardSize());
    attron(COLOR_PAIR(6));
    mvprintw(menuStartY - 2, (maxX - strlen(sizeInfo)) / 2 - 1, "%s", sizeInfo);
    attroff(COLOR_PAIR(6));
    
    // Draw all menu options (highlight selected one)
    for (int j = 0; j < 7; j++) {
        if (j == i) {
            attron(COLOR_PAIR(2) | A_BOLD);
            mvprintw(menuStartY + j, cursorX, ">>>");
            attroff(COLOR_PAIR(2) | A_BOLD);
            attron(A_STANDOUT);
        }
        mvprintw(menuStartY + j, menuStartX, options[j].c_str());
        if (j == i) {
            attroff(A_STANDOUT);
        }
    }
    
    // Enable non-blocking input for animation
    nodelay(stdscr, TRUE);
    
    bool isActiveMenu = true;
    int frame = 0;
    int returnValue = -1;
    
    // Main menu loop with animation
    while (isActiveMenu) {
        prev = i;
        int c = getch();
        
        // Handle input if key was pressed
        if (c != ERR) {
            switch (c) {
                case KEY_UP:
                case 'w':
                case 'W':
                    i--;
                    i = (i < 0) ? 6 : i;  // Wrap to bottom
                    break;
                case KEY_DOWN:
                case 's':
                case 'S':
                    i++;
                    i = (i > 6) ? 0 : i;  // Wrap to top
                    break;
                case 10:  // Enter key
                case ' ':
                    nodelay(stdscr, FALSE);
                    selectedOption = i;
                    returnValue = i;
                    isActiveMenu = false;
                    break;
            }
            
            // Redraw menu if selection changed
            mvprintw(menuStartY + prev, cursorX, "   ");
            attroff(A_STANDOUT);
            mvprintw(menuStartY + prev, menuStartX, options[prev].c_str());
            
            attron(COLOR_PAIR(2) | A_BOLD);
            mvprintw(menuStartY + i, cursorX, ">>>");
            attroff(COLOR_PAIR(2) | A_BOLD);
            attron(A_STANDOUT);
            mvprintw(menuStartY + i, menuStartX, options[i].c_str());
            attroff(A_STANDOUT);
        }
        
        // Draw animated background
        UIAnimation::drawMenuAnimation(frame);
        refresh();
        
        // Control animation speed (~10 FPS)
        SLEEP_MS(100);
        
        // Update frame counter (reset to prevent overflow)
        frame++;
        if (frame >= 60) {
            frame = 0;
        }
    }
    
    // Restore blocking input mode and show cursor
    nodelay(stdscr, FALSE);
    curs_set(2);
    return returnValue;
}
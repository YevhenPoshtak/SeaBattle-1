/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: main.cpp
 * Description: Main entry point for the Battleship game. Initializes the game systems,
 *              handles the main menu loop, and routes to different game modes (AI vs Player,
 *              Multiplayer Host/Client). Manages terminal size validation and game settings.
 */

#include "ui/ui_renderer.hpp"
#include "ui/ui_config.hpp"
#include "logic/network_logic.hpp"
#include "game/game_modes.hpp"
#include "game/ai_game_loop.hpp"
#include "game/multiplayer_game_loop.hpp"
#include "tests/SeaBattle_1_test.hpp"
#include <locale.h>
#include <cstdlib>
#include <ctime>

// Global game settings
GameSettings g_gameSettings;

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    // Enable locale support for proper character display
    setlocale(LC_ALL, "");
    
    // Initialize networking subsystem
    if (!NetworkLogic::initializeNetworking()) {
        printf("Failed to initialize networking\n");
        return 1;
    }

    // Initialize random seed for ship placement and AI
    srand(time(NULL));
    
    // Set default game configuration
    setBoardSize(10);
    g_gameSettings.shotsPerTurn = 5;

    // Initialize ncurses UI
    UIRenderer::setupWindow();
    
    int selectedOption = 0;
    bool running = true;
    
    // Main menu loop
    while (running) {
        clear();
        UIRenderer::drawTitle();
        
        // Display menu and get user choice
        int choice = UIRenderer::showMainMenu(selectedOption);

        // Get current terminal dimensions for validation
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX); 

        switch (choice) {
            case MODE_AI_EASY: 
                // Check if terminal is large enough for the interface
                if (!canFitInterface(getBoardSize(), maxY, maxX)) { 
                    UIRenderer::showTerminalSizeWarning(getBoardSize());
                } else {
                    playAIGame(EASY);
                }
                break;
                
            case MODE_AI_SMART: 
                if (!canFitInterface(getBoardSize(), maxY, maxX)) {
                    UIRenderer::showTerminalSizeWarning(getBoardSize());
                } else {
                    playAIGame(SMART);
                }
                break;

            case MODE_MULTIPLAYER_HOST:
                if (!canFitInterface(getBoardSize(), maxY, maxX)) {
                    UIRenderer::showTerminalSizeWarning(getBoardSize());
                } else {
                    playMultiplayerHost();
                }
                break;

            case MODE_MULTIPLAYER_CLIENT: 
                if (!canFitInterface(getBoardSize(), maxY, maxX)) {
                    UIRenderer::showTerminalSizeWarning(getBoardSize());
                } else {
                    playMultiplayerClient();
                }
                break;
                
            case MODE_BOARD_SIZE_SETTINGS:
                // Allow user to customize board size
                UIRenderer::selectBoardSize();
                break;
                
            case MODE_DEBUG_TESTS:
                // Run internal validation tests
                runDebugTests();
                break;
                
            case MODE_QUIT: 
                running = false;
                break;
        }
    }

    // Clean up resources
    UIRenderer::cleanup();
    NetworkLogic::cleanupNetworking();
    
    return 0;
}
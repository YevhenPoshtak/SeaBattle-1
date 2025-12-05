/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: SeaBattle_1_test.cpp
 * Description: Comprehensive test suite for the Battleship game. Implements automated,
 *              manual, and file-based testing of core game mechanics including board
 *              initialization, ship placement, shot validation, AI behavior, and game
 *              state management. Provides detailed test reporting and interactive
 *              testing capabilities.
 */

#include "SeaBattle_1_test.hpp"
#include "../data/board_data.hpp"
#include "../data/game_state.hpp"
#include "../logic/game_logic.hpp"
#include "../logic/ai_logic.hpp"
#include "../ui/ui_config.hpp"
#include "../ui/ui_renderer.hpp"
#include <fstream>
#include <vector>
#include <cstring>
#include <sstream>
#include <set>

#ifdef _WIN32
    #include <windows.h>
    #include <pdcurses.h>
    #define SLEEP_MS(x) Sleep(x)
#else
    #include <unistd.h>
    #include <ncurses.h>
    #define SLEEP_MS(x) usleep((x) * 1000)
#endif

#define TEST_AREA_START 5
#define TEST_AREA_HEIGHT 20

// Structure to store individual test results
struct TestResult {
    std::string testName;    // Name/description of the test
    bool passed;             // Whether test passed or failed
    std::string message;     // Additional information or error message
};

// Global test tracking variables
static std::vector<TestResult> testResults;  // Collection of all test results
static std::ofstream outputFile;              // Output file for test logs

/*
 * Add a test result to the results collection and log file
 * Parameters:
 *   name - Name of the test
 *   passed - Whether the test passed
 *   msg - Optional additional message
 */
static void addTestResult(const std::string& name, bool passed, const std::string& msg = "") {
    testResults.push_back({name, passed, msg});
    
    if (outputFile.is_open()) {
        outputFile << (passed ? "[PASS] " : "[FAIL] ") << name;
        if (!msg.empty()) {
            outputFile << " - " << msg;
        }
        outputFile << std::endl;
    }
}

/*
 * Display a single test result on screen with color coding
 * Parameters:
 *   y - Starting Y coordinate for display area
 *   currentY - Current Y position (updated after display)
 *   result - Test result to display
 */
static void displayTestResult(int y, int& currentY, const TestResult& result) {
    if (currentY >= y + 20) return;
    
    // Display PASS/FAIL indicator with appropriate color
    if (result.passed) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(currentY, 2, "[PASS]");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(currentY, 2, "[FAIL]");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Display test name
    attron(COLOR_PAIR(1));
    mvprintw(currentY, 10, "%s", result.testName.c_str());
    
    // Display additional message if present and fits on screen
    if (!result.message.empty() && result.message.length() < 50) {
        attron(COLOR_PAIR(6));
        mvprintw(currentY, 50, "- %s", result.message.c_str());
        attroff(COLOR_PAIR(6));
    }
    
    currentY++;
}

/*
 * Test Category 1: Board Size Validation
 * Tests board initialization with various sizes and validates water initialization
 */
static void testBoardSizeValidation() {
    // Test minimum board size
    BoardData board1(MIN_BOARD_SIZE);
    addTestResult("Board: Min Size", board1.boardSize == MIN_BOARD_SIZE, 
                  "Size=" + std::to_string(MIN_BOARD_SIZE));
    
    // Test maximum board size
    BoardData board2(MAX_BOARD_SIZE);
    addTestResult("Board: Max Size", board2.boardSize == MAX_BOARD_SIZE,
                  "Size=" + std::to_string(MAX_BOARD_SIZE));
    
    // Test standard 10x10 board
    BoardData board3(10);
    addTestResult("Board: Normal Size", board3.boardSize == 10, "10x10");
    
    // Verify all cells initialized to water
    bool allWater = true;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (board3.boardArray[i][j] != 'w') {
                allWater = false;
            }
        }
    }
    addTestResult("Board: Water Init", allWater, "All cells = 'w'");
}

/*
 * Test Category 2: Ship Placement Validation
 * Tests valid and invalid ship placements including boundary cases
 */
static void testShipPlacementValidation() {
    BoardData board(10);
    
    // Test valid horizontal placement
    bool valid1 = GameLogic::isValidShipPlacement(board, 5, 5, 0, 3);
    addTestResult("Ship: Valid Horizontal", valid1, "pos(5,5) len=3");
    
    // Test valid vertical placement
    bool valid2 = GameLogic::isValidShipPlacement(board, 5, 5, 1, 3);
    addTestResult("Ship: Valid Vertical", valid2, "pos(5,5) len=3");
    
    // Test out-of-bounds left
    bool invalid1 = !GameLogic::isValidShipPlacement(board, 1, 5, 0, 3);
    addTestResult("Ship: OOB Left", invalid1, "x=1, len=3");
    
    // Test out-of-bounds top
    bool invalid2 = !GameLogic::isValidShipPlacement(board, 5, 1, 1, 3);
    addTestResult("Ship: OOB Top", invalid2, "y=1, len=3");
    
    // Test edge placement right
    bool valid3 = GameLogic::isValidShipPlacement(board, 9, 5, 0, 2);
    addTestResult("Ship: Valid at Right Edge", valid3, "x=9, len=2");
    
    // Test edge placement bottom
    bool valid4 = GameLogic::isValidShipPlacement(board, 5, 9, 1, 2);
    addTestResult("Ship: Valid at Bottom Edge", valid4, "y=9, len=2");
    
    // Test overlap detection
    GameLogic::placeShip(board, 5, 5, 0, 3, 'A');
    bool invalid5 = !GameLogic::isValidShipPlacement(board, 4, 5, 0, 3);
    addTestResult("Ship: Overlap Detection", invalid5, "Blocks existing ship");
}

/*
 * Test Category 3: Ship Rotation
 * Tests horizontal and vertical ship placement and rotation constraints
 */
static void testShipRotation() {
    BoardData board(10);
    
    // Test horizontal ship placement
    GameLogic::placeShip(board, 5, 5, 0, 3, 'H');
    bool horiz = (board.boardArray[5][3] == 'H' && 
                  board.boardArray[5][4] == 'H' && 
                  board.boardArray[5][5] == 'H');
    addTestResult("Rotation: Horizontal Ship", horiz, "cells (3,5)-(5,5)");
    
    // Test vertical ship placement
    board.clear();
    GameLogic::placeShip(board, 5, 5, 1, 3, 'V');
    bool vert = (board.boardArray[3][5] == 'V' && 
                 board.boardArray[4][5] == 'V' && 
                 board.boardArray[5][5] == 'V');
    addTestResult("Rotation: Vertical Ship", vert, "cells (5,3)-(5,5)");
    
    // Test rotation blocked by left edge
    board.clear();
    bool cantRotateLeft = !GameLogic::isValidShipPlacement(board, 2, 5, 0, 4);
    addTestResult("Rotation: Left Edge Block", cantRotateLeft, "x=2, len=4 goes to x=-1");
}

/*
 * Test Category 4: Shot Validation
 * Tests hit detection, miss marking, duplicate shot blocking, ship sinking, and boundary protection
 */
static void testShotValidation() {
    BoardData board(10);
    board.addShip(1, 55, 3, 'S');  // Place ship at (5,5) vertical, length 3
    
    // Test miss detection and marking
    int miss = board.receiveShot(0, 0);
    addTestResult("Shot: Miss", miss == 0 && board.boardArray[0][0] == 'o', 
                  "Returns 0, marks 'o'");
    
    // Test hit detection and marking
    int hit = board.receiveShot(5, 5);
    addTestResult("Shot: Hit", hit == 1 && board.boardArray[5][5] == 'x',
                  "Returns 1, marks 'x'");
    
    // Test duplicate shot blocking
    int dup = board.receiveShot(5, 5);
    addTestResult("Shot: Duplicate Block", dup == 0, "Returns 0 for re-shot");
    
    // Test ship sinking
    board.receiveShot(5, 6);
    int sink = board.receiveShot(5, 7);
    bool sunk = (sink == 2);
    bool allMarkedSunk = true;
    for (int i = 5; i <= 7; i++) {
        if (board.boardArray[i][5] != 's') {
            allMarkedSunk = false;
        }
    }
    addTestResult("Shot: Sink Ship", sunk && allMarkedSunk, 
                  "Returns 2, all cells marked 's'");
    
    // Test shooting already sunk ship
    int shootSunk = board.receiveShot(5, 5);
    addTestResult("Shot: Sunk Ship Block", shootSunk == 0, 
                  "Shooting sunk ship returns 0");
    
    // Test out-of-bounds protection
    int oob1 = board.receiveShot(-1, 5);
    int oob2 = board.receiveShot(5, -1);
    int oob3 = board.receiveShot(10, 5);
    int oob4 = board.receiveShot(5, 10);
    addTestResult("Shot: OOB Protection", 
                  oob1 == 0 && oob2 == 0 && oob3 == 0 && oob4 == 0,
                  "All OOB return 0");
}

/*
 * Test Category 5: Ship Counting
 * Tests ship count tracking, wounded count, and sunk count functionality
 */
static void testShipCounting() {
    BoardData board(10);
    
    // Place multiple ships
    board.addShip(1, 11, 4, 'A');  // 4-cell ship
    board.addShip(1, 33, 3, 'B');  // 3-cell ship
    board.addShip(1, 55, 2, 'C');  // 2-cell ship
    board.addShip(0, 77, 1, 'D');  // 1-cell ship
    
    // Test initial ship count
    int totalShips = board.getRemainingShips();
    addTestResult("Ships: Initial Count", totalShips == 4, 
                  "4 ships placed");
    
    // Sink one ship and test count update
    board.receiveShot(1, 1);
    board.receiveShot(1, 2);
    board.receiveShot(1, 3);
    board.receiveShot(1, 4);
    
    int remaining = board.getRemainingShips();
    addTestResult("Ships: After Sink", remaining == 3,
                  "3 ships remaining");
    
    // Test wounded count
    board.receiveShot(3, 3);
    int wounded = board.getWoundedCount();
    addTestResult("Ships: Wounded Count", wounded == 1,
                  "1 wounded hit");
    
    // Test sunk count
    int sunk = board.getSunkCount();
    addTestResult("Ships: Sunk Count", sunk == 1,
                  "1 ship sunk");
}

/*
 * Test Category 6: Volley System
 * Tests multiple simultaneous shots and miss tracking
 */
static void testVolleySystem() {
    BoardData board(10);
    board.addShip(1, 11, 3, 'A');
    board.addShip(0, 55, 2, 'B');
    
    // Execute multiple shots
    int shot1 = board.receiveShot(1, 1);  // Hit
    int shot2 = board.receiveShot(0, 0);  // Miss
    int shot3 = board.receiveShot(5, 5);  // Hit
    
    bool volleyWorks = (shot1 == 1 && shot2 == 0 && shot3 == 1);
    addTestResult("Volley: Multi-Shot", volleyWorks,
                  "3 shots: hit, miss, hit");
    
    // Verify miss counting
    int missCount = board.missCount;
    addTestResult("Volley: Miss Tracking", missCount == 1,
                  "1 miss recorded");
}

/*
 * Test Category 7: Easy AI Behavior
 * Tests random shot generation and board coverage for Easy difficulty AI
 */
static void testEasyAI() {
    AILogic easyAI(EASY, 10);
    
    // Test random shot uniqueness
    std::set<std::pair<int,int>> coords;
    for (int i = 0; i < 10; i++) {
        AICoordinates shot = easyAI.pickAttackCoordinates();
        coords.insert({shot.x, shot.y});
    }
    
    bool randomShots = (coords.size() == 10);
    addTestResult("AI Easy: Random Shooting", randomShots,
                  "10 unique shots");
    
    // Test full board coverage capability
    AILogic easyAI2(EASY, 10);
    bool noRepeat = true;
    for (int i = 0; i < 100; i++) {
        AICoordinates shot = easyAI2.pickAttackCoordinates();
        if (shot.x < 0 || shot.y < 0) {
            noRepeat = false;
            break;
        }
    }
    addTestResult("AI Easy: Full Board Coverage", noRepeat,
                  "100 shots without crash");
}

/*
 * Test Category 8: Smart AI Behavior
 * Tests intelligent targeting, parity strategy, and hunt mode for Smart difficulty AI
 */
static void testSmartAI() {
    AILogic smartAI(SMART, 10);
    
    // Test adjacent targeting after hit
    smartAI.recordShotResult(5, 5, true, false);
    AICoordinates nextShot = smartAI.pickAttackCoordinates();
    
    bool smartTargeting = (
        (nextShot.x == 4 && nextShot.y == 5) ||
        (nextShot.x == 6 && nextShot.y == 5) ||
        (nextShot.x == 5 && nextShot.y == 4) ||
        (nextShot.x == 5 && nextShot.y == 6)
    );
    
    addTestResult("AI Smart: Adjacent Targeting", smartTargeting,
                  "Targets neighbors");
    
    // Test checkerboard parity pattern
    AILogic smartAI2(SMART, 10);
    std::set<std::pair<int,int>> parityCoords;
    
    for (int i = 0; i < 50; i++) {
        AICoordinates shot = smartAI2.pickAttackCoordinates();
        parityCoords.insert({shot.x, shot.y});
    }
    
    int parityMatches = 0;
    for (const auto& coord : parityCoords) {
        if ((coord.first + coord.second) % 2 == 0) {
            parityMatches++;
        }
        if (parityMatches > 25) break;
    }
    
    bool usesParity = (parityMatches >= 25);
    addTestResult("AI Smart: Parity Strategy", usesParity,
                  std::to_string(parityMatches) + "/50 parity shots");
    
    // Test hunt mode reset after sink
    AILogic smartAI3(SMART, 10);
    smartAI3.recordShotResult(3, 3, true, false);
    smartAI3.recordShotResult(3, 4, true, true);
    
    AICoordinates afterSink = smartAI3.pickAttackCoordinates();
    bool behaviorOk = (afterSink.x >= 0 && afterSink.y >= 0);
    addTestResult("AI Smart: Hunt Mode Reset", behaviorOk,
                  "Continues after sink");
}

/*
 * Test Category 9: Game State Management
 * Tests game state initialization and game over detection
 */
static void testGameState() {
    GameState state;
    state.initialize(10, 3, true);
    
    // Test initialization
    addTestResult("GameState: Initialization", 
                  state.boardSize == 10 && state.shotsPerTurn == 3,
                  "10x10, 3 shots");
    
    // Test game not over condition
    state.playerHits = 15;
    state.maxHits = 20;
    bool notOver = !state.isGameOver();
    addTestResult("GameState: Game Not Over", notOver,
                  "15/20 hits");
    
    // Test game over condition
    state.playerHits = 20;
    bool isOver = state.isGameOver();
    addTestResult("GameState: Game Over", isOver,
                  "20/20 hits reached");
    
    // Test victory detection
    bool playerWon = state.hasPlayerWon();
    addTestResult("GameState: Player Victory", playerWon,
                  "Player reached max hits");
}

/*
 * Test Category 10: Ship Configuration
 * Tests ship count and shot scaling for different board sizes
 */
static void testShipConfiguration() {
    // Test 10x10 board configuration
    ShipConfiguration config10 = getShipConfig(10);
    int total10 = config10.fourDeck + config10.threeDeck + 
                  config10.twoDeck + config10.oneDeck;
    addTestResult("Config: 10x10 Ships", total10 == 10,
                  std::to_string(total10) + " ships");
    
    // Test 15x15 board configuration
    ShipConfiguration config15 = getShipConfig(15);
    int total15 = config15.fourDeck + config15.threeDeck + 
                  config15.twoDeck + config15.oneDeck;
    addTestResult("Config: 15x15 Ships", total15 > total10,
                  std::to_string(total15) + " ships (more than 10x10)");
    
    // Test shot scaling
    addTestResult("Config: Shots Scale", config15.shotsPerTurn >= config10.shotsPerTurn,
                  "Bigger board = more shots");
}

/*
 * Test Category 11: Board Generation
 * Tests automatic ship placement and generation stability
 */
static void testBoardGeneration() {
    BoardData board(10);
    std::vector<GamePiece> pieces;
    GameLogic::initializeGamePieces(board, pieces);
    
    // Test piece creation
    addTestResult("Generation: Pieces Created", !pieces.empty(),
                  std::to_string(pieces.size()) + " pieces");
    
    // Test ship placement
    GameLogic::generateBoardPlacement(board, pieces);
    
    int shipCells = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (board.boardArray[i][j] >= 'A' && board.boardArray[i][j] <= 'Z') {
                shipCells++;
            }
        }
    }
    
    int expectedCells = getTotalShipCells(10);
    addTestResult("Generation: All Ships Placed", shipCells == expectedCells,
                  std::to_string(shipCells) + "/" + std::to_string(expectedCells) + " cells");
    
    // Test generation stability
    bool multipleGens = true;
    for (int i = 0; i < 10; i++) {
        board.clear();
        GameLogic::generateBoardPlacement(board, pieces);
        if (board.getRemainingShips() != (int)pieces.size()) {
            multipleGens = false;
            break;
        }
    }
    addTestResult("Generation: Stability", multipleGens,
                  "10 regenerations successful");
}

/*
 * Test Category 12: Coordinate System
 * Tests coordinate-to-position conversion and ship cell retrieval
 */
static void testCoordinateSystem() {
    BoardData board(10);
    board.addShip(1, 55, 3, 'T');  // Position 55 = (x=5, y=5)
    
    // Test position calculation
    bool correctPos = (board.boardArray[5][5] == 'T');
    addTestResult("Coordinates: Position Calc", correctPos,
                  "pos 55 = (5,5)");
    
    // Test occupied cell retrieval
    auto cells = board.getShipOccupiedCells(5, 5);
    bool correctCells = (cells.size() == 3);
    addTestResult("Coordinates: Ship Cells", correctCells,
                  std::to_string(cells.size()) + " cells");
}

/*
 * Run interactive manual tests with user input
 * Allows testing of all major game features through console interaction
 */
void runManualTests() {
    // Enable character echoing and show the cursor for manual input
    echo();
    curs_set(1);
    
    // ==========================================
    // Manual Test 1: Board Size Validation
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD); // Set yellow bold text
    mvprintw(2, 2, "TEST 1/12: Board Size Validation");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    mvprintw(4, 2, "Enter board size (10-26): ");
    refresh();
    
    int size;
    scanw("%d", &size); // Capture user input
    
    // Clamp size to valid range (10 to 26)
    if (size < MIN_BOARD_SIZE) size = MIN_BOARD_SIZE;
    if (size > MAX_BOARD_SIZE) size = MAX_BOARD_SIZE;
    
    // Initialize board and verify every cell is initialized to 'w' (Water)
    BoardData board(size);
    bool allWater = true;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board.boardArray[i][j] != 'w') {
                allWater = false;
            }
        }
    }
    
    std::string sizeStr = std::to_string(size) + "x" + std::to_string(size);
    addTestResult("Console: Board Size", allWater && size >= 10 && size <= 26, sizeStr);
    
    // Display result
    mvprintw(6, 2, "Result:");
    if (allWater) {
        attron(COLOR_PAIR(2) | A_BOLD); // Green for Pass
        mvprintw(7, 4, "[PASS] Board created: %dx%d, all cells = water", size, size);
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD); // Red for Fail
        mvprintw(7, 4, "[FAIL] Board initialization error");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(9, 2, "Press any key to continue...");
    refresh();
    getch(); // Wait for user input
    
    // ==========================================
    // Manual Test 2: Ship Placement Validation
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 2/12: Ship Placement Validation");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    // --- Part A: Test Valid Placement ---
    mvprintw(4, 2, "VALID placement - Enter ship position:");
    mvprintw(5, 2, "  X (0-%d): ", size-1);
    refresh();
    int x1;
    scanw("%d", &x1);
    
    mvprintw(6, 2, "  Y (0-%d): ", size-1);
    refresh();
    int y1;
    scanw("%d", &y1);
    
    mvprintw(7, 2, "  Orientation (0=horizontal, 1=vertical): ");
    refresh();
    int orient1;
    scanw("%d", &orient1);
    
    mvprintw(8, 2, "  Length (1-4): ");
    refresh();
    int len1;
    scanw("%d", &len1);
    
    // Check if placement is allowed by game rules
    bool valid1 = GameLogic::isValidShipPlacement(board, x1, y1, orient1, len1);
    if (valid1) {
        GameLogic::placeShip(board, x1, y1, orient1, len1, 'A');
    }
    
    char coords1[100];
    sprintf(coords1, "(%d,%d) len=%d orient=%d", x1, y1, len1, orient1);
    addTestResult("Console: Valid Placement", valid1, coords1);
    
    mvprintw(10, 2, "Result:");
    if (valid1) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(11, 4, "[PASS] Ship placed at (%d,%d), length=%d, orient=%d", x1, y1, len1, orient1);
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(11, 4, "[FAIL] Invalid placement at (%d,%d)", x1, y1);
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // --- Part B: Test Invalid Placement ---
    mvprintw(13, 2, "INVALID placement - Enter position OUT OF BOUNDS:");
    mvprintw(14, 2, "  X: ");
    refresh();
    int x2;
    scanw("%d", &x2);
    
    mvprintw(15, 2, "  Y: ");
    refresh();
    int y2;
    scanw("%d", &y2);
    
    mvprintw(16, 2, "  Orientation (0=horizontal, 1=vertical): ");
    refresh();
    int orient2;
    scanw("%d", &orient2);
    
    mvprintw(17, 2, "  Length (1-4): ");
    refresh();
    int len2;
    scanw("%d", &len2);
    
    // Logic should reject this (return false)
    bool invalid = !GameLogic::isValidShipPlacement(board, x2, y2, orient2, len2);
    
    char coords2[100];
    sprintf(coords2, "(%d,%d) len=%d orient=%d", x2, y2, len2, orient2);
    addTestResult("Console: Invalid Placement", invalid, coords2);
    
    mvprintw(19, 2, "Result:");
    if (invalid) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(20, 4, "[PASS] Correctly rejected invalid placement");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(20, 4, "[FAIL] Accepted invalid placement (should reject!)");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(22, 2, "Press any key to continue...");
    refresh();
    getch();

    // ==========================================
    // Manual Test 3: Ship Rotation Logic
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 3/12: Ship Rotation");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    BoardData rotBoard(10);
    
    // Test Horizontal Placement
    mvprintw(4, 2, "Testing HORIZONTAL ship at (5,5) len=3...");
    GameLogic::placeShip(rotBoard, 5, 5, 0, 3, 'H');
    // Verify specific array indices: Horizontal grows along X axis
    // Note: This logic assumes ships grow left-to-right or right-to-left depending on implementation
    bool horizOk = (rotBoard.boardArray[5][3] == 'H' && 
                    rotBoard.boardArray[5][4] == 'H' && 
                    rotBoard.boardArray[5][5] == 'H');
    addTestResult("Console: Horizontal Ship", horizOk, "cells (3,5)-(5,5)");
    
    mvprintw(6, 2, "Result:");
    if (horizOk) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(7, 4, "[PASS] Horizontal ship placed correctly at cells (5,3), (5,4), (5,5)");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(7, 4, "[FAIL] Horizontal ship placement error");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Test Vertical Placement
    rotBoard.clear();
    mvprintw(9, 2, "Testing VERTICAL ship at (5,5) len=3...");
    GameLogic::placeShip(rotBoard, 5, 5, 1, 3, 'V');
    // Verify specific array indices: Vertical grows along Y axis
    bool vertOk = (rotBoard.boardArray[3][5] == 'V' && 
                   rotBoard.boardArray[4][5] == 'V' && 
                   rotBoard.boardArray[5][5] == 'V');
    addTestResult("Console: Vertical Ship", vertOk, "cells (5,3)-(5,5)");
    
    mvprintw(11, 2, "Result:");
    if (vertOk) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(12, 4, "[PASS] Vertical ship placement error");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(14, 2, "Press any key to continue...");
    refresh();
    getch();

    // ==========================================
    // Manual Test 4: Shot Validation
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 4/12: Shot Validation");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    BoardData shotBoard(10);
    shotBoard.addShip(1, 55, 3, 'S');  // Manually add a vertical ship at (5,5)
    
    mvprintw(4, 2, "Ship placed at (5,5) vertical, length 3 (cells 5,5 / 5,6 / 5,7)");
    mvprintw(6, 2, "Test MISS - Enter coordinates AWAY from ship:");
    mvprintw(7, 2, "  X (0-9): ");
    refresh();
    int missX;
    scanw("%d", &missX);
    
    mvprintw(8, 2, "  Y (0-9): ");
    refresh();
    int missY;
    scanw("%d", &missY);
    
    // Test Miss Logic: Should return 0 and mark cell as 'o'
    int missResult = shotBoard.receiveShot(missX, missY);
    bool isMiss = (missResult == 0 && shotBoard.boardArray[missY][missX] == 'o');
    
    char missCoords[100];
    sprintf(missCoords, "(%d,%d) -> %s", missX, missY, missResult == 0 ? "Miss" : "Hit");
    addTestResult("Console: Shot Miss", isMiss, missCoords);
    
    mvprintw(10, 2, "Result:");
    if (isMiss) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(11, 4, "[PASS] Shot at (%d,%d) = MISS (marked 'o')", missX, missY);
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(11, 4, "[FAIL] Shot at (%d,%d) should be miss!", missX, missY);
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Test Hit Logic: Should return 1 and mark cell as 'x'
    mvprintw(13, 2, "Test HIT - shooting at (5,5)...");
    int hitResult = shotBoard.receiveShot(5, 5);
    bool isHit = (hitResult == 1 && shotBoard.boardArray[5][5] == 'x');
    addTestResult("Console: Shot Hit", isHit, "(5,5) -> Hit");
    
    mvprintw(15, 2, "Result:");
    if (isHit) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(16, 4, "[PASS] Shot at (5,5) = HIT (marked 'x')");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(16, 4, "[FAIL] Shot at (5,5) should be hit!");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Test Duplicate Shot: Should return 0 (blocked)
    mvprintw(18, 2, "Test DUPLICATE - shooting at (5,5) again...");
    int dupResult = shotBoard.receiveShot(5, 5);
    bool isDup = (dupResult == 0);
    addTestResult("Console: Duplicate Block", isDup, "(5,5) -> Blocked");
    
    mvprintw(19, 2, "Result:");
    if (isDup) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(20, 4, "[PASS] Duplicate shot blocked (returns 0)");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(20, 4, "[FAIL] Duplicate shot should be blocked!");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(22, 2, "Press any key to continue...");
    refresh();
    getch();
    
    // Test Sinking Logic: Last shot on a ship should return 2
    clear();
    mvprintw(2, 2, "Continuing shot tests...");
    mvprintw(4, 2, "Test SINK - shooting at (5,6) and (5,7)...");
    shotBoard.receiveShot(5, 6); // Hit
    int sinkResult = shotBoard.receiveShot(5, 7); // Sink (last deck)
    bool isSunk = (sinkResult == 2);
    addTestResult("Console: Ship Sunk", isSunk, "(5,7) -> Sunk");
    
    mvprintw(6, 2, "Result:");
    if (isSunk) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(7, 4, "[PASS] Ship SUNK after all 3 hits (returns 2)");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(7, 4, "[FAIL] Ship should be sunk after 3 hits!");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(9, 2, "Press any key to continue...");
    refresh();
    getch();
    
    // ==========================================
    // Manual Test 5: Ship Counting
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 5/12: Ship Counting");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    // Initialize specific ship scenario
    BoardData countBoard(10);
    countBoard.addShip(1, 11, 4, 'A');  // 4-cell ship
    countBoard.addShip(1, 33, 3, 'B');  // 3-cell ship
    countBoard.addShip(1, 55, 2, 'C');  // 2-cell ship
    countBoard.addShip(0, 77, 1, 'D');  // 1-cell ship
    
    // Verify initial count
    int initialCount = countBoard.getRemainingShips();
    mvprintw(4, 2, "Placed 4 ships on board");
    mvprintw(5, 2, "Remaining ships: %d", initialCount);
    addTestResult("Console: Initial Count", initialCount == 4, "4 ships");
    
    mvprintw(7, 2, "Result:");
    if (initialCount == 4) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(8, 4, "[PASS] Ship count = 4");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(8, 4, "[FAIL] Ship count = %d (expected 4)", initialCount);
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Sink one ship and verify counts update
    mvprintw(10, 2, "Sinking first ship (4 hits at 1,1 / 1,2 / 1,3 / 1,4)...");
    countBoard.receiveShot(1, 1);
    countBoard.receiveShot(1, 2);
    countBoard.receiveShot(1, 3);
    countBoard.receiveShot(1, 4);
    
    int afterSink = countBoard.getRemainingShips();
    mvprintw(11, 2, "Remaining ships: %d", afterSink);
    addTestResult("Console: After Sink", afterSink == 3, "3 ships");
    
    int sunkCount = countBoard.getSunkCount();
    mvprintw(12, 2, "Sunk count: %d", sunkCount);
    addTestResult("Console: Sunk Count", sunkCount == 1, "1 sunk");
    
    mvprintw(14, 2, "Result:");
    if (afterSink == 3 && sunkCount == 1) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(15, 4, "[PASS] 3 ships remaining, 1 sunk");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(15, 4, "[FAIL] Remaining=%d (exp 3), Sunk=%d (exp 1)", afterSink, sunkCount);
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(17, 2, "Press any key to continue...");
    refresh();
    getch();
    
    // ==========================================
    // Manual Test 6: Volley System (Loop)
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 6/12: Volley System (Multiple Shots)");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    BoardData volleyBoard(10);
    volleyBoard.addShip(1, 11, 3, 'V');
    
    mvprintw(4, 2, "Ship placed at (1,1) vertical, length 3");
    mvprintw(5, 2, "How many shots in volley? (1-5): ");
    refresh();
    int volleyCount;
    scanw("%d", &volleyCount);
    
    int hits = 0, misses = 0;
    int currentLine = 7;
    
    // Loop to process multiple shots in sequence
    for (int i = 0; i < volleyCount; i++) {
        mvprintw(currentLine++, 2, "Shot %d - X: ", i+1);
        refresh();
        int vx;
        scanw("%d", &vx);
        
        mvprintw(currentLine++, 2, "Shot %d - Y: ", i+1);
        refresh();
        int vy;
        scanw("%d", &vy);
        
        int vResult = volleyBoard.receiveShot(vx, vy);
        if (vResult == 0) {
            misses++;
            mvprintw(currentLine++, 4, "-> MISS");
        } else if (vResult >= 1) {
            hits++;
            mvprintw(currentLine++, 4, "-> HIT%s", vResult == 2 ? " & SUNK!" : "");
        }
        refresh();
    }
    
    mvprintw(currentLine + 1, 2, "Volley result: %d hits, %d misses", hits, misses);
    addTestResult("Console: Volley System", hits + misses == volleyCount, 
                  std::to_string(volleyCount) + " shots");
    
    mvprintw(currentLine + 3, 2, "Result:");
    if (hits + misses == volleyCount) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(currentLine + 4, 4, "[PASS] All %d shots processed (%d hits, %d misses)", 
                 volleyCount, hits, misses);
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(currentLine + 4, 4, "[FAIL] Shot processing error");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(currentLine + 6, 2, "Press any key to continue...");
    refresh();
    getch();
    
    // ==========================================
    // Manual Test 7: AI Behavior & Targeting
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 7/12: AI Behavior");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    mvprintw(4, 2, "Test AI difficulty (0=Easy, 1=Smart): ");
    refresh();
    int aiDiff;
    scanw("%d", &aiDiff);
    
    AIDifficulty difficulty = (aiDiff == 1) ? SMART : EASY;
    AILogic testAI(difficulty, 10);
    
    mvprintw(6, 2, "AI making 5 shots:");
    std::set<std::pair<int,int>> aiShots;
    int aiLine = 7;
    
    // Check for shot uniqueness using a Set
    for (int i = 0; i < 5; i++) {
        AICoordinates shot = testAI.pickAttackCoordinates();
        aiShots.insert({shot.x, shot.y});
        mvprintw(aiLine++, 4, "Shot %d: (%d,%d)", i+1, shot.x, shot.y);
    }
    refresh();
    
    bool allUnique = (aiShots.size() == 5);
    addTestResult("Console: AI Unique Shots", allUnique, "5 unique");
    
    mvprintw(aiLine + 1, 2, "Result:");
    if (allUnique) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(aiLine + 2, 4, "[PASS] All 5 shots are unique");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(aiLine + 2, 4, "[FAIL] AI repeated coordinates!");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Test Smart AI "Targeting Mode" (Should shoot adjacent after hit)
    if (difficulty == SMART) {
        mvprintw(aiLine + 4, 2, "Testing Smart AI targeting after hit at (5,5)...");
        AILogic smartTest(SMART, 10);
        smartTest.recordShotResult(5, 5, true, false); // Record a hit without sinking
        AICoordinates nextShot = smartTest.pickAttackCoordinates();
        
        // Verify adjacency
        bool adjacent = (
            (nextShot.x == 4 && nextShot.y == 5) ||
            (nextShot.x == 6 && nextShot.y == 5) ||
            (nextShot.x == 5 && nextShot.y == 4) ||
            (nextShot.x == 5 && nextShot.y == 6)
        );
        
        mvprintw(aiLine + 5, 4, "After hit at (5,5), AI shot at (%d,%d)", nextShot.x, nextShot.y);
        addTestResult("Console: Smart AI Targeting", adjacent, "Adjacent cell");
        
        mvprintw(aiLine + 7, 2, "Result:");
        if (adjacent) {
            attron(COLOR_PAIR(2) | A_BOLD);
            mvprintw(aiLine + 8, 4, "[PASS] AI targets adjacent cell");
            attroff(COLOR_PAIR(2) | A_BOLD);
        } else {
            attron(COLOR_PAIR(4) | A_BOLD);
            mvprintw(aiLine + 8, 4, "[FAIL] AI should target adjacent cell!");
            attroff(COLOR_PAIR(4) | A_BOLD);
        }
    }
    
    mvprintw(aiLine + 10, 2, "Press any key to continue...");
    refresh();
    getch();
    
    // ==========================================
    // Manual Test 8: Game State Logic
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 8/12: Game State");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    GameState state;
    state.initialize(10, 3, true);
    
    mvprintw(4, 2, "Game initialized: 10x10 board, 3 shots per turn");
    addTestResult("Console: State Init", 
                  state.boardSize == 10 && state.shotsPerTurn == 3, 
                  "10x10, 3 shots");
    
    mvprintw(6, 2, "Result:");
    if (state.boardSize == 10 && state.shotsPerTurn == 3) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(7, 4, "[PASS] Game state initialized correctly");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(7, 4, "[FAIL] Game state initialization error");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Test: Game continues if hits < maxHits
    state.playerHits = 15;
    state.maxHits = 20;
    bool notOver = !state.isGameOver();
    mvprintw(9, 2, "Player hits: 15/20");
    mvprintw(10, 2, "Game over? %s", notOver ? "No" : "Yes");
    addTestResult("Console: Game Not Over", notOver, "15/20 hits");
    
    mvprintw(12, 2, "Result:");
    if (notOver) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(13, 4, "[PASS] Game continues (15/20 hits)");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(13, 4, "[FAIL] Game should not be over yet!");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Test: Game over if hits == maxHits
    state.playerHits = 20;
    bool isOver = state.isGameOver();
    mvprintw(15, 2, "Player hits: 20/20");
    mvprintw(16, 2, "Game over? %s", isOver ? "Yes" : "No");
    addTestResult("Console: Game Over", isOver, "20/20 hits");
    
    mvprintw(18, 2, "Result:");
    if (isOver) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(19, 4, "[PASS] Game over detected (20/20 hits)");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(19, 4, "[FAIL] Game should be over!");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(21, 2, "Press any key to continue...");
    refresh();
    getch();
    
    // ==========================================
    // Manual Test 9: Ship Configuration
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 9/12: Ship Configuration");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    // Check config for 10x10 board
    ShipConfiguration config10 = getShipConfig(10);
    int total10 = config10.fourDeck + config10.threeDeck + 
                  config10.twoDeck + config10.oneDeck;
    mvprintw(4, 2, "10x10 board configuration:");
    mvprintw(5, 4, "Total ships: %d", total10);
    mvprintw(6, 4, "Shots per turn: %d", config10.shotsPerTurn);
    addTestResult("Console: 10x10 Config", total10 == 10, "10 ships");
    
    mvprintw(8, 2, "Result:");
    if (total10 == 10) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(9, 4, "[PASS] 10x10 has 10 ships, %d shots/turn", config10.shotsPerTurn);
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(9, 4, "[FAIL] 10x10 should have 10 ships (has %d)", total10);
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Check config for 15x15 board (should have more ships)
    ShipConfiguration config15 = getShipConfig(15);
    int total15 = config15.fourDeck + config15.threeDeck + 
                  config15.twoDeck + config15.oneDeck;
    mvprintw(11, 2, "15x15 board configuration:");
    mvprintw(12, 4, "Total ships: %d", total15);
    mvprintw(13, 4, "Shots per turn: %d", config15.shotsPerTurn);
    addTestResult("Console: 15x15 Config", total15 > total10, 
                  std::to_string(total15) + " ships");
    
    mvprintw(15, 2, "Result:");
    if (total15 > total10) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(16, 4, "[PASS] 15x15 has more ships (%d > %d)", total15, total10);
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(16, 4, "[FAIL] 15x15 should have more ships than 10x10!");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(18, 2, "Press any key to continue...");
    refresh();
    getch();
    
    // ==========================================
    // Manual Test 10: Random Board Generation
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 10/12: Board Generation");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    BoardData genBoard(10);
    std::vector<GamePiece> pieces;
    GameLogic::initializeGamePieces(genBoard, pieces);
    
    mvprintw(4, 2, "Initializing game pieces for 10x10 board...");
    mvprintw(5, 2, "Pieces created: %d", (int)pieces.size());
    addTestResult("Console: Pieces Init", !pieces.empty(), 
                  std::to_string(pieces.size()) + " pieces");
    refresh();
    SLEEP_MS(500);
    
    mvprintw(7, 2, "Generating board placement...");
    refresh();
    GameLogic::generateBoardPlacement(genBoard, pieces);
    SLEEP_MS(500);
    
    // Count cells occupied by ships ('A'-'Z')
    int shipCells = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (genBoard.boardArray[i][j] >= 'A' && genBoard.boardArray[i][j] <= 'Z') {
                shipCells++;
            }
        }
    }
    
    int expectedCells = getTotalShipCells(10);
    mvprintw(8, 2, "Ship cells placed: %d/%d", shipCells, expectedCells);
    addTestResult("Console: Generation", shipCells == expectedCells, 
                  std::to_string(shipCells) + "/" + std::to_string(expectedCells));
    
    mvprintw(10, 2, "Result:");
    if (shipCells == expectedCells) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(11, 4, "[PASS] All %d ship cells placed correctly", expectedCells);
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(11, 4, "[FAIL] Expected %d cells, got %d", expectedCells, shipCells);
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Test multiple generations to ensure stability
    mvprintw(13, 2, "Testing multiple board generations (10 times)...");
    refresh();
    
    bool multipleGens = true;
    for (int i = 0; i < 10; i++) {
        genBoard.clear();
        GameLogic::generateBoardPlacement(genBoard, pieces);
        if (genBoard.getRemainingShips() != (int)pieces.size()) {
            multipleGens = false;
            break;
        }
        mvprintw(14, 4, "Generation %d/10 - %d ships placed", 
                 i+1, genBoard.getRemainingShips());
        refresh();
        SLEEP_MS(100);
    }
    
    addTestResult("Console: Multiple Gens", multipleGens, "10 successful");
    
    mvprintw(16, 2, "Result:");
    if (multipleGens) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(17, 4, "[PASS] All 10 generations successful");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(17, 4, "[FAIL] Generation failed during multiple runs");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(19, 2, "Press any key to continue...");
    refresh();
    getch();
    
    // ==========================================
    // Manual Test 11: Coordinate System
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 11/12: Coordinate System");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    BoardData coordBoard(10);
    coordBoard.addShip(1, 55, 3, 'T');
    
    mvprintw(4, 2, "Ship placed at position 55 (should be x=5, y=5)");
    mvprintw(5, 2, "Vertical orientation, length 3");
    
    // Verify 1D index to 2D coordinate mapping
    bool correctPos = (coordBoard.boardArray[5][5] == 'T');
    addTestResult("Console: Position Calc", correctPos, "pos 55 = (5,5)");
    
    mvprintw(7, 2, "Result:");
    if (correctPos) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(8, 4, "[PASS] Position 55 correctly maps to (5,5)");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(8, 4, "[FAIL] Position mapping error");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Verify retrieval of all cells occupied by a specific ship
    auto cells = coordBoard.getShipOccupiedCells(5, 5);
    bool correctCells = (cells.size() == 3);
    
    if (correctCells) {
        bool coordsMatch = true;
        for (size_t i = 0; i < cells.size(); i++) {
            if (cells[i].first != 5 || cells[i].second != 5 + (int)i) {
                coordsMatch = false;
            }
        }
        correctCells = coordsMatch;
    }
    
    mvprintw(10, 2, "Testing getShipOccupiedCells(5,5):");
    mvprintw(11, 4, "Cells returned: %d (expected 3)", (int)cells.size());
    
    if (cells.size() == 3) {
        mvprintw(12, 4, "Cell 1: (%d,%d)", cells[0].first, cells[0].second);
        mvprintw(13, 4, "Cell 2: (%d,%d)", cells[1].first, cells[1].second);
        mvprintw(14, 4, "Cell 3: (%d,%d)", cells[2].first, cells[2].second);
    }
    
    addTestResult("Console: Ship Cells", correctCells, 
                  std::to_string(cells.size()) + " cells verified");
    
    mvprintw(16, 2, "Result:");
    if (correctCells) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(17, 4, "[PASS] All ship cells correctly identified");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(17, 4, "[FAIL] Ship cell retrieval error");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(19, 2, "Press any key to continue...");
    refresh();
    getch();
    
    // ==========================================
    // Manual Test 12: Smart AI Parity Strategy
    // ==========================================
    clear();
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, 2, "TEST 12/12: Smart AI Parity Strategy");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    mvprintw(4, 2, "Testing Smart AI checkerboard pattern...");
    mvprintw(5, 2, "Simulating 50 shots without any hits (hunt mode)");
    refresh();
    
    AILogic smartParityAI(SMART, 10);
    std::set<std::pair<int,int>> pCoords;
    
    // Simulate 50 AI shots to analyze distribution
    for (int k = 0; k < 50; k++) {
        AICoordinates c = smartParityAI.pickAttackCoordinates();
        pCoords.insert({c.x, c.y});
    }
    
    // Parity check: Smart AI should prioritize specific cells (checkerboard pattern)
    // Sum of x+y is even (or odd) for optimized search
    int parityMatches = 0;
    for (const auto& p : pCoords) {
        if ((p.first + p.second) % 2 == 0) parityMatches++;
    }
    
    mvprintw(7, 2, "Results:");
    mvprintw(8, 4, "Total unique shots: %d/50", (int)pCoords.size());
    mvprintw(9, 4, "Shots matching parity pattern: %d", parityMatches);
    mvprintw(10, 4, "Parity percentage: %d%%", (parityMatches * 100) / 50);
    
    bool parityOk = (parityMatches >= 30);
    
    addTestResult("Console: Smart AI Parity", parityOk, 
                  std::to_string(parityMatches) + "/50 parity shots");
    
    mvprintw(12, 2, "Result:");
    if (parityOk) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(13, 4, "[PASS] Smart AI uses checkerboard strategy (%d%% parity)", 
                 (parityMatches * 100) / 50);
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(13, 4, "[FAIL] Smart AI not following parity pattern (only %d%%)",
                 (parityMatches * 100) / 50);
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    // Test immediate targeting of adjacent cells after a hit
    mvprintw(15, 2, "Testing targeting mode after hit...");
    AILogic targetTest(SMART, 10);
    targetTest.recordShotResult(5, 5, true, false); 
    
    AICoordinates nextShot = targetTest.pickAttackCoordinates();
    bool adjacentOk = (
        (nextShot.x == 4 && nextShot.y == 5) ||
        (nextShot.x == 6 && nextShot.y == 5) ||
        (nextShot.x == 5 && nextShot.y == 4) ||
        (nextShot.x == 5 && nextShot.y == 6)
    );
    
    mvprintw(16, 4, "After hit at (5,5), next shot: (%d,%d)", nextShot.x, nextShot.y);
    addTestResult("Console: Target Mode", adjacentOk, "Adjacent targeting");
    
    mvprintw(18, 2, "Result:");
    if (adjacentOk) {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(19, 4, "[PASS] AI correctly targets adjacent cells after hit");
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(19, 4, "[FAIL] AI should target adjacent cells after hit");
        attroff(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(21, 2, "Press any key to finish manual tests...");
    refresh();
    getch();
    
    // ==========================================
    // Final Summary
    // ==========================================
    clear();
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(2, 2, "===== ALL MANUAL TESTS COMPLETE! =====");
    attroff(COLOR_PAIR(2) | A_BOLD);
    
    mvprintw(4, 2, "All 12 test categories have been executed.");
    mvprintw(5, 2, "Results have been recorded to test_results.txt");
    
    attron(COLOR_PAIR(3));
    mvprintw(7, 2, "Press any key to return to test menu...");
    attroff(COLOR_PAIR(3));
    
    refresh();
    noecho();
    curs_set(0); // Hide cursor before exiting
    getch();
}

static void runFileTests() {
    clear();
    
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(1, 2, "=== FILE-BASED TESTS ===");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    mvprintw(3, 2, "Reading from tests/SeaBattle_1_test.dat...");
    refresh();
    SLEEP_MS(1000);
    
    // Open the external test data file
    std::ifstream inFile("tests/SeaBattle_1_test.dat");
    if (!inFile.is_open()) {
        mvprintw(5, 2, "ERROR: Cannot open tests/SeaBattle_1_test.dat");
        addTestResult("File Tests", false, "File not found");
        mvprintw(7, 2, "Press any key...");
        refresh();
        getch();
        return;
    }
    
    std::string line;
    int testNum = 0;
    int displayY = 5;
    
    // Category tracking variables
    std::string currentCategory = "";
    int categoryTests = 0;
    int categoryPassed = 0;
    int globalCategoryNumber = 0;
    
    // Board logic for shot testing
    BoardData* shotBoard = nullptr;
    int shotSequence = 0;
    bool needNewShotBoard = true;
    
    // Statistics
    int totalBoardTests = 0;
    int totalShipTests = 0;
    int totalShotTests = 0;
    int totalTests = 0;
    int totalPassed = 0;
    
    // Loop through the file line by line
    while (std::getline(inFile, line)) {
        if (line.empty()) {
            continue;
        }
        
        // Handle Comments and Category Headers (Lines starting with #)
        if (line[0] == '#') {
            if (line.find("CATEGORY") != std::string::npos) {
                // If ending a previous category, print its summary
                if (!currentCategory.empty() && categoryTests > 0) {
                    displayY++;
                    attron(COLOR_PAIR(categoryPassed == categoryTests ? 2 : 6));
                    mvprintw(displayY++, 4, "Category %d Summary: %d/%d passed (%.1f%%)", 
                             globalCategoryNumber,
                             categoryPassed, categoryTests,
                             categoryTests > 0 ? (categoryPassed * 100.0 / categoryTests) : 0);
                    attroff(COLOR_PAIR(categoryPassed == categoryTests ? 2 : 6));
                    displayY++;
                    
                    if (outputFile.is_open()) {
                        outputFile << "  Category " << globalCategoryNumber << " Summary: " 
                                   << categoryPassed << "/" << categoryTests << " passed\n\n";
                    }
                }
                
                // Parse new category name
                size_t catPos = line.find("CATEGORY");
                if (catPos != std::string::npos) {
                    currentCategory = line.substr(catPos);
                    size_t start = currentCategory.find_first_not_of("# ");
                    if (start != std::string::npos) {
                        currentCategory = currentCategory.substr(start);
                    }
                    globalCategoryNumber++;
                }
                
                // Reset category counters
                categoryTests = 0;
                categoryPassed = 0;
                
                // Reset board for shot tests if entering a combat/shot category
                if (currentCategory.find("SHOT") != std::string::npos || 
                    currentCategory.find("COMBAT") != std::string::npos ||
                    currentCategory.find("VOLLEY") != std::string::npos) {
                    needNewShotBoard = true;
                    if (shotBoard) {
                        delete shotBoard;
                        shotBoard = nullptr;
                    }
                    shotSequence = 0;
                }
                
                // Print Category Header
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw(displayY++, 2, ">>> %s", currentCategory.c_str());
                attroff(COLOR_PAIR(3) | A_BOLD);
                
                if (outputFile.is_open()) {
                    outputFile << "\n=== " << currentCategory << " ===\n";
                }
                
                refresh();
            }
            continue;
        }
        
        // Parse Test Command
        std::istringstream iss(line);
        std::string testType;
        iss >> testType;
        
        // --- TEST TYPE: BOARD INITIALIZATION ---
        if (testType == "BOARD_INIT") {
            int size;
            iss >> size;
            
            // Perform logic check: Create board and verify it is empty (water)
            BoardData board(size);
            bool allWater = true;
            for (int i = 0; i < size && allWater; i++) {
                for (int j = 0; j < size && allWater; j++) {
                    if (board.boardArray[i][j] != 'w') {
                        allWater = false;
                    }
                }
            }
            
            bool validSize = (size >= MIN_BOARD_SIZE && size <= MAX_BOARD_SIZE);
            bool testPassed = allWater && validSize && board.boardSize == size;
            
            char msg[100];
            sprintf(msg, "%dx%d", size, size);
            addTestResult("File: Board Init", testPassed, msg);
            
            // UI Output for result
            if (testPassed) {
                attron(COLOR_PAIR(2));
                categoryPassed++;
                totalPassed++;
            } else {
                attron(COLOR_PAIR(4));
            }
            mvprintw(displayY++, 4, "[%s] Board %dx%d: init=%s water=%s", 
                     testPassed ? "PASS" : "FAIL", 
                     size, size,
                     validSize ? "OK" : "BAD",
                     allWater ? "OK" : "BAD");
            attroff(COLOR_PAIR(2));
            attroff(COLOR_PAIR(4));
            
            categoryTests++;
            totalBoardTests++;
            totalTests++;
            
        } 
        // --- TEST TYPE: SHIP PLACEMENT ---
        else if (testType == "SHIP_PLACE") {
            int size, x, y, orient, len;
            char symbol;
            iss >> size >> x >> y >> orient >> len >> symbol;
            
            BoardData board(size);
            // Check if placement is valid according to game rules
            bool valid = GameLogic::isValidShipPlacement(board, x, y, orient, len);
            bool placed = false;
            
            // If valid, attempt to place and verify grid update
            if (valid) {
                GameLogic::placeShip(board, x, y, orient, len, symbol);
                placed = (board.boardArray[y][x] == symbol);
            }
            
            bool testPassed = valid && placed;
            
            char msg[200];
            sprintf(msg, "(%d,%d) %c len=%d %s", 
                    x, y, symbol, len, orient == 0 ? "H" : "V");
            addTestResult("File: Ship Place", testPassed, msg);
            
            if (testPassed) {
                attron(COLOR_PAIR(2));
                categoryPassed++;
                totalPassed++;
            } else {
                attron(COLOR_PAIR(4));
            }
            mvprintw(displayY++, 4, "[%s] Ship '%c': (%d,%d) %s L=%d", 
                     testPassed ? "PASS" : "FAIL",
                     symbol, x, y,
                     orient == 0 ? "→" : "↓",
                     len);
            attroff(COLOR_PAIR(2));
            attroff(COLOR_PAIR(4));
            
            categoryTests++;
            totalShipTests++;
            totalTests++;
            
        } 
        // --- TEST TYPE: SHOOTING MECHANICS ---
        else if (testType == "SHOT") {
            int x, y;
            int expectedResult;
            iss >> x >> y >> expectedResult;
            
            // Initialize a board with a target ship if needed
            if (needNewShotBoard || shotBoard == nullptr) {
                if (shotBoard) delete shotBoard;
                shotBoard = new BoardData(10);
                shotBoard->addShip(1, 55, 3, 'S'); // Add dummy ship for testing
                shotSequence = 0;
                needNewShotBoard = false;
            }
            
            // Execute shot and compare result
            int result = shotBoard->receiveShot(x, y);
            bool correct = (result == expectedResult);
            shotSequence++;
            
            const char* resultStr;
            const char* resultSymbol;
            
            switch(result) {
                case 0: resultStr = "Miss"; resultSymbol = "○"; break;
                case 1: resultStr = "Hit "; resultSymbol = "×"; break;
                case 2: resultStr = "Sunk"; resultSymbol = "⊗"; break;
                default: resultStr = "????"; resultSymbol = "?"; break;
            }
            
            char msg[200];
            sprintf(msg, "(%d,%d) %s", x, y, resultStr);
            addTestResult("File: Shot", correct, msg);
            
            if (correct) {
                attron(COLOR_PAIR(2));
                categoryPassed++;
                totalPassed++;
            } else {
                attron(COLOR_PAIR(4));
            }
            mvprintw(displayY++, 4, "[%s] Shot #%-2d: (%d,%d) %s %s%s", 
                     correct ? "PASS" : "FAIL",
                     shotSequence, x, y, 
                     resultSymbol,
                     resultStr,
                     correct ? "" : " [WRONG]");
            attroff(COLOR_PAIR(2));
            attroff(COLOR_PAIR(4));
            
            categoryTests++;
            totalShotTests++;
            totalTests++;
        }
        
        refresh();
        SLEEP_MS(50); // Small delay for visual effect
        testNum++;
        
        // Handle screen scrolling/paging if tests exceed screen height
        if (displayY > 21) {
            attron(COLOR_PAIR(6));
            mvprintw(23, 2, "[Test %d/%d] [Passed: %d] Press any key...", 
                     testNum, totalTests, totalPassed);
            attroff(COLOR_PAIR(6));
            refresh();
            getch();
            clear();
            displayY = 3;
            attron(COLOR_PAIR(5) | A_BOLD);
            mvprintw(1, 2, "=== FILE-BASED TESTS (continued) ===");
            attroff(COLOR_PAIR(5) | A_BOLD);
            
            if (!currentCategory.empty()) {
                attron(COLOR_PAIR(3));
                mvprintw(2, 2, ">>> %s", currentCategory.c_str());
                attroff(COLOR_PAIR(3));
            }
        }
    }
    
    // Clean up
    if (shotBoard) {
        delete shotBoard;
        shotBoard = nullptr;
    }
    
    inFile.close();
    
    // Print summary for the final category
    if (!currentCategory.empty() && categoryTests > 0) {
        displayY++;
        attron(COLOR_PAIR(categoryPassed == categoryTests ? 2 : 6));
        mvprintw(displayY++, 4, "Category %d Summary: %d/%d passed (%.1f%%)", 
                 globalCategoryNumber,
                 categoryPassed, categoryTests,
                 categoryTests > 0 ? (categoryPassed * 100.0 / categoryTests) : 0);
        attroff(COLOR_PAIR(categoryPassed == categoryTests ? 2 : 6));
        
        if (outputFile.is_open()) {
            outputFile << "  Category " << globalCategoryNumber << " Summary: " 
                       << categoryPassed << "/" << categoryTests << " passed\n\n";
        }
    }
    
    // Final Global Report for File Tests
    displayY += 2;
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(displayY++, 2, "==============================");
    mvprintw(displayY++, 2, "  FILE TEST FINAL RESULTS");
    mvprintw(displayY++, 2, "==============================");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    attron(COLOR_PAIR(1));
    mvprintw(displayY++, 4, "Total Categories: %d", globalCategoryNumber);
    mvprintw(displayY++, 4, "Total Tests: %d", totalTests);
    displayY++;
    mvprintw(displayY++, 4, "Board Init Tests: %d", totalBoardTests);
    mvprintw(displayY++, 4, "Ship Place Tests: %d", totalShipTests);
    mvprintw(displayY++, 4, "Shot Tests: %d", totalShotTests);
    displayY++;
    attroff(COLOR_PAIR(1));
    
    double successRate = totalTests > 0 ? (totalPassed * 100.0 / totalTests) : 0;
    
    if (successRate >= 95.0) {
        attron(COLOR_PAIR(2) | A_BOLD);
    } else if (successRate >= 80.0) {
        attron(COLOR_PAIR(6) | A_BOLD);
    } else {
        attron(COLOR_PAIR(4) | A_BOLD);
    }
    
    mvprintw(displayY++, 4, "PASSED: %d/%d (%.1f%%)", 
             totalPassed, totalTests, successRate);
    mvprintw(displayY++, 4, "FAILED: %d/%d (%.1f%%)", 
             totalTests - totalPassed, totalTests, 100.0 - successRate);
    
    attroff(COLOR_PAIR(2) | A_BOLD);
    attroff(COLOR_PAIR(4) | A_BOLD);
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    displayY += 2;
    attron(COLOR_PAIR(3));
    mvprintw(displayY++, 2, "All file-based tests complete!");
    mvprintw(displayY++, 2, "Detailed results: test_results.txt");
    mvprintw(displayY + 1, 2, "Press any key to return to menu...");
    attroff(COLOR_PAIR(3));
    
    if (outputFile.is_open()) {
        outputFile << "\n==============================\n";
        outputFile << "FILE TEST FINAL RESULTS\n";
        outputFile << "==============================\n";
        outputFile << "Total Categories: " << globalCategoryNumber << "\n";
        outputFile << "Total Tests: " << totalTests << "\n";
        outputFile << "  Board Init: " << totalBoardTests << "\n";
        outputFile << "  Ship Place: " << totalShipTests << "\n";
        outputFile << "  Shot Tests: " << totalShotTests << "\n";
        outputFile << "\nPASSED: " << totalPassed << "/" << totalTests 
                   << " (" << successRate << "%)\n";
        outputFile << "FAILED: " << (totalTests - totalPassed) << "/" << totalTests 
                   << " (" << (100.0 - successRate) << "%)\n";
        outputFile << "==============================\n";
    }
    
    refresh();
    getch();
}


void runDebugTests() {
    bool inTestMenu = true;
    
    // Main Test Menu Loop
    while (inTestMenu) {
        clear();
        testResults.clear();
        
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(2, 2, "=== TEST MODE SELECTION ===");
        attroff(COLOR_PAIR(5) | A_BOLD);
        
        attron(COLOR_PAIR(1));
        mvprintw(4, 2, "0) Return to Main Menu");
        mvprintw(5, 2, "1) Automatic Tests (comprehensive pre-programmed)");
        mvprintw(6, 2, "2) Manual Console Input Tests");
        mvprintw(7, 2, "3) File-based Tests (tests/SeaBattle_1_test.dat)");
        mvprintw(8, 2, "4) All Tests");
        mvprintw(10, 2, "Select mode (0-4): ");
        attroff(COLOR_PAIR(1));
        
        refresh();
        
        int mode = getch();
        
        // Return to main game
        if (mode == '0') {
            inTestMenu = false;
            clear();
            continue;
        }
        
        // Initialize log file
        outputFile.open("test_results.txt");
        if (outputFile.is_open()) {
            time_t now = time(0);
            outputFile << "========== SEABATTLE COMPREHENSIVE TESTS ==========\n";
            outputFile << "Test Date: " << ctime(&now);
            outputFile << "===================================================\n\n";
        }
        
        clear();
        mvprintw(2, 2, "Running tests...");
        refresh();
        
        // --- AUTOMATIC TESTS EXECUTION ---
        if (mode == '1' || mode == '4') {
            if (outputFile.is_open()) {
                outputFile << "--- AUTOMATIC TESTS ---\n";
                outputFile << "Running all 12 test categories...\n\n";
            }
            
            clear();
            attron(COLOR_PAIR(5) | A_BOLD);
            mvprintw(1, 2, "=== RUNNING AUTOMATIC TESTS ===");
            attroff(COLOR_PAIR(5) | A_BOLD);
            
            int testY = 3;
            
            // Execute each test category individually
            mvprintw(testY++, 2, "Running Category 1: Board Size Validation...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 1: Board Size Validation\n";
            testBoardSizeValidation();
            SLEEP_MS(100);
            
            mvprintw(testY++, 2, "Running Category 2: Ship Placement Validation...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 2: Ship Placement Validation\n";
            testShipPlacementValidation();
            SLEEP_MS(100);
            
            mvprintw(testY++, 2, "Running Category 3: Ship Rotation...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 3: Ship Rotation\n";
            testShipRotation();
            SLEEP_MS(100);
            
            mvprintw(testY++, 2, "Running Category 4: Shot Validation...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 4: Shot Validation\n";
            testShotValidation();
            SLEEP_MS(100);
            
            mvprintw(testY++, 2, "Running Category 5: Ship Counting...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 5: Ship Counting\n";
            testShipCounting();
            SLEEP_MS(100);
            
            mvprintw(testY++, 2, "Running Category 6: Volley System...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 6: Volley System\n";
            testVolleySystem();
            SLEEP_MS(100);
            
            mvprintw(testY++, 2, "Running Category 7: Easy AI...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 7: Easy AI\n";
            testEasyAI();
            SLEEP_MS(100);
            
            mvprintw(testY++, 2, "Running Category 8: Smart AI...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 8: Smart AI\n";
            testSmartAI();
            SLEEP_MS(100);
            
            mvprintw(testY++, 2, "Running Category 9: Game State...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 9: Game State\n";
            testGameState();
            SLEEP_MS(100);
            
            mvprintw(testY++, 2, "Running Category 10: Ship Configuration...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 10: Ship Configuration\n";
            testShipConfiguration();
            SLEEP_MS(100);
            
            mvprintw(testY++, 2, "Running Category 11: Board Generation...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 11: Board Generation\n";
            testBoardGeneration();
            SLEEP_MS(100);
        
            mvprintw(testY++, 2, "Running Category 12: Coordinate System...");
            refresh();
            if (outputFile.is_open()) outputFile << "Category 12: Coordinate System\n";
            testCoordinateSystem();
            SLEEP_MS(100);
            
            mvprintw(testY + 2, 2, "All automatic tests completed!");
            mvprintw(testY + 3, 2, "Press any key to see results...");
            refresh();
            getch();
        }
        
        // --- MANUAL TESTS EXECUTION ---
        if (mode == '2' || mode == '4') {
            if (outputFile.is_open()) {
                outputFile << "\n--- CONSOLE MANUAL TESTS ---\n";
            }
            runManualTests();
        }
        
        // --- FILE TESTS EXECUTION ---
        if (mode == '3' || mode == '4') {
            if (outputFile.is_open()) {
                outputFile << "\n--- FILE-BASED TESTS ---\n";
            }
            runFileTests();
        }
        
        // Invalid input handling
        if (mode < '0' || mode > '4') {
            attron(COLOR_PAIR(4));
            mvprintw(13, 2, "Invalid selection. Press any key to try again...");
            attroff(COLOR_PAIR(4));
            refresh();
            getch();
            if (outputFile.is_open()) outputFile.close();
            continue;
        }
        
        // --- DISPLAYING RESULTS SUMMARY ---
        if (mode >= '1' && mode <= '4') {
            clear();
            
            attron(COLOR_PAIR(5) | A_BOLD);
            mvprintw(1, 2, "===== TEST RESULTS SUMMARY =====");
            attroff(COLOR_PAIR(5) | A_BOLD);
            
            int currentY = 3;
            int passed = 0, failed = 0;
            
            // List all results captured in the global vector
            for (const auto& result : testResults) {
                displayTestResult(3, currentY, result);
                if (result.passed) passed++;
                else failed++;
            }
            
            currentY += 2;
            attron(COLOR_PAIR(1));
            mvprintw(currentY++, 2, "============================================");
            
            char summary[100];
            sprintf(summary, "Total: %d | Passed: %d | Failed: %d", 
                    (int)testResults.size(), passed, failed);
            
            if (failed == 0) {
                attron(COLOR_PAIR(2) | A_BOLD);
            } else {
                attron(COLOR_PAIR(4) | A_BOLD);
            }
            mvprintw(currentY++, 2, "%s", summary);
            attroff(COLOR_PAIR(2) | A_BOLD);
            attroff(COLOR_PAIR(4) | A_BOLD);
            
            int successRate = testResults.empty() ? 0 : (passed * 100) / testResults.size();
            sprintf(summary, "Success Rate: %d%%", successRate);
            mvprintw(currentY++, 2, "%s", summary);
            
            // Write final summary to file
            if (outputFile.is_open()) {
                outputFile << "\n============================================\n";
                outputFile << "Total: " << testResults.size() << " | ";
                outputFile << "Passed: " << passed << " | ";
                outputFile << "Failed: " << failed << "\n";
                outputFile << "Success Rate: " << successRate << "%\n";
                outputFile << "============================================\n";
                outputFile.close();
            }
            
            currentY += 2;
            attron(COLOR_PAIR(6));
            mvprintw(currentY++, 2, "Results saved to: test_results.txt");
            
            attron(COLOR_PAIR(3));
            mvprintw(currentY + 1, 2, "Press any key to return to test menu...");
            attroff(COLOR_PAIR(3));
            
            refresh();
            getch();
        }
    }
    
    clear();
}
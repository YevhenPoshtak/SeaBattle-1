#include "game_board.hpp"
#include "board_layout.hpp"
#include <chrono>
#include <cstring>
#include <algorithm>
#include <sstream>

using namespace std;

std::string generateShipSymbol(int shipId) {
    if (shipId < 26) {
        return std::string(1, 'a' + shipId);
    } else {
        int first = (shipId / 26) - 1;
        int second = shipId % 26;
        return std::string(1, 'A' + first) + std::string(1, 'a' + second);
    }
}

bool shipSymbolsMatch(const std::string& symbol1, char boardChar) {
    if (symbol1.length() == 1) {
        return tolower(symbol1[0]) == tolower(boardChar);
    }
    return tolower(symbol1[0]) == tolower(boardChar);
}

Gameboard::Gameboard() {
    boardSize = ::getBoardSize();
    boardArray.resize(boardSize, std::vector<char>(boardSize, 'w'));
    missCount = 0; 
}

Gameboard::Gameboard(int size) {
    boardSize = size;
    boardArray.resize(size, std::vector<char>(size, 'w'));
    missCount = 0;
}

void Gameboard::setBoardSize(int size) {
    boardSize = size;
    resizeBoard(size);
}

void Gameboard::resizeBoard(int newSize) {
    boardArray.clear();
    boardArray.resize(newSize, std::vector<char>(newSize, 'w'));
    boardSize = newSize;
}

int Gameboard::generateRandomBoard(bool host) {
    setIsHost(host);
    initializeBoard();
    initializeGamePieces();
    generateBoardPlacement();
    buildShipCellMap(); 
    displayGeneratedBoard();

    bool isDeciding = true;
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    while (isDeciding) {
        switch (getch()) {
            case 'y':
            case 'Y':
                isDeciding = false;
                break;
            case 'n':
            case 'N':
                initializeBoard();
                generateBoardPlacement();
                displayGeneratedBoard();
                break;
            case 'm':
            case 'M':
                return 0;
            default:
                move(maxY - 3, 1);
                printw("Invalid Input please enter y/n to accept board\n");
                refresh();
                break;
        }
    }
    return 1;
}

int Gameboard::generateRandomBoardAuto(bool host) {
    setIsHost(host);
    initializeBoard();
    initializeGamePieces();
    generateBoardPlacement();
    buildShipCellMap(); 
    return 1;
}

void Gameboard::displayGeneratedBoard() {
    int size = getBoardSize();
    BoardLayout layout = calculateBoardLayout(size);
    int boardWidth = size * 4 + 8;
    
    clear();
    move(layout.startY, layout.board1StartX);
    
    const char* title = (size >= 20) ? "You" : (size >= 15) ? "Your" : "Your Board";
    int titleLen = strlen(title);
    int leftPad = (boardWidth - titleLen) / 2;
    int rightPad = boardWidth - leftPad - titleLen;
    
    for (int i = 0; i < leftPad; i++) printw("-");
    printw("%s", title);
    for (int i = 0; i < rightPad; i++) printw("-");
    printw("\n");
    
    move(layout.startY + 1, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("_");
    printw("\n");
    
    move(layout.startY + 2, layout.board1StartX);
    printw("|  | A |");
    for (int i = 1; i < size; i++) {
        printw(" %c |", 'A' + i);
    }
    printw("\n");

    for (int i = 0; i < size; i++) {
        move(layout.startY + 3 + i, layout.board1StartX);
        printw("|%2d|", i + 1);
        for (int j = 0; j < size; j++) {
            printw("   |");
        }
    }
    
    move(layout.startY + 3 + size, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("-");
    printw("\n");
    
    attron(A_UNDERLINE);
    mvprintw(1, 1, "instructions");
    attroff(A_UNDERLINE);
    
    mvprintw(2, 1, "y - accept board placement");
    mvprintw(3, 1, "n - randomize board");
    mvprintw(4, 1, "m - switch to manual mode");

    refresh();
    
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (boardArray[i][j] != 'w') {
                move(layout.startY + 3 + i, layout.board1StartX + 5 + (j * 4));
                addch(boardArray[i][j]);
            }
        }
    }
    refresh();
}

int Gameboard::generateManualBoard() {
    initializeBoard();
    initializeGamePieces();
    displayEmptyBoard(); 

    bool isPlacing = true;
    int size = getBoardSize();
    BoardLayout layout = calculateBoardLayout(size);
    
    int cursor_x = layout.board1StartX + 5;
    int cursor_y = layout.startY + 3;
    int ship_to_place = 0;
    int orientation = 0; 
    
    int maxCursorX = layout.board1StartX + 5 + (size - 1) * 4;
    int maxCursorY = layout.startY + 3 + size - 1;
    
    while (isPlacing) {
        if (ship_to_place >= (int)piece_array.size()) {
            if (!acceptGameboard()) {
                ship_to_place = 0;
                orientation = 0;
                cursor_y = layout.startY + 3;
                cursor_x = layout.board1StartX + 5;
                initializeBoard();
                displayEmptyBoard();
                continue;
            } else {
                isPlacing = false;
                break;
            }
        }
        
        displayEmptyBoard();

        Game_Piece ship = piece_array[ship_to_place];
        bool isValidPlacement = highlightShip(cursor_x, cursor_y, ship.Get_Piece_Length(), orientation,
                                              ship.Get_Piece_Symbol());

        refresh();

        switch (getch()) {
            case KEY_LEFT:
            case 'a':
            case 'A':
                if (orientation == 0) {
                    if ((cursor_x - (4 * (ship.Get_Piece_Length() - 1))) > layout.board1StartX + 5) {
                        cursor_x -= 4;
                    }
                } else {
                    if (cursor_x > layout.board1StartX + 5) {
                        cursor_x -= 4;
                    }
                }
                break;
            case KEY_RIGHT:
            case 'd':
            case 'D':
                if (cursor_x < maxCursorX) {
                    cursor_x += 4;
                }
                break;
            case KEY_UP:
            case 'w':
            case 'W':
                if (orientation == 1) {
                    if ((cursor_y - (ship.Get_Piece_Length() - 1)) > layout.startY + 3) {
                        cursor_y -= 1;
                    }
                } else {
                    if (cursor_y > layout.startY + 3) {
                        cursor_y -= 1;
                    }
                }
                break;
            case KEY_DOWN:
            case 's':
            case 'S':
                if (cursor_y < maxCursorY) {
                    cursor_y += 1;
                }
                break;
            case 'r':
            case 'R':
                if (orientation == 0) {
                    if ((cursor_y - (ship.Get_Piece_Length() - 1)) >= layout.startY + 3) {
                        orientation = 1;
                    }
                } else {
                    if ((cursor_x - (4 * (ship.Get_Piece_Length() - 1))) >= layout.board1StartX + 5) {
                        orientation = 0;
                    }
                }
                break;
            case 'g':
            case 'G':
                return 0; 
            case ' ':
            case 10: 
                if (isValidPlacement) {
                    placeGamePiece(cursor_x, cursor_y, orientation, ship.Get_Piece_Length(), ship.Get_Piece_Symbol());
                    ship_to_place++;
                    cursor_y = layout.startY + 3;
                    cursor_x = layout.board1StartX + 5;
                } else {
                    int maxY, maxX;
                    getmaxyx(stdscr, maxY, maxX);
                    move(maxY - 3, 1);
                    printw("Collision cannot place ship");
                    refresh();
                    napms(500); 
                }
                break;
        }
    }
    buildShipCellMap(); 
    return 1;
}

bool Gameboard::acceptGameboard() {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    move(maxY - 3, 1);
    printw("Do you want to use this board? Enter y/n");
    while (true) {
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

void Gameboard::placeGamePiece(int cursor_x, int cursor_y, int orientation, int ship_length, char ship_symbol) {
    BoardLayout layout = calculateBoardLayout(getBoardSize());
    
    int gridX = (cursor_x - layout.board1StartX - 5) / 4;
    int gridY = cursor_y - layout.startY - 3;
    
    if (orientation == 0) {
        for (int i = 0; i < ship_length; i++) {
            boardArray[gridY][gridX - i] = ship_symbol;
        }
    } else {
        for (int i = 0; i < ship_length; i++) {
            boardArray[gridY - i][gridX] = ship_symbol;
        }
    }
}

std::vector<std::pair<int, int>> Gameboard::getShipCoordinates(char shipSymbol) {
    std::vector<std::pair<int, int>> coords;
    for (const auto& pair : shipCellMap) {
        if (pair.second == shipSymbol) {
            coords.push_back(pair.first);
        }
    }
    return coords;
}

std::vector<std::pair<int, int>> Gameboard::getShipOccupiedCells(int x, int y) {
    std::vector<std::pair<int, int>> cells;

    for (const auto& ship : myShips) {
        bool belongsToShip = false;
        if (ship.orientation == 1) { 
            if (x == ship.startCol && y >= ship.startRow && y < ship.startRow + ship.length) {
                belongsToShip = true;
            }
        } else { 
            if (y == ship.startRow && x <= ship.startCol && x > ship.startCol - ship.length) {
                belongsToShip = true;
            }
        }

        if (belongsToShip) {
            for (int i = 0; i < ship.length; i++) {
                int r = ship.startRow;
                int c = ship.startCol;

                if (ship.orientation == 1) {
                    r += i; 
                } else {
                    c -= i; 
                }
                cells.push_back({c, r}); 
            }
            return cells; 
        }
    }
    return cells; 
}

bool Gameboard::highlightShip(int cursor_x, int cursor_y, int ship_length, int orientation, char ship_symbol) {
    BoardLayout layout = calculateBoardLayout(getBoardSize());
    bool validPlacement = true;
    
    int gridX = (cursor_x - layout.board1StartX - 5) / 4;
    int gridY = cursor_y - layout.startY - 3;
    
    if (orientation == 0) {
        for (int i = 0; i < ship_length; i++) {
            int checkX = gridX - i;
            
            if (checkX < 0 || checkX >= boardSize || boardArray[gridY][checkX] != 'w') {
                move(cursor_y, cursor_x - (4 * i));
                
                char c = inch() & A_CHARTEXT;
                
                attron(COLOR_PAIR(4)); 
                addch(c); 
                attroff(COLOR_PAIR(4)); 
                
                validPlacement = false;
            } else {
                move(cursor_y, cursor_x - (4 * i));
                attron(A_STANDOUT);
                addch(ship_symbol);
                attroff(A_STANDOUT);
            }
        }
    } else { 
        for (int i = 0; i < ship_length; i++) {
            int checkY = gridY - i;
            
            if (checkY < 0 || checkY >= boardSize || boardArray[checkY][gridX] != 'w') {
                move(cursor_y - i, cursor_x);
                
                char c = inch() & A_CHARTEXT;
                
                attron(COLOR_PAIR(4));
                addch(c);
                attroff(COLOR_PAIR(4));
                
                validPlacement = false;
            } else {
                move(cursor_y - i, cursor_x);
                attron(A_STANDOUT);
                addch(ship_symbol);
                attroff(A_STANDOUT);
            }
        }
    }
    
    move(cursor_y, cursor_x); 
    return validPlacement;
}

void Gameboard::displayEmptyBoard() {
    int size = getBoardSize();
    BoardLayout layout = calculateBoardLayout(size);
    int boardWidth = size * 4 + 8;
    
    clear(); 
    move(layout.startY, layout.board1StartX);
    
    const char* title = (size >= 20) ? "You" : (size >= 15) ? "Your" : "Your Board";
    int titleLen = strlen(title);
    int leftPad = (boardWidth - titleLen) / 2;
    int rightPad = boardWidth - leftPad - titleLen;
    
    for (int i = 0; i < leftPad; i++) printw("-");
    printw("%s", title);
    for (int i = 0; i < rightPad; i++) printw("-");
    printw("\n");
    
    move(layout.startY + 1, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("_");
    printw("\n");
    
    move(layout.startY + 2, layout.board1StartX);
    printw("|  |");
    for (int i = 0; i < size; i++) {
        printw(" %c |", 'A' + i);
    }
    printw("\n");
    
    for (int i = 0; i < size; i++) {
        move(layout.startY + 3 + i, layout.board1StartX);
        if (i + 1 < 10) {
            printw("| %d|", i + 1);
        } else {
            printw("|%d|", i + 1);
        }
        for (int j = 0; j < size; j++) {
            printw("   |");
        }
    }
    
    move(layout.startY + 3 + size, layout.board1StartX);
    for (int i = 0; i < boardWidth; i++) printw("-");
    printw("\n");
    
    attron(A_UNDERLINE);
    mvprintw(1, 1, "instructions");
    attroff(A_UNDERLINE);
    
    mvprintw(2, 1, "w/u - up      a/<- - left");
    mvprintw(3, 1, "s/d - down    d/-> - right");
    mvprintw(4, 1, "r - rotate piece");
    mvprintw(5, 1, "space/enter - place piece");
    mvprintw(6, 1, "g - switch to random mode");

    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (boardArray[i][j] != 'w') {
                move(layout.startY + 3 + i, layout.board1StartX + 5 + (j * 4));
                attron(A_BOLD); 
                addch(boardArray[i][j]);
                attroff(A_BOLD);
            }
        }
    }
 
    refresh();
}

void Gameboard::initializeBoard() {
    boardSize = ::getBoardSize();
    boardArray.clear();
    boardArray.resize(boardSize, std::vector<char>(boardSize, 'w'));
}

void Gameboard::initializeGamePieces() {
    piece_array.clear();
    shipStatus.clear();
    myShips.clear();
    
    ShipConfiguration config = getShipConfig(boardSize);
    
    int shipCounter = 0;
    
    auto createShip = [&](int len, char sym) -> ActiveShip {
        ActiveShip s;
        s.id = shipCounter;
        s.symbol = sym;
        s.length = len;
        s.hitCount = 0;
        s.isSunk = false;
        s.startRow = -1;
        s.startCol = -1;
        s.orientation = 0;
        return s;
    };

    for (int i = 0; i < config.fourDeck; i++) {
        char symbol = 'A' + (shipCounter % 26);
        piece_array.push_back(Game_Piece(4, symbol));
        shipStatus[symbol] = createShip(4, symbol);
        shipCounter++;
    }
    
    for (int i = 0; i < config.threeDeck; i++) {
        char symbol = 'A' + (shipCounter % 26);
        piece_array.push_back(Game_Piece(3, symbol));
        shipStatus[symbol] = createShip(3, symbol);
        shipCounter++;
    }
    
    for (int i = 0; i < config.twoDeck; i++) {
        char symbol = 'A' + (shipCounter % 26);
        piece_array.push_back(Game_Piece(2, symbol));
        shipStatus[symbol] = createShip(2, symbol);
        shipCounter++;
    }
    
    for (int i = 0; i < config.oneDeck; i++) {
        char symbol = 'A' + (shipCounter % 26);
        piece_array.push_back(Game_Piece(1, symbol));
        shipStatus[symbol] = createShip(1, symbol);
        shipCounter++;
    }
}

void Gameboard::printBoard() {
    cout << "------ Printing Game board ------\n";
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            cout << boardArray[i][j] << " ";
        }
        cout << "\n";
    }
    cout << "\n";
}

void Gameboard::generateBoardPlacement() {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    if (isHost) {
        srand(static_cast<unsigned int>(seed) + 3000);
    } else {
        srand(static_cast<unsigned int>(seed));
    }

    for (size_t i = 0; i < piece_array.size(); i++) {
        int starting_peg = (rand() % (boardSize * boardSize));
        int orientation = (rand() % 2) + 1;
        int piece_length = piece_array[i].Get_Piece_Length();
        char piece_symbol = piece_array[i].Get_Piece_Symbol();
        
        int ret = 0;
        int attempts = 0;
        const int MAX_ATTEMPTS = 1000;
        
        while ((ret = checkStartingPeg(orientation, starting_peg, piece_length)) != 1) {
            if (ret == 2) {
                orientation = (orientation == 1) ? 2 : 1;
                ret = checkStartingPeg(orientation, starting_peg, piece_length);
                if (ret == 1) {
                    break;
                }
            }

            starting_peg = (rand() % (boardSize * boardSize));
            orientation = (rand() % 2) + 1;
            attempts++;
            
            if (attempts > MAX_ATTEMPTS) {
                initializeBoard();
                myShips.clear();
                i = -1;
                break;
            }
        }
        
        if (i >= 0 && ret == 1) {
            addPieceToBoardArray(orientation, starting_peg, piece_length, piece_symbol);
        }
    }
}

void Gameboard::buildShipCellMap() {
    shipCellMap.clear();
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            char cell = boardArray[i][j];
            if (cell >= 'A' && cell <= 'Z') {
                shipCellMap[{j, i}] = cell;
            }
        }
    }
}

short Gameboard::checkStartingPeg(int orientation, int starting_peg, int piece_length) {
    if (orientation == 1) {
        for (int j = 0; j < piece_length; j++) {
            if ((starting_peg / boardSize) + j >= boardSize) {
                return 2;
            } else if (boardArray[(starting_peg / boardSize) + j][(starting_peg % boardSize)] != 'w') {
                return 3;
            }
        }
        return 1;
    } else {
        for (int j = 0; j < piece_length; j++) {
            if ((starting_peg % boardSize) - j < 0) {
                return 2;
            }
            if (boardArray[(starting_peg / boardSize)][(starting_peg % boardSize) - j] != 'w') {
                return 3;
            }
        }
        return 1;
    }
}

void Gameboard::addPieceToBoardArray(int orientation, int starting_peg, int piece_length, char piece_symbol) {
    int row = starting_peg / boardSize;
    int col = starting_peg % boardSize;

    ActiveShip newShip;
    newShip.startRow = row;
    newShip.startCol = col;
    newShip.length = piece_length;
    newShip.orientation = orientation;
    newShip.symbol = piece_symbol;
    newShip.hitCount = 0;
    newShip.isSunk = false;
    newShip.id = (int)myShips.size();

    myShips.push_back(newShip);
    shipStatus[piece_symbol] = newShip;

    if (orientation == 1) { 
        for (int j = 0; j < piece_length; j++) {
            boardArray[row + j][col] = piece_symbol;
        }
    } else { 
        for (int j = 0; j < piece_length; j++) {
            boardArray[row][col - j] = piece_symbol;
        }
    }
}

void Gameboard::setIsHost(bool host) {
    isHost = host;
}

bool Gameboard::isShipSunk(char shipSymbol) {
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (boardArray[i][j] == shipSymbol) {
                return false;
            }
        }
    }
    return true;
}

void Gameboard::markShipAsHit(char shipSymbol) {
    if (shipStatus.find(shipSymbol) != shipStatus.end()) {
        shipStatus[shipSymbol].hitCount++;
        if (shipStatus[shipSymbol].hitCount >= shipStatus[shipSymbol].length) {
            shipStatus[shipSymbol].isSunk = true;
        }
    }
}

int Gameboard::getRemainingShips() {
    int count = 0;
    for (auto& pair : shipStatus) {
        if (!pair.second.isSunk) {
            count++;
        }
    }
    return count;
}

int Gameboard::receiveShot(int x, int y) {
    if (x < 0 || x >= boardSize || y < 0 || y >= boardSize) return 0;
    
    char cell = boardArray[y][x];
    
    if (cell == 'x' || cell == 'o' || cell == 's') return 0;

    if (cell == 'w') { 
        boardArray[y][x] = 'o';  
        missCount++; 
        return 0; 
    }

    if (cell >= 'A' && cell <= 'Z') {
        char targetSymbol = cell;
        
        for (auto& ship : myShips) {
            if (ship.isSunk) continue;

            if (ship.symbol != targetSymbol) continue;
            
            bool isHit = false;

            if (ship.length == 1) {
                if (x == ship.startCol && y == ship.startRow) {
                    isHit = true;
                }
            }
            else if (ship.orientation == 1) { 
                if (x == ship.startCol && y >= ship.startRow && y < ship.startRow + ship.length) {
                    isHit = true;
                }
            } else { 
                if (y == ship.startRow && x <= ship.startCol && x > ship.startCol - ship.length) {
                    isHit = true;
                }
            }

            if (isHit) {
                ship.hitCount++;
                
                if (ship.hitCount >= ship.length) {
                    ship.isSunk = true;

                    shipStatus[ship.symbol].isSunk = true;
                    shipStatus[ship.symbol].hitCount = ship.hitCount;
                    
                    for (int i = 0; i < ship.length; i++) {
                        int r = ship.startRow;
                        int c = ship.startCol;

                        if (ship.orientation == 1) {
                            r += i; 
                        } else {
                            c -= i; 
                        }
                        
                        if (r >= 0 && r < boardSize && c >= 0 && c < boardSize) {
                            boardArray[r][c] = 's'; 
                        }
                    }
                    return 2; 
                } else {
                    boardArray[y][x] = 'x';  
                    
                    shipStatus[ship.symbol].hitCount = ship.hitCount;
                    
                    return 1;
                }
            }
        }
    }

    boardArray[y][x] = 'x';
    return 1; 
}

int Gameboard::getWoundedCount() {
    int count = 0;
    for (const auto& ship : myShips) {
        if (!ship.isSunk && ship.hitCount > 0) {
            count += ship.hitCount; 
        }
    }
    return count;
}

int Gameboard::getSunkCount() {
    int count = 0;
    for (const auto& ship : myShips) {
        if (ship.isSunk) count++;
    }
    return count;
}

int Gameboard::getMissCount() {
    return missCount;
}
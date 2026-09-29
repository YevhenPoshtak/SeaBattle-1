#ifndef AI_PLAYER_HPP
#define AI_PLAYER_HPP

#include "../user/user.hpp"
#include "../game/game_board.hpp"
#include "../ui/config.hpp"
#include <vector>
#include <cstdlib>
#include <ctime>

enum AIDifficulty {
    EASY,  
    SMART   
};

struct AICoordinates {
    int x, y;
};

class AIPlayer : public User {
private:
    AIDifficulty difficulty;
    Gameboard myBoard;
    std::vector<std::vector<char>> opponentBoard;
    
    std::vector<AICoordinates> availableShots;
    
    std::vector<AICoordinates> targetQueue;
    AICoordinates lastHit;
    bool hunting; 
    int huntDirection; 
    
    unsigned int attackSeed; 
    
    void initializeAvailableShots();
    AICoordinates getRandomShot();
    AICoordinates getSmartShot();
    void addAdjacentCells(int x, int y);
    void addDirectionalCells(int x, int y);

    bool isValidCoordinate(int x, int y); 
    void clearTargetQueue();
    
public:
    AIPlayer(AIDifficulty diff);
    virtual ~AIPlayer() {}
    
    virtual void connect() override;

    AICoordinates pickAttackCoordinates(int boardSize);
    void recordShotResult(int x, int y, bool isHit, bool isSunk);
    
    virtual void gameLoop(SOCKET client_socket) override;
    
    void initializeBoard(bool isHost) {
        myBoard.generateRandomBoard(isHost);
    }
    
    void setupBoard();
    AICoordinates makeMove();
    void receiveAttackResult(AICoordinates coords, char result);
    char receiveAttack(AICoordinates coords);
    Gameboard& getBoard();
    bool hasLost();

    void reset();
    
    static void playGame(AIDifficulty difficulty);
};

#endif
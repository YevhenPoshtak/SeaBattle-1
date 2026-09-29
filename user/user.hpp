#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include <queue>
#include <vector>
#include <string.h> 

#ifdef _WIN32
    #include <pdcurses.h> 
#else
    #include <ncurses.h> 
#endif

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define GET_SOCKET_ERROR() WSAGetLastError()
    typedef int socklen_t;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h> 
    #include <errno.h>
    #include <netdb.h>
    
    #define GET_SOCKET_ERROR() errno
    #define closesocket close 
    
    typedef int SOCKET;
    const int INVALID_SOCKET = -1;
    const int SOCKET_ERROR = -1;   
#endif

#include "../game/game_board.hpp"
#include "../ui/config.hpp"
#include "../game/ship_config.hpp"

#define PORT 8888

using namespace std;

struct coordinates {
    int y, x;
};

struct char_coordinates {
    int y, x;
    char c;
};

struct ShotResult {
    int x, y;
    char result;
    bool used;
};

class User {

public:

    void printClientIP(struct sockaddr_in their_address);

    virtual void gameLoop(SOCKET client_socket); 

    void displayBoard(Gameboard board);

    struct coordinates cursor;

    virtual void connect() = 0;

private:
protected:
    SOCKET client_socket; 

    bool isHost;

    User(bool isHost) : isHost(isHost) {}

    int handleAttack(SOCKET client_socket, Gameboard& board);

    void messageLog(string message);

    void handleFullBoard(char_coordinates cc);
};

class Host : public User {
public:
    virtual ~Host() {}
    Host() : User(true) {}

    void connect();

private:
    bool acceptClientConnection();
};

class Client : public User {

public:
    virtual ~Client() {}
    Client(char *serverName) : User(false), serverName(serverName) {}

    void connect();

private:
    char *serverName;
    unsigned long resolveName(char *name);
};

#endif
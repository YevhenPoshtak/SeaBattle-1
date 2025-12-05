/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: network_logic.hpp
 * Description: Header file for network communication logic. Handles cross-platform
 *              socket operations for multiplayer gameplay, including host/client
 *              connection management and game data transmission.
 */

#ifndef NETWORK_LOGIC_HPP
#define NETWORK_LOGIC_HPP

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SOCKET_TYPE;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define closesocket closesocket
    #define GET_SOCKET_ERROR() WSAGetLastError()
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    typedef int SOCKET_TYPE;
    const int INVALID_SOCKET_VALUE = -1;
    #define closesocket close
    #define GET_SOCKET_ERROR() errno
    #define MSG_WAITALL MSG_WAITALL
#endif

#include "../data/game_state.hpp"
#include <string>

#define PORT 12345  // Default port for game connections

class NetworkLogic {
public:
    // Initialize networking subsystem (Windows WSA startup)
    static bool initializeNetworking();
    
    // Clean up networking resources
    static void cleanupNetworking();
    
    // Create and configure socket for hosting a game
    static SOCKET_TYPE createHostSocket();
    
    // Accept incoming client connection on host socket
    static SOCKET_TYPE acceptClientConnection(SOCKET_TYPE hostSocket, bool& accepted);
    
    // Create socket and connect to host
    static SOCKET_TYPE createClientSocket(const char* hostname);
    
    // Resolve hostname to IP address
    static unsigned long resolveName(const char* name);
    
    // Send game configuration (board size, shots per turn)
    static bool sendGameSettings(SOCKET_TYPE socket, int boardSize, int shotsPerTurn);
    
    // Receive game configuration from opponent
    static bool receiveGameSettings(SOCKET_TYPE socket, int& boardSize, int& shotsPerTurn);
    
    // Send shot coordinates to opponent
    static bool sendShot(SOCKET_TYPE socket, const coordinates& shot);
    
    // Receive shot coordinates from opponent
    static bool receiveShot(SOCKET_TYPE socket, coordinates& shot);
    
    // Send result of opponent's shot (hit/miss/sunk)
    static bool sendShotResult(SOCKET_TYPE socket, char result);
    
    // Receive result of our shot
    static bool receiveShotResult(SOCKET_TYPE socket, char& result);
    
    // Send number of shots in current turn
    static bool sendShotCount(SOCKET_TYPE socket, int count);
    
    // Receive number of shots opponent is making
    static bool receiveShotCount(SOCKET_TYPE socket, int& count);
};

#endif
/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: network_logic.cpp
 * Description: Implementation of network communication logic. Provides cross-platform
 *              socket operations with proper error handling and timeout configuration
 *              for reliable multiplayer gameplay.
 */

#include "network_logic.hpp"
#include <cstring>

// Initialize networking subsystem (required for Windows)
bool NetworkLogic::initializeNetworking() {
    #ifdef _WIN32
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    #else
        return true;  // No initialization needed on Unix-like systems
    #endif
}

// Clean up networking resources
void NetworkLogic::cleanupNetworking() {
    #ifdef _WIN32
        WSACleanup();
    #endif
}

// Create and configure host socket for accepting connections
SOCKET_TYPE NetworkLogic::createHostSocket() {
    // Create TCP socket
    SOCKET_TYPE hostSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (hostSocket == INVALID_SOCKET_VALUE) {
        return INVALID_SOCKET_VALUE;
    }
    
    // Enable address reuse to avoid "address already in use" errors
    int on = 1;
    if (setsockopt(hostSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) < 0) {
        closesocket(hostSocket);
        return INVALID_SOCKET_VALUE;
    }
    
    // Configure server address structure
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;  // Accept connections on any interface
    serverAddress.sin_port = htons(PORT);
    
    // Bind socket to address
    if (bind(hostSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        closesocket(hostSocket);
        return INVALID_SOCKET_VALUE;
    }
    
    // Start listening for connections (queue up to 5)
    if (listen(hostSocket, 5) < 0) {
        closesocket(hostSocket);
        return INVALID_SOCKET_VALUE;
    }
    
    return hostSocket;
}

// Accept incoming client connection
SOCKET_TYPE NetworkLogic::acceptClientConnection(SOCKET_TYPE hostSocket, bool& accepted) {
    struct sockaddr_in clientAddress;
    socklen_t addressSize = sizeof(clientAddress);
    
    // Accept connection
    SOCKET_TYPE clientSocket = accept(hostSocket, (struct sockaddr*)&clientAddress, &addressSize);
    if (clientSocket == INVALID_SOCKET_VALUE) {
        accepted = false;
        return INVALID_SOCKET_VALUE;
    }
    
    // Set receive timeout (60 seconds) to prevent indefinite blocking
    #ifdef _WIN32
        DWORD timeout = 60000;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    #else
        struct timeval tv;
        tv.tv_sec = 60;
        tv.tv_usec = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    #endif
    
    accepted = true;
    return clientSocket;
}

// Create client socket and connect to host
SOCKET_TYPE NetworkLogic::createClientSocket(const char* hostname) {
    // Create TCP socket
    SOCKET_TYPE clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET_VALUE) {
        return INVALID_SOCKET_VALUE;
    }
    
    // Set receive timeout
    #ifdef _WIN32
        DWORD timeout = 60000;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    #else
        struct timeval tv;
        tv.tv_sec = 60;
        tv.tv_usec = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    #endif
    
    // Configure server address
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = resolveName(hostname);
    
    if (serverAddress.sin_addr.s_addr == 0) {
        closesocket(clientSocket);
        return INVALID_SOCKET_VALUE;
    }
    
    serverAddress.sin_port = htons(PORT);
    
    // Connect to server
    if (connect(clientSocket, (const struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        closesocket(clientSocket);
        return INVALID_SOCKET_VALUE;
    }
    
    return clientSocket;
}

// Resolve hostname to IP address
unsigned long NetworkLogic::resolveName(const char* name) {
    struct hostent* host = gethostbyname(name);
    if (host == NULL) {
        return 0;
    }
    return *((unsigned long*)host->h_addr_list[0]);
}

// Send game settings to opponent
bool NetworkLogic::sendGameSettings(SOCKET_TYPE socket, int boardSize, int shotsPerTurn) {
    if (send(socket, (const char*)&boardSize, sizeof(int), 0) <= 0) return false;
    if (send(socket, (const char*)&shotsPerTurn, sizeof(int), 0) <= 0) return false;
    return true;
}

// Receive game settings from opponent
bool NetworkLogic::receiveGameSettings(SOCKET_TYPE socket, int& boardSize, int& shotsPerTurn) {
    if (recv(socket, (char*)&boardSize, sizeof(int), MSG_WAITALL) <= 0) return false;
    if (recv(socket, (char*)&shotsPerTurn, sizeof(int), MSG_WAITALL) <= 0) return false;
    return true;
}

// Send shot coordinates
bool NetworkLogic::sendShot(SOCKET_TYPE socket, const coordinates& shot) {
    return send(socket, (const char*)&shot, sizeof(shot), 0) > 0;
}

// Receive shot coordinates
bool NetworkLogic::receiveShot(SOCKET_TYPE socket, coordinates& shot) {
    return recv(socket, (char*)&shot, sizeof(shot), MSG_WAITALL) > 0;
}

// Send shot result (hit/miss/sunk indicator)
bool NetworkLogic::sendShotResult(SOCKET_TYPE socket, char result) {
    return send(socket, &result, sizeof(char), 0) > 0;
}

// Receive shot result
bool NetworkLogic::receiveShotResult(SOCKET_TYPE socket, char& result) {
    return recv(socket, &result, sizeof(char), MSG_WAITALL) > 0;
}

// Send number of shots in volley
bool NetworkLogic::sendShotCount(SOCKET_TYPE socket, int count) {
    return send(socket, (const char*)&count, sizeof(int), 0) > 0;
}

// Receive number of shots in opponent's volley
bool NetworkLogic::receiveShotCount(SOCKET_TYPE socket, int& count) {
    return recv(socket, (char*)&count, sizeof(int), MSG_WAITALL) > 0;
}
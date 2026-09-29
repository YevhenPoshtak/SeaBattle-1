#include "user/user.hpp"

void Client::connect() {
    struct sockaddr_in server_address;
    unsigned short serverPort = PORT;
    
    client_socket = socket(PF_INET, SOCK_STREAM, 0);

    #ifdef _WIN32
        DWORD timeout = 60000;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    #else
        struct timeval tv;
        tv.tv_sec = 60;  
        tv.tv_usec = 0;
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    #endif

    if (client_socket == INVALID_SOCKET) {
        attrset(A_NORMAL);
        clear(); 
        mvprintw(7, 2, "Error creating client socket\n");
        mvprintw(9, 2, "Press any key to return..."); 
        refresh();
        getch(); 
        clear(); 
        return;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    
    server_address.sin_addr.s_addr = resolveName(serverName);
    
    if (server_address.sin_addr.s_addr == 0) {
        clear();
        mvprintw(7, 2, "Invalid IP address or host not found.\n");
        mvprintw(9, 2, "Press any key to return...");
        refresh();
        getch();
        closesocket(client_socket);
        clear(); 
        return;
    }
    
    server_address.sin_port = htons(serverPort);

    clear(); 
    mvprintw(8, 2, "Connecting to %s...", serverName);
    refresh();

    if (::connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        
        int error = GET_SOCKET_ERROR();
        #ifdef _WIN32
        if (error == WSAECONNREFUSED) {
        #else
        if (error == ECONNREFUSED) {
        #endif
            mvprintw(8, 2, "Connection refused. Is the Host started?\n");
        } else {
            mvprintw(8, 2, "Connection failed. Error code: %d\n", error);
        }
        
        mvprintw(10, 2, "Press any key to return...");
        refresh();
        getch(); 
        closesocket(client_socket); 
        clear(); 
        return;
    } 

    mvprintw(8, 2, "Connected! Waiting for host to start game...\n");
    refresh();
    
    char *received_msg = (char *) malloc(1024);
    int len = recv(client_socket, received_msg, 1024, 0);
    
    if (len <= 0) {
        attrset(A_NORMAL);
        clear();
        mvprintw(6, 2, "The Host has disconnected.\n");
        mvprintw(8, 2, "Press any key to return...");
        refresh();
        flushinp();
        getch();
        closesocket(client_socket); 
        free(received_msg);
        clear();
        return;
    } else {
        free(received_msg);
        
        clear();
        refresh();
        
        this->gameLoop(client_socket);
        
        clear();
        refresh();
        closesocket(client_socket); 
    }
}

unsigned long Client::resolveName(char *name) {
    struct hostent *host;
    if ((host = gethostbyname(name)) == NULL) {
        return 0; 
    }
    return *((unsigned long *) host->h_addr_list[0]);
}
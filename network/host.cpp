#include "user/user.hpp"

void Host::connect() {
    SOCKET host_socket;
    struct sockaddr_in server_address;

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    
    clear(); 
    refresh();

    host_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (host_socket == INVALID_SOCKET) {
        mvprintw(0, 0, "Error creating host socket. Error = %d", GET_SOCKET_ERROR());
        mvprintw(2, 0, "Press any key to return...");
        refresh();
        getch();
        closesocket(host_socket); 
        clear();
        return; 
    }

    int on = 1;
    if (setsockopt(host_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) < 0) { 
        mvprintw(0, 0, "Error setting socket options. Error = %d", GET_SOCKET_ERROR());
        mvprintw(2, 0, "Press any key to return...");
        refresh();
        getch();
        closesocket(host_socket);
        clear();
        return;
    }

    if (bind(host_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        mvprintw(0, 0, "Error binding socket. Error = %d", GET_SOCKET_ERROR());
        mvprintw(2, 0, "Press any key to return...");
        refresh();
        getch();
        closesocket(host_socket); 
        clear();
        return;
    }

    if (listen(host_socket, 5) < 0) {
        mvprintw(0, 0, "Error listening. Error = %d", GET_SOCKET_ERROR());
        mvprintw(2, 0, "Press any key to return...");
        refresh();
        getch();
        closesocket(host_socket); 
        clear();
        return;
    }

    mvprintw(8, 2,"Waiting for a client to connect...\n");
    refresh();

    while (1) {
        struct sockaddr_in client_address;
        socklen_t address_size = sizeof(client_address);
        
        if ((client_socket = accept(host_socket, (struct sockaddr *) &client_address, &address_size)) < 0) { 
            attrset(A_NORMAL);
            mvprintw(10, 2, "Error trying to accept client connection.\n");
            refresh();
            continue; 
        }

        #ifdef _WIN32
            DWORD timeout = 60000; 
            setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        #else
            struct timeval tv;
            tv.tv_sec = 60;
            tv.tv_usec = 0;
            setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        #endif

        if (acceptClientConnection()) {
            const char *msg = "Connection accepted by host\n";
            send(client_socket, msg, strlen(msg), 0);
            
            clear();
            refresh();
            
            this->gameLoop(client_socket);
            
            clear();
            refresh();
            closesocket(client_socket); 
            closesocket(host_socket); 
            return;
        } else {
            closesocket(client_socket); 
            move(9, 0); clrtoeol();
            move(10, 0); clrtoeol();
            move(11, 0); clrtoeol();
            mvprintw(9, 2, "Connection rejected. Waiting for another client...\n");
            refresh();
        }
    }
}

bool Host::acceptClientConnection() {
    mvprintw(9, 2, "\n  Would you like to accept the connection, Y/N?\n");
    refresh();

    while (1) {
        flushinp();
        char user_input;
        user_input = getch();
        if (user_input == 'Y' || user_input == 'y') {
            mvprintw(10, 2, "  accepting connection! Have fun!\n");
            refresh();
            #ifdef _WIN32
                Sleep(500);
            #else
                usleep(500000);
            #endif
            return true;
        } else if (user_input == 'N' || user_input == 'n') {
            mvprintw(10, 2, "Connection refused...\n");
            refresh();
            #ifdef _WIN32
                Sleep(500);
            #else
                usleep(500000);
            #endif
            return false;
        }
    }
}
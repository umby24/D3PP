//
// Created by Wande on 2/25/2021.
//

#ifndef __linux__
#ifndef D3PP_WINDOWSSERVERSOCKETS_H
#define D3PP_WINDOWSSERVERSOCKETS_H

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "../Logger.h"

class ServerSocket {
public:
    ServerSocket(int port);

    void Listen();
private:
    int listenPort;
    SOCKET listenSocket;
    struct addrinfo hints;
    struct addrinfo *result;
};

#endif //D3PP_WINDOWSSERVERSOCKETS_H
#endif
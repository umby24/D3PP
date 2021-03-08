//
// Created by Wande on 2/25/2021.
//

#ifndef __linux__
#ifndef D3PP_WINDOWSSERVERSOCKETS_H
#define D3PP_WINDOWSSERVERSOCKETS_H

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <ws2def.h>
#include <stdio.h>
#include "../Logger.h"
#include "WindowsSockets.h"

class ServerSocket {
public:
    ServerSocket();
    ServerSocket(int port);
    void Init(int port);
    void Listen();
    Sockets Accept();
    void Stop();
private:
    bool hasInit;
    int listenPort;
    SOCKET listenSocket;
    fd_set mySockDescripts;
    struct addrinfo hints;
    struct addrinfo *result;
};

#endif //D3PP_WINDOWSSERVERSOCKETS_H
#endif
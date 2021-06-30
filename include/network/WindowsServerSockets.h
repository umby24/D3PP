//
// Created by Wande on 2/25/2021.
//

#ifndef __linux__
#ifndef D3PP_WINDOWSSERVERSOCKETS_H
#define D3PP_WINDOWSSERVERSOCKETS_H
#define MAXIMUM_CONNECTIONS 255

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <ws2def.h>
#include <stdio.h>
#include <memory>

class Sockets;

enum ServerSocketEvent {
    SOCKET_EVENT_NONE,
    SOCKET_EVENT_CONNECT,
    SOCKET_EVENT_DATA,
    SOCKET_EVENT_DISCONNECT
};

class ServerSocket {
public:
    ServerSocket();
    ServerSocket(int port);
    void Init(int port);
    void Listen();
    ServerSocketEvent CheckEvents();
    std::unique_ptr<Sockets> Accept();
    void Unaccept(SOCKET fd);
    void Stop();
    SOCKET GetEventSocket();
private:
    bool hasInit;
    int listenPort;
    WSADATA wsa;
    SOCKET listenSocket;
    SOCKET clientSockets[MAXIMUM_CONNECTIONS];
    SOCKET eventSocket;
    struct sockaddr_in server, address;

    fd_set readfds;
};

#endif //D3PP_WINDOWSSERVERSOCKETS_H
#endif
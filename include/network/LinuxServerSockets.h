//
// Created by Wande on 2/25/2021.
//


#ifdef __linux__
#ifndef D3PP_LINUXSERVERSOCKETS_H
#define D3PP_LINUXSERVERSOCKETS_H
#include <memory>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "../Logger.h"

#include "LinuxSockets.h"
#define MAXIMUM_CONNECTIONS 255

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
    void Unaccept(int fd);
    void Stop();
private:
    bool hasInit;
    int listenPort;
    int listenSocket;
    int clientSockets[MAXIMUM_CONNECTIONS];
    struct sockaddr_in server, address;

    fd_set readfds;
};
#endif //D3PP_LINUXSERVERSOCKETS_H
#endif
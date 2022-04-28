//
// Created by Wande on 2/25/2021.
//


#ifdef __linux__
#ifndef D3PP_LINUXSERVERSOCKETS_H
#define D3PP_LINUXSERVERSOCKETS_H
#include <memory>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#define MAXIMUM_CONNECTIONS 255

enum ServerSocketEvent {
    SOCKET_EVENT_NONE,
    SOCKET_EVENT_CONNECT,
    SOCKET_EVENT_DATA,
    SOCKET_EVENT_DISCONNECT
};

class Sockets;

class ServerSocket {
public:
    ServerSocket();
    ServerSocket(int port);
    void Init(int port);
    void Listen();
    ServerSocketEvent CheckEvents();
    std::unique_ptr<Sockets> Accept();
    [[nodiscard]] int GetEventSocket() const;
    void Unaccept(int fd);
    void Stop();
private:
    bool hasInit;
    int listenPort;
    int listenSocket;
    int clientSockets[MAXIMUM_CONNECTIONS];
    int eventSocket;
    struct sockaddr_in server, address;

    fd_set readfds;
};
#endif //D3PP_LINUXSERVERSOCKETS_H
#endif
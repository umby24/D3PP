//
// Created by unknown on 4/2/21.
//


#include "network/LinuxServerSockets.h"

#ifdef __linux__
#include "common/Logger.h"
#include "Utils.h"
#include <errno.h>
#include <memory>
#include "arpa/inet.h"
#include "network/LinuxSockets.h"

const std::string MODULE_NAME = "ServerSocket";

ServerSocket::ServerSocket() : server(), address(), readfds(), clientSockets() {
    hasInit = false;
    listenPort = 25565;
    listenSocket = -1;
    eventSocket = -1;

    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        clientSockets[i] = 0;
    }
}

ServerSocket::ServerSocket(int port) : server(), address(), readfds(), clientSockets() {
    hasInit = false;
    listenPort = 25565;
    listenSocket = -1;
    eventSocket = -1;
    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        clientSockets[i] = 0;
    }
    Init(port);
}

void ServerSocket::Listen() {
    if (!hasInit) {
        Logger::LogAdd(MODULE_NAME, "SOCKETS NOT INITIALIZED YET!!!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error("Attempted to start listening before initializing SOCKETS");
    }

    int iResult;
    iResult = bind(listenSocket, (struct sockaddr*)&server, sizeof(server));
    if (iResult < 0) {
        Logger::LogAdd(MODULE_NAME, "Failed to start networking -- " + stringulate(errno), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    iResult = listen(listenSocket, 5);

    if (iResult < 0) {
        Logger::LogAdd(MODULE_NAME, "Failed to start listening", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        close(listenSocket);
        return;
    }

    // -- Now we're ready to accept clients <3
}

void ServerSocket::Init(int port) {
    if (hasInit)
        return;

    listenPort = port;

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        Logger::LogAdd(MODULE_NAME, "Failed to initialize sockets[3]", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    int enable = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int) < 0);
    hasInit = true;
}

void ServerSocket::Stop() {

}

std::unique_ptr<Sockets> ServerSocket::Accept() {
    int newSocket;
    int addrlen = sizeof(struct sockaddr_in);
    newSocket = accept(listenSocket, (struct sockaddr*)&address, (socklen_t *) &addrlen);

    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        if (clientSockets[i] == 0) {
            clientSockets[i] = newSocket;
            break;
        }
    }

    return std::make_unique<Sockets>(newSocket, inet_ntoa(address.sin_addr));
}

ServerSocketEvent ServerSocket::CheckEvents() {
    if (!hasInit)
        return SOCKET_EVENT_NONE;
    if (listenSocket == -1) {
        return SOCKET_EVENT_NONE;
    }

    FD_ZERO(&readfds);
    FD_SET(listenSocket, &readfds);
    int maxFd = listenSocket;

    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        if (clientSockets[i] > 0) {
            if (clientSockets[i] > maxFd)
                maxFd = clientSockets[i];
            FD_SET(clientSockets[i], &readfds);
        }
    }
    struct timeval time = {0, 5};
    int activity = select(maxFd+1, &readfds, NULL, NULL, &time);

    if (activity == -1) {
        Logger::LogAdd("ServerSocket", "Some error occured calling select.", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return SOCKET_EVENT_NONE;
    }

    if (FD_ISSET(listenSocket, &readfds)) {
        // -- Incoming connection
        return SOCKET_EVENT_CONNECT;
    }

    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        int s = clientSockets[i];
        if (FD_ISSET(s, &readfds)) {
            eventSocket = s;
            return SOCKET_EVENT_DATA;
        }
    }

    return SOCKET_EVENT_NONE;
}

void ServerSocket::Unaccept(int fd) {
    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        if (clientSockets[i] == fd) {
            clientSockets[i] = 0;
        }
    }
}

int ServerSocket::GetEventSocket() const {
    return eventSocket;
}

#endif
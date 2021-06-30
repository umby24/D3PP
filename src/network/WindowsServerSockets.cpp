//
// Created by Wande on 2/26/2021.
//
#ifndef __linux__
#include "network/WindowsServerSockets.h"
#include "network/WindowsSockets.h"
#include "Logger.h"
#include "Utils.h"

const std::string MODULE_NAME = "ServerSocket";

ServerSocket::ServerSocket() {
    hasInit = false;

    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        clientSockets[i] = 0;
    }
}

ServerSocket::ServerSocket(int port) {
    hasInit = false;
    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        clientSockets[i] = 0;
    }
    Init(port);
}

void ServerSocket::Listen() {
    if (!hasInit) {
        Logger::LogAdd(MODULE_NAME, "WINSOCK NOT INITIALIZED YET!!!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error("Attempted to start listening before initializing winsock");
    }

    int iResult;
    iResult = bind(listenSocket, (struct sockaddr*)&server, sizeof(server));
    if (iResult== SOCKET_ERROR) {
        Logger::LogAdd(MODULE_NAME, "Failed to start networking", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;

    }
    iResult = listen(listenSocket, 5);

    if (iResult== SOCKET_ERROR) {
        Logger::LogAdd(MODULE_NAME, "Failed to start listening", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        closesocket(listenSocket);
        return;
    }

    // -- Now we're ready to accept clients <3
}

void ServerSocket::Init(int port) {
    if (hasInit)
        return;

    listenPort = port;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (iResult != 0) {
        int wsaError = WSAGetLastError();
        Logger::LogAdd(MODULE_NAME, "Failed to initialize winsock [" + stringulate(wsaError) + "]", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        Logger::LogAdd(MODULE_NAME, "Failed to initialize winsock[3]", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        WSACleanup();
        return;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    hasInit = true;
}

void ServerSocket::Stop() {

}

std::unique_ptr<Sockets> ServerSocket::Accept() {
    SOCKET newSocket;
    int addrlen = sizeof(struct sockaddr_in);
    newSocket = accept(listenSocket, (struct sockaddr*)&address, (int*)&addrlen);

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

    FD_ZERO(&readfds);
    FD_SET(listenSocket, &readfds);

    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        if (clientSockets[i] > 0) {
            FD_SET(clientSockets[i], &readfds);
        }
    }

    struct timeval time = {0, 5};
    int activity = select(0, &readfds, NULL, NULL, &time);

    if (activity == SOCKET_ERROR) {
        
        int errMsg = WSAGetLastError();
        Logger::LogAdd("ServerSocket", "Some error occured calling select." + stringulate(errMsg), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);

        return SOCKET_EVENT_NONE;
    }

    if (FD_ISSET(listenSocket, &readfds)) {
        // -- Incoming connection
        return SOCKET_EVENT_CONNECT;
    }

    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        SOCKET s = clientSockets[i];
        if (FD_ISSET(s, &readfds)) {
            eventSocket = s;
            return SOCKET_EVENT_DATA;
        }
    }

    return SOCKET_EVENT_NONE;
}

SOCKET ServerSocket::GetEventSocket() {
    return eventSocket;
}

void ServerSocket::Unaccept(SOCKET fd) {
    for (auto i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        if (clientSockets[i] == fd) {
            clientSockets[i] = 0;
            break;
        }
    }
}
#endif




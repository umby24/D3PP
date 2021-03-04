//
// Created by Wande on 2/26/2021.
//
#include "network/WindowsServerSockets.h"

ServerSocket::ServerSocket() {
    hasInit = false;
}

ServerSocket::ServerSocket(int port) {
    Init(port);
}

void ServerSocket::Listen() {
    if (!hasInit) {
        // -- exception?
    }
    int iResult;

    iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult== SOCKET_ERROR) {
        Logger::LogAdd("ServerSocket", "Failed to start networking", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult== SOCKET_ERROR) {
        Logger::LogAdd("ServerSocket", "Failed to start listening", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        closesocket(listenSocket);
        return;
    }

    // -- Now we're ready to accept clients <3
}

void ServerSocket::Init(int port) {
    if (hasInit)
        return;

    listenPort = port;
    WSADATA wsaData;
    int iResult;

    listenSocket = INVALID_SOCKET;
    result = NULL;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        Logger::LogAdd("ServerSocket", "Failed to initilize winsock", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // -- IPV4 only plz
    hints.ai_socktype = SOCK_STREAM; // -- TCP Socket
    hints.ai_protocol = IPPROTO_TCP; // -- TCP Protocol
    hints.ai_flags = AI_PASSIVE; // -- dunno.

    iResult = getaddrinfo(NULL, stringulate(listenPort).c_str(), &hints, &result);
    if (iResult!= 0) {
        Logger::LogAdd("ServerSocket", "Failed to initilize winsock[2]", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        WSACleanup();
        return;
    }

    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        Logger::LogAdd("ServerSocket", "Failed to initilize winsock[3]", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        WSACleanup();
        freeaddrinfo(result);
        return;
    }
    hasInit = true;
}





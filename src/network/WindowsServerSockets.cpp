//
// Created by Wande on 2/26/2021.
//
#include "network/WindowsServerSockets.h"

ServerSocket::ServerSocket() {
    hasInit = false;
}

ServerSocket::ServerSocket(int port) {
    hasInit = false;
    Init(port);
}

void ServerSocket::Listen() {
    if (!hasInit) {
        Logger::LogAdd("ServerSocket", "WINSOCK NOT INITIALIZED YET!!!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        throw std::runtime_error("Attempted to start listening before initializing winsock");
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

void ServerSocket::Stop() {

}

Sockets ServerSocket::Accept() {
    // -- TODO: return a unique_ptr<Sockets> here.. maybe start tracking sockets inside this class using the sock descriptor.
    FD_ZERO(&mySockDescripts);
    FD_SET(listenSocket, &mySockDescripts);
    int activity = select(0, &mySockDescripts, NULL, NULL, reinterpret_cast<PTIMEVAL const>(10));
    if (FD_ISSET(listenSocket, &mySockDescripts)) {
        sockaddr_in from;
        int addrLen = sizeof(sockaddr_in);
        SOCKET clientSocket = accept(listenSocket, (struct sockaddr *) &from, &addrLen);
        char *ipAddr;
        ipAddr = inet_ntoa(from.sin_addr);

        std::cout << "Accepting.." << ipAddr << std::endl;

        if (clientSocket == INVALID_SOCKET)
            throw std::runtime_error("Failed to accept client socket");

        return Sockets(clientSocket);
    } else {
        return -1;
    }
}





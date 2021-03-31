//
// Created by Wande on 2/25/2021.
//

#ifndef __linux__
#include "network/WindowsSockets.h"


Sockets::Sockets(std::string address, std::string port)
{
    WSADATA wsaData;
    socketfd = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0) {
        std::cout << "Failed to init winsock" << std::endl;
    }

    ZeroMemory(&host_info, sizeof(host_info));
    hints.ai_family = AF_UNSPEC; // -- IPV4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // -- TCP Socket
    hints.ai_protocol = IPPROTO_TCP; // -- TCP Protocol

    iResult = getaddrinfo(address.c_str(), port.c_str(), &host_info, &host_info_list);

    if (iResult != 0) {
        std::cout << "Failed to do a DNS Lookup" << std::endl;
    }

    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
}

bool Sockets::Connect() {
    int iResult;
    iResult = connect(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);

    if (iResult == SOCKET_ERROR) {
        closesocket(socketfd);
        return false;
    }

    connected = true;
    return true;
}

void Sockets::Disconnect() {
    if (connected) {
        shutdown(socketfd, SD_SEND);
        closesocket(socketfd);
        connected = false;
    }
}

int Sockets::Read(char* buffer, int size) {
    int bytesRecv;
    bytesRecv = recv(socketfd, buffer, size, 0);

    if (bytesRecv == -1) {
        connected = false;
    }

    return bytesRecv;
}

int Sockets::Send(char* data, int size) {
    int bytesSend;
    bytesSend = send(socketfd, data, size, 0);

    if (bytesSend == -1) {
        connected = false;
    }

    return bytesSend;
}

Sockets::~Sockets()
{
    Disconnect();
    WSACleanup();
}

Sockets::Sockets(SOCKET accept) {
    socketfd = accept;
    connected = true;
}

Sockets::Sockets() {

}

SOCKET Sockets::GetSocketFd() {
    return socketfd;
}

#endif
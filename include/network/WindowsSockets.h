//
// Created by Wande on 2/25/2021.
//
#ifndef __linux__
#pragma once
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif

#ifndef D3PP_WINDOWSSOCKETS_H
#define D3PP_WINDOWSSOCKETS_H

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <exception>

class Sockets
{
public:
    Sockets();
    Sockets(SOCKET accept);
    Sockets(std::string address, std::string port);
    virtual ~Sockets();
    bool Connect();
    void Disconnect();
    int Read(char* buffer, int size);
    int Send(char* data, int size);
private:
    bool connected;
    SOCKET socketfd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
};

#endif //D3PP_WINDOWSSOCKETS_H
#endif

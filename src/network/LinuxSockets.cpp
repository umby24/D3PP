//
// Created by Wande on 2/25/2021.
//

#ifdef __linux__
#include "network/LinuxSockets.h"
#include <cerrno>
#include <iostream>
#include <cstring>      // Needed for memset

Sockets::Sockets(const std::string& address, const std::string& port) : host_info(), host_info_list()
{
    //ctor
    m_Counter = 0;
    connected = false;
    socketIp = "";
    memset(&host_info, 0, sizeof host_info);
    host_info.ai_family = AF_UNSPEC;     // IPv4 or IPv6
    host_info.ai_socktype = SOCK_STREAM; // TCP
    // DNS Lookup the host, results are pushed into host_info struct.
    status = getaddrinfo(address.c_str(), port.c_str(), &host_info, &host_info_list);
    // getaddrinfo returns 0 on success, or some other value when an error occurred.
    // (translated into human readable text by the gai_gai_strerror function).
    if (status != 0)
        std::cout << "getaddrinfo error" << gai_strerror(status) ;
     // The socket descriptor
    // socket(IPv4/6?, TCP/UDP?, Protocol)
    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                      host_info_list->ai_protocol);
}

bool Sockets::Connect() {
    // Connect(socketID, IP, IPLength)
    status = connect(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);

    if (status == -1) {// -- Error occurred trying to connect.
        int errsv = errno;
        std::cout << "Error Number: " << errsv << std::endl;
        return false;
    }
    connected = true;
    return true;
}

void Sockets::Disconnect() {
    if (connected) {
        close(socketfd);
        connected = false;
    }
}

int Sockets::Read(char* buffer, int size) {
    ssize_t bytesRead;
    // -- Receive from socketfd, into buffer, of max length 'size'

    bytesRead = recv(socketfd, buffer, (size_t)size, 0);

    if (bytesRead == -1)
        connected = false;

    return (int)bytesRead;
}

int Sockets::Send(char* data, int size) {
    ssize_t bytesSent;
    bytesSent = send(socketfd, data, (size_t)size, MSG_NOSIGNAL);

    if (bytesSent == -1)
        connected = false;

    return (int)bytesSent;
}

Sockets::~Sockets()
{
    //dtor
    Disconnect();
    if (host_info_list != nullptr)
        freeaddrinfo(host_info_list);
}

Sockets::Sockets() {
 connected = false;
 status = 0;
 socketfd = -1;
 host_info_list = nullptr;
 m_Counter = 0;
 socketIp = "";
}

Sockets::Sockets(int acceptfd, std::string clientIp) {
    socketfd = acceptfd;
    connected = true;
    status = 0;
    socketIp = "";
    host_info_list = nullptr;
    socketIp = clientIp;
}

std::string Sockets::GetSocketIp() {
    return socketIp;
}

void Sockets::SetSocketIp(std::string toSet) {
    socketIp = toSet;
}

#endif

//
// Created by Wande on 2/25/2021.
//
#ifdef __linux__
#ifndef D3PP_LINUXSOCKETS_H
#define D3PP_LINUXSOCKETS_H

#include <string>
#include <cstring>      // Needed for memset
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <iostream>

class Sockets
{
    public:
        Sockets(std::string address, std::string port);
        virtual ~Sockets();
        bool Connect();
        void Disconnect();
        int Read(char* buffer, int size);
        int Send(char* data, int size);

        // -- Getters
        unsigned int GetCounter() { return m_Counter; }
        int GetStatus() { return status; }
        bool GetConnected() { return connected; }
        // -- Setters
        void SetCounter(int val) { m_Counter = val; }
    protected:
    private:
        bool connected;
        int status;
        int socketfd;
        struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
        struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.
        unsigned int m_Counter;
};

#endif //D3PP_LINUXSOCKETS_H
#endif
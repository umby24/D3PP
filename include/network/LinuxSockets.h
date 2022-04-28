//
// Created by Wande on 2/25/2021.
//
#ifdef __linux__
#ifndef D3PP_LINUXSOCKETS_H
#define D3PP_LINUXSOCKETS_H

#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

class Sockets
{
    public:
        Sockets();
        Sockets(int acceptfd, std::string clientIp);
        Sockets(const std::string& address, const std::string& port);
        virtual ~Sockets();
        bool Connect();
        void Disconnect();
        int Read(char* buffer, int size);
        int Send(char* data, int size);

        // -- Getters
        unsigned int GetCounter() { return m_Counter; }
        int GetStatus() { return status; }
        bool GetConnected() { return connected; }
        int GetSocketFd() { return socketfd; }
        // -- Setters
        void SetCounter(int val) { m_Counter = val; }
        std::string GetSocketIp();
        void SetSocketIp(std::string toSet);
    protected:
    private:
        bool connected;
        int status;
        int socketfd;
        std::string socketIp;
        struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
        struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.
        unsigned int m_Counter;
};

#endif //D3PP_LINUXSOCKETS_H
#endif
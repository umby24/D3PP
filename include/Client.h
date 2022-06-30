//
// Created by Wande on 3/31/2021.
//

#ifndef D3PP_CLIENT_H
#define D3PP_CLIENT_H

#include <string>

class Client {
public:
    static void Login(int clientId, std::string name, std::string mppass, char version);
    static void LoginCpe(int clientId, std::string name, std::string mppass, char version);
    static void Logout(int clientId, std::string message, bool showtoall);
};
#endif //D3PP_CLIENT_H

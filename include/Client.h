//
// Created by Wande on 3/31/2021.
//

#ifndef D3PP_CLIENT_H
#define D3PP_CLIENT_H
#include <string>
#include "Network.h"
#include "Player.h"
#include "Network_Functions.h"
#include "Map.h"
#include "watchdog.h"


class Client {
public:
    static void Login(int clientId, std::string name, std::string mppass, char version);
    static void Logout(int clientId, std::string message, bool showtoall);
    static void LoginThread();
};
#endif //D3PP_CLIENT_H

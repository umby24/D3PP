//
// Created by Wande on 3/17/2021.
//

#ifndef D3PP_NETWORK_FUNCTIONS_H
#define D3PP_NETWORK_FUNCTIONS_H
#include <string>

#include "common/MinecraftLocation.h"

class NetworkFunctions {
public:
    static void SystemLoginScreen(const int& clientId, const std::string& message0, const std::string& message1, const char& opMode);
    static void SystemRedScreen(const int& clientId, const std::string& message);
    static void SystemMessageNetworkSend(const int& clientId,const  std::string& message, const int& type = 0);
    static void SystemMessageNetworkSend2All(const int& mapId, const std::string& message, const int& type = 0);
    // -- Map
    static void NetworkOutBlockSet(const int& clientId, const short& x, const short& y, const short& z, const unsigned char& type);
    static void NetworkOutBlockSet2Map(const int& mapId, const unsigned short& x, const unsigned short& y, const unsigned short& z, const unsigned char& type);
    // -- Player
    static void NetworkOutEntityAdd(const int& clientId, const char& playerId, const std::string& name, const MinecraftLocation& location);
    static void NetworkOutEntityDelete(const int& clientId, const char& playerId);
    static void NetworkOutEntityPosition(const int& clientId, const char& playerId, const MinecraftLocation& location);
};
#endif //D3PP_NETWORK_FUNCTIONS_H

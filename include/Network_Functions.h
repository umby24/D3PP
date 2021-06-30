//
// Created by Wande on 3/17/2021.
//

#ifndef D3PP_NETWORK_FUNCTIONS_H
#define D3PP_NETWORK_FUNCTIONS_H
#include <string>

class NetworkFunctions {
public:
    static void SystemLoginScreen(int clientId, std::string message0, std::string message1, char opMode);
    static void SystemRedScreen(int clientId, std::string message);
    static void SystemMessageNetworkSend(int clientId, std::string message, int type = 0);
    static void SystemMessageNetworkSend2All(int mapId, std::string message, int type = 0);
    // -- Map
    static void NetworkOutBlockSet(int clientId, short x, short y, short z, char type);
    static void NetworkOutBlockSet2Map(int mapId, short x, short y, short z, char type);
    // -- Player
    static void NetworkOutEntityAdd(int clientId, char playerId, std::string name, float x, float y, float z, float rotation, float look);
    static void NetworkOutEntityDelete(int clientId, char playerId);
    static void NetworkOutEntityPosition(int clientId, char playerId, float x, float y, float z, float rotation, float look);
};
#endif //D3PP_NETWORK_FUNCTIONS_H

//
// Created by Wande on 3/17/2021.
//

#include "Network_Functions.h"

static shared_ptr<NetworkClient> GetPlayer(int id) {
    auto network = Network::GetInstance();
    auto result = network->GetClient(id);
    return result;
}

void NetworkFunctions::SystemLoginScreen(int clientId, std::string message0, std::string message1, char opMode) {
    message0 = Chat::StringGV(message0);
    message1 = Chat::StringGV(message1);
    Utils::padTo(message0, 64);
    Utils::padTo(message1, 64);
    std::string blankie;
    Utils::padTo(blankie, 64);
    if (message0 != blankie && message1 != blankie) {
        Packets::SendClientHandshake(clientId, 7, message0, message1, opMode);
    }
}

void NetworkFunctions::SystemRedScreen(int clientId, std::string message) {
    message = Chat::StringGV(message);
    Utils::padTo(message, 64);
    std::string blankie;
    Utils::padTo(blankie, 64);

    if (message != blankie) {
        Packets::SendDisconnect(clientId, message);
    }
}

void NetworkFunctions::SystemMessageNetworkSend(int clientId, std::string message, int type) {
    Utils::replaceAll(message, "<br>", "\n");
    message = Chat::StringMultiline(message);
    message = Chat::StringGV(message);
    // -- emote replace
    for (auto i = 1; i < 20; i++) {

    }
}

void NetworkFunctions::SystemMessageNetworkSend2All(int mapId, std::string message, int type) {

}

void NetworkFunctions::NetworkOutBlockSet(int clientId, short x, short y, short z, char type) {

}

void NetworkFunctions::NetworkOutBlockSet2Map(int mapId, short x, short y, short z, char type) {

}

void NetworkFunctions::NetworkOutEntityAdd(int clientId, char playerId, std::string name, float x, float y, float z,
                                           float rotation, float look) {

}

void NetworkFunctions::NetworkOutEntityDelete(int clientId, char playerId) {

}

void NetworkFunctions::NetworkOutEntityPosition(int clientId, char playerId, float x, float y, float z, float rotation,
                                                float look) {

}

//
// Created by Wande on 3/17/2021.
//

#include "Network_Functions.h"
#include "Chat.h"
#include "Utils.h"
#include "Packets.h"
#include "Network.h"
#include "Block.h"
#include "Player.h"

static std::shared_ptr<NetworkClient> GetPlayer(int id) {
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
    std::string blankie;
    Utils::padTo(blankie, 64);
    int lines = Utils::strCount(message, '\n') + 1;
    std::vector<std::string> linev = Utils::splitString(message, '\n');
    // -- emote replace
    for (auto i = 0; i < lines; i++) {
        std::string text = linev.at(i);
        Utils::padTo(text, 64);
        
        if (!text.empty() && text != blankie) {
            Packets::SendChatMessage(clientId, text, type);
        }
    }
}

void NetworkFunctions::SystemMessageNetworkSend2All(int mapId, std::string message, int type) {
    Network* n = Network::GetInstance();
    Utils::replaceAll(message, "<br>", "\n");
    message = Chat::StringMultiline(message);
    message = Chat::StringGV(message);
    std::string blankie;
    Utils::padTo(blankie, 64);
    int lines = Utils::strCount(message, '\n') + 1;
    std::vector<std::string> linev = Utils::splitString(message, '\n');
    // -- emote replace
    for (auto i = 0; i < lines; i++) {
        std::string text = linev.at(i);
        Utils::padTo(text, 64);
        if (!text.empty() && text != blankie) {
            for (auto const &nc : n->_clients) {
                if (mapId == -1 || nc.second->player->MapId == mapId) {
                    Packets::SendChatMessage(nc.first, text, type);
                }
            }
        }
    }
}

void NetworkFunctions::NetworkOutBlockSet(int clientId, short x, short y, short z, char type) {
    Network* n = Network::GetInstance();
    Block* b = Block::GetInstance();
    MapBlock mb = b->GetBlock(type);
    std::shared_ptr<NetworkClient> nc = n->GetClient(clientId);
    if (nc->LoggedIn) {
        if (mb.CpeLevel > nc->CustomBlocksLevel) {
            type = mb.CpeReplace;
        } else {
            type = mb.OnClient;
        }
        Packets::SendBlockChange(clientId, x, y, z, type);
    }
}

void NetworkFunctions::NetworkOutBlockSet2Map(int mapId, short x, short y, short z, char type) {
    Network* n = Network::GetInstance();
    Block* b = Block::GetInstance();
    MapBlock mb = b->GetBlock(type);
    for(auto const &nc : n->_clients) {
        if (nc.second->player->MapId != mapId || !nc.second->LoggedIn)
            continue;

        if (mb.CpeLevel > nc.second->CustomBlocksLevel) {
            type = mb.CpeReplace;
        } else {
            type = mb.OnClient;
        }

        Packets::SendBlockChange(nc.first, x, y, z, type);
    }
}

void NetworkFunctions::NetworkOutEntityAdd(int clientId, char playerId, std::string name, float x, float y, float z,
                                           float rotation, float look) {
    // -- TODO: ExtPlayerList2
    Utils::padTo(name, 64);
    x *= 32;
    y *= 32;
    z = (z *32) + 51;
    rotation = rotation/360*256.0;
    look = look/360*256.0;
    Packets::SendSpawnEntity(clientId, playerId, name, x, y, z, rotation, look);
}

void NetworkFunctions::NetworkOutEntityDelete(int clientId, char playerId) {
    Packets::SendDespawnEntity(clientId, playerId);
}

void NetworkFunctions::NetworkOutEntityPosition(int clientId, char playerId, float x, float y, float z, float rotation,
                                                float look) {
    x *= 32;
    y *= 32;
    z = (z * 32) + 51;
    rotation = rotation/360*256.0;
    look = look/360*256.0;
    Packets::SendPlayerTeleport(clientId, playerId, x, y, z, rotation, look);
}

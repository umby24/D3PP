//
// Created by Wande on 3/17/2021.
//

#include "network/Network_Functions.h"
#include "network/Chat.h"
#include "Utils.h"
#include "network/Packets.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "Block.h"
#include "world/Player.h"
#include "CPE.h"
#include "common/MinecraftLocation.h"

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
    Utils::replaceAll(message, "\n", "");
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
    Utils::replaceAll(message, "\n", "");
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
            for (auto const &nc : n->roClients) {
                if (!nc->LoggedIn || nc->player == nullptr) {
                    continue;
                }
                if (mapId == -1 || nc->player->MapId == mapId) {
                    Packets::SendChatMessage(nc->GetId(), text, type);
                }
            }
        }
    }
}

void NetworkFunctions::NetworkOutBlockSet(int clientId, short x, short y, short z, unsigned char type) {
    Network* n = Network::GetInstance();
    Block* b = Block::GetInstance();
    MapBlock mb = b->GetBlock(type);
    std::shared_ptr<NetworkClient> nc = std::static_pointer_cast<NetworkClient>(n->GetClient(clientId));

    if (nc->LoggedIn) {
        if (mb.CpeLevel > nc->CustomBlocksLevel) {
            type = mb.CpeReplace;
        } else {
            type = mb.OnClient;
        }
        Packets::SendBlockChange(clientId, x, y, z, type);
    }
}

void NetworkFunctions::NetworkOutBlockSet2Map(int mapId, unsigned short x, unsigned short y, unsigned short z, unsigned char type) {
    Network* n = Network::GetInstance();
    Block* b = Block::GetInstance();
    MapBlock mb = b->GetBlock(type);

    for(auto const &nc : n->roClients) {
        if (nc->player != NULL && nc->player->MapId != mapId || !nc->LoggedIn)
            continue;

        int onClient = mb.OnClient;
        if (mb.CpeLevel > nc->CustomBlocksLevel) {
            onClient = mb.CpeReplace;
        }

        Packets::SendBlockChange(nc->GetId(), x, y, z, onClient);
    }
}

void NetworkFunctions::NetworkOutEntityAdd(int clientId, char playerId, std::string name, MinecraftLocation& location) {
    Network* nm = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> c = nm->GetClient(clientId);

    Utils::padTo(name, 64);

    unsigned char rotation = location.Rotation/360*256.0;
    unsigned char look = location.Look/360*256.0;
    if (CPE::GetClientExtVersion(c, EXT_PLAYER_LIST_EXT_NAME) < 2) {
        Packets::SendSpawnEntity(clientId, playerId, name, location.X(), location.Y(), location.Z(), rotation, look);
    } else {
        Packets::SendExtAddEntity2(std::static_pointer_cast<NetworkClient>(c), static_cast<unsigned char>(playerId), name, name, location.X(), location.Y(), location.Z(), rotation, look);
    }
}

void NetworkFunctions::NetworkOutEntityDelete(int clientId, char playerId) {
    Packets::SendDespawnEntity(clientId, playerId);
}

void NetworkFunctions::NetworkOutEntityPosition(int clientId, char playerId, MinecraftLocation& location) {
    Packets::SendPlayerTeleport(clientId, playerId, location.X(), location.Y(), location.Z(), location.Rotation, location.Look);
}

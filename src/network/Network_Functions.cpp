//
// Created by Wande on 3/17/2021.
//

#include "network/Network_Functions.h"
#include "network/Chat.h"
#include "Utils.h"
#include "network/Packets.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/packets/ChatPacket.h"
#include "network/Server.h"

#include "Block.h"
#include "world/Player.h"
#include "CPE.h"
#include "common/MinecraftLocation.h"

void NetworkFunctions::SystemLoginScreen(const int& clientId, const std::string& message0, const std::string& message1, const char& opMode) {
    std::string sanitized0 = Chat::StringGV(message0);
    std::string sanitized1 = Chat::StringGV(message1);

    Packets::SendClientHandshake(clientId, 7, message0, message1, opMode);
}

void NetworkFunctions::SystemRedScreen(const int& clientId, const std::string& message) {
    std::string sanitized = Chat::StringGV(message);

    Packets::SendDisconnect(clientId, message);
}

std::string SanitizeMessageString(const std::string &message) {
    std::string sanitized(message);
    Utils::replaceAll(sanitized, "\n", "");
    Utils::replaceAll(sanitized, "<br>", "\n");
    sanitized = Chat::StringMultiline(sanitized);
    sanitized = Chat::StringGV(sanitized);
    return sanitized;
}

void NetworkFunctions::SystemMessageNetworkSend(const int& clientId, const std::string& message, const int& type) {
    if (clientId == CONSOLE_CLIENT_ID) {
        std::shared_ptr<IMinecraftClient> c = Network::GetClient(CONSOLE_CLIENT_ID);
        c->SendChat(message);
        return;
    }

    std::string sanitized = SanitizeMessageString(message);
    int lines = Utils::strCount(sanitized, '\n') + 1;
    std::vector<std::string> linev = Utils::splitString(sanitized, '\n');
    // -- emote replace
    for (auto i = 0; i < lines; i++) {
        std::string text = linev.at(i);
        
        if (!text.empty()) {
            Packets::SendChatMessage(clientId, text, static_cast<char>(type));
        }
    }
}

void NetworkFunctions::SystemMessageNetworkSend2All(const int& mapId, const std::string& message, const int& type) {
    std::string sanitized = SanitizeMessageString(message);
    int lines = Utils::strCount(sanitized, '\n') + 1;
    std::vector<std::string> linev = Utils::splitString(sanitized, '\n');
    // -- emote replace
    for (auto i = 0; i < lines; i++) {
        std::string text = linev.at(i);

        if (!text.empty()) {
            D3PP::network::ChatPacket thisP(static_cast<char>(type), text);

            if (mapId == -1)
                D3PP::network::Server::SendToAll(thisP, "", 0);

        }
    }
}

void NetworkFunctions::NetworkOutBlockSet(const int& clientId, const short& x, const short& y, const short& z, const unsigned char& type) {
    Network* n = Network::GetInstance();
    Block* b = Block::GetInstance();
    MapBlock mb = b->GetBlock(type);
    std::shared_ptr<NetworkClient> nc = std::static_pointer_cast<NetworkClient>(n->GetClient(clientId));
    unsigned char newType(type);

    if (nc->LoggedIn) {
        if (mb.CpeLevel > nc->CustomBlocksLevel) {
            newType = mb.CpeReplace;
        } else {
            newType = mb.OnClient;
        }
        Packets::SendBlockChange(clientId, x, y, z, newType);
    }
}

void NetworkFunctions::NetworkOutBlockSet2Map(const int& mapId, const unsigned short& x, const unsigned short& y, const unsigned short& z, const unsigned char& type) {
    Block* b = Block::GetInstance();
    MapBlock mb = b->GetBlock(type);

    for(auto const &nc : D3PP::network::Server::roClients) {
        if (nc->player != NULL && nc->GetMapId() != mapId || !nc->LoggedIn)
            continue;

        int onClient = mb.OnClient;

        if (mb.CpeLevel > nc->GetCustomBlocksLevel()) {
            onClient = mb.CpeReplace;
        }

        Packets::SendBlockChange(nc->GetId(), x, y, z, onClient);
    }
}

void NetworkFunctions::NetworkOutEntityAdd(const int& clientId, const char& playerId, const std::string& name, const MinecraftLocation& location) {
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(clientId);

    auto rotation = static_cast<unsigned char>((location.Rotation/360)*256.0);
    auto look = static_cast<unsigned char>((location.Look/360)*256.0);

    if (CPE::GetClientExtVersion(c, EXT_PLAYER_LIST_EXT_NAME) < 2) {
        Packets::SendSpawnEntity(clientId, playerId, name, location.X(), location.Y(), location.Z(), static_cast<char>(rotation), static_cast<char>(look));
    } else {
        Packets::SendExtAddEntity2(std::static_pointer_cast<NetworkClient>(c), static_cast<unsigned char>(playerId), name, name, location.X(), location.Y(), location.Z(), rotation, look);
    }
}

void NetworkFunctions::NetworkOutEntityDelete(const int& clientId, const char& playerId) {
    Packets::SendDespawnEntity(clientId, playerId);
}

void NetworkFunctions::NetworkOutEntityPosition(const int& clientId, const char& playerId, const MinecraftLocation& location) {
    auto rotation = static_cast<unsigned char>((location.Rotation / 360) * 256.0);
    auto look = static_cast<unsigned char>((location.Look / 360) * 256.0);
    Packets::SendPlayerTeleport(clientId, playerId, location.X(), location.Y(), location.Z(), static_cast<char>(rotation), static_cast<char>(look));
}

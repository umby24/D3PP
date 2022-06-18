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

void NetworkFunctions::SystemLoginScreen(const int& clientId, const std::string& message0, const std::string& message1, const char& opMode) {
    std::string sanitized0 = Chat::StringGV(message0);
    std::string sanitized1 = Chat::StringGV(message1);
    Chat::EmoteReplace(sanitized0);
    Chat::EmoteReplace(sanitized1);
    Packets::SendClientHandshake(clientId, 7, message0, message1, opMode);
}

void NetworkFunctions::SystemRedScreen(const int& clientId, const std::string& message) {
    std::string sanitized = Chat::StringGV(message);
    Chat::EmoteReplace(sanitized);
    Packets::SendDisconnect(clientId, message);
}

void NetworkFunctions::SystemMessageNetworkSend(const int& clientId, const std::string& message, const int& type) {
    if (clientId == CONSOLE_CLIENT_ID) {
        Network* n = Network::GetInstance();
        std::shared_ptr<IMinecraftClient> c = n->GetClient(CONSOLE_CLIENT_ID);
        c->SendChat(message);
        return;
    }

    std::string sanitized(message);
    Utils::replaceAll(sanitized, "\n", "");
    Utils::replaceAll(sanitized, "<br>", "\n");
    sanitized = Chat::StringMultiline(sanitized);
    sanitized = Chat::StringGV(sanitized);
    Chat::EmoteReplace(sanitized);
    int lines = Utils::strCount(sanitized, '\n') + 1;
    std::vector<std::string> linev = Utils::splitString(sanitized, '\n');
    // -- emote replace
    for (auto i = 0; i < lines; i++) {
        std::string text = linev.at(i);
        
        if (!text.empty()) {
            Packets::SendChatMessage(clientId, text, type);
        }
    }
}

void NetworkFunctions::SystemMessageNetworkSend2All(const int& mapId, const std::string& message, const int& type) {
    Network* n = Network::GetInstance();
    std::string sanitized(message);
    Utils::replaceAll(sanitized, "\n", "");
    Utils::replaceAll(sanitized, "<br>", "\n");
    sanitized = Chat::StringMultiline(sanitized);
    sanitized = Chat::StringGV(sanitized);
    Chat::EmoteReplace(sanitized);
    int lines = Utils::strCount(sanitized, '\n') + 1;
    std::vector<std::string> linev = Utils::splitString(sanitized, '\n');
    // -- emote replace
    for (auto i = 0; i < lines; i++) {
        std::string text = linev.at(i);

        if (!text.empty()) {
            for (auto const &nc : n->roClients) {
                if (!nc->LoggedIn || nc->player == nullptr)
                    continue;

                if (mapId == -1 || nc->player->MapId == mapId) {
                    Packets::SendChatMessage(nc->GetId(), text, type);
                }
            }
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
    Network* n = Network::GetInstance();
    Block* b = Block::GetInstance();
    MapBlock mb = b->GetBlock(type);

    for(auto const &nc : n->roClients) {
        if (nc->player != nullptr && nc->player->MapId != mapId || !nc->LoggedIn)
            continue;

        int onClient = mb.OnClient;
        int dbl = CPE::GetClientExtVersion(nc, BLOCK_DEFS_EXT_NAME);
        if (mb.CpeLevel > nc->CustomBlocksLevel) {
            onClient = mb.CpeReplace;
        }
        if (mb.OnClient > 65 && dbl < 1) { // -- If the user doesn't support customblocks and this is a custom block, replace with stone.
            onClient = 1;
            continue;
        }
        Packets::SendBlockChange(nc->GetId(), x, y, z, onClient);
    }
}

void NetworkFunctions::NetworkOutEntityAdd(const int& clientId, const char& playerId, const std::string& name, const MinecraftLocation& location) {
    Network* nm = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> c = nm->GetClient(clientId);

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

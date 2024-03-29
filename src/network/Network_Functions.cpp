//
// Created by Wande on 3/17/2021.
//

#include "common/Vectors.h"
#include "network/Network_Functions.h"
#include "network/Chat.h"
#include "network/Server.h"
#include "Utils.h"
#include "network/Packets.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/packets/BlockChangePacket.h"
#include "network/packets/SpawnEffectPacket.h"
#include "network/IPacket.h"
#include "world/Entity.h"
#include "Block.h"
#include "world/Player.h"
#include "CPE.h"

std::string SanitizeMessageString(const std::string &message) {
    std::string sanitized(message);
    Utils::replaceAll(sanitized, "\n", "");
    Utils::replaceAll(sanitized, "<br>", "\n");
    sanitized = Chat::StringMultiline(sanitized);
    sanitized = Chat::StringGV(sanitized);
    return sanitized;
}

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

    std::string sanitized = SanitizeMessageString(message);
    Chat::EmoteReplace(sanitized);
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
    Network* n = Network::GetInstance();
    std::string sanitized = SanitizeMessageString(message);
    Chat::EmoteReplace(sanitized);
    int lines = Utils::strCount(sanitized, '\n') + 1;
    std::vector<std::string> linev = Utils::splitString(sanitized, '\n');
    // -- emote replace
    for (auto i = 0; i < lines; i++) {
        std::string text = linev.at(i);

        if (!text.empty()) {
            std::shared_lock lock(D3PP::network::Server::roMutex);
            for (auto const &nc : D3PP::network::Server::roClients) {
                if (!nc->GetLoggedIn() || nc->GetPlayerInstance() == nullptr)
                    continue;

                if (mapId == -1 || nc->GetPlayerInstance()->GetEntity()->MapID == mapId) {
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
void NetworkFunctions::PacketToMap(const int& mapId, D3PP::network::IPacket& p, std::string reqExt, int reqVer) {
    for(auto const &nc : D3PP::network::Server::roClients) {
        if (!nc->GetLoggedIn() || nc->GetPlayerInstance() == nullptr)
            continue;

        if (nc->GetPlayerInstance()->GetEntity()->MapID != mapId)
            continue;

        if (reqExt.empty() || CPE::GetClientExtVersion(nc, reqExt) == reqVer) {
            nc->SendPacket(p);
        }
    }
}

void NetworkFunctions::NetworkOutBlockSet2Map(const int& mapId, const unsigned short& x, const unsigned short& y, const unsigned short& z, const unsigned char& type) {
    Block* b = Block::GetInstance();
    MapBlock mb = b->GetBlock(type);

    std::shared_lock lock(D3PP::network::Server::roMutex);
    for(auto const &nc : D3PP::network::Server::roClients) {
        if (!nc->GetLoggedIn() || nc->GetPlayerInstance() == nullptr)
            continue;

        if (nc->GetPlayerInstance()->GetEntity()->MapID != mapId)
            continue;

        int onClient = mb.OnClient;
        int dbl = CPE::GetClientExtVersion(nc, BLOCK_DEFS_EXT_NAME);
        if (mb.CpeLevel > nc->GetCustomBlocksLevel()) {
            onClient = mb.CpeReplace;
        }
        if (mb.OnClient > 65 && dbl < 1) { // -- If the user doesn't support customblocks and this is a custom block, replace with stone.
            onClient = 1;
            continue;
        }

        D3PP::network::BlockChangePacket p(D3PP::Common::Vector3S(x, y, z), 0, onClient);
        nc->SendPacket(p);
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

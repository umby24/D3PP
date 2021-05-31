//
// Created by Wande on 3/30/2021.
//

#include "PacketHandlers.h"

void PacketHandlers::HandleHandshake(const std::shared_ptr<NetworkClient>& client) {
    client->InputAddOffset(1);
    char clientVersion = client->InputReadByte();
    std::string clientName = client->InputReadString();
    std::string mppass = client->InputReadString();
    char isCpe = client->InputReadByte();
        Utils::TrimString(clientName);
    if (!client->LoggedIn && client->DisconnectTime == 0 && isCpe != 66) {
        Logger::LogAdd("PacketHandler", "Womg, incoming: " + clientName, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
        Client::Login(client->Id, clientName, mppass, clientVersion);
    } else if (isCpe == 66 && !client->LoggedIn && client->DisconnectTime == 0) {
        Logger::LogAdd("PacketHandler", "Womg, CPE incoming: " + clientName, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
        Client::Login(client->Id, clientName, mppass, clientVersion);
    }
}

void PacketHandlers::HandlePing(const shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
    client->Ping = time(nullptr) - client->PingSentTime;
}

void PacketHandlers::HandleBlockChange(const shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
    short X = client->InputReadShort();
    short Z = client->InputReadShort();
    short Y = client->InputReadShort();
    char Mode = (client->InputReadByte() & 255);
    char Type = (client->InputReadByte() & 255);

    if (!client->LoggedIn || !client->player->tEntity)
        return;

    // -- TODO: BuildModeDistrubute
}

void PacketHandlers::HandlePlayerTeleport(const shared_ptr<NetworkClient> &client) {
    // -- CPE :)
    client->InputAddOffset(2);
    short X = client->InputReadShort();
    short Z = client->InputReadShort();
    short Y = client->InputReadShort();
    char R = client->InputReadByte();
    char L = client->InputReadByte();

    if (!client->LoggedIn || !client->player->tEntity)
        return;

    if (client->player->tEntity->MapID == client->player->MapId)
        client->player->tEntity->PositionSet(client->player->tEntity->MapID, X/32, Y/32, (Z-51)/32, R*360/256, L*360/256, 1, false);
}

void PacketHandlers::HandleChatPacket(const shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
    char playerId = client->InputReadByte();
    std::string message = client->InputReadString();
    if (client->LoggedIn && client->player->tEntity) {
        Chat::HandleIncomingChat(client, message, playerId);
    }
}

void PacketHandlers::HandleExtInfo(const shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
}

void PacketHandlers::HandleExtEntry(const shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
}

void PacketHandlers::HandleCustomBlockSupportLevel(const shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
}

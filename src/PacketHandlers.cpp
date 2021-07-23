//
// Created by Wande on 3/30/2021.
//

#include "PacketHandlers.h"

#include "Chat.h"
#include "Network.h"
#include "Client.h"
#include "Player.h"
#include "Entity.h"
#include "Logger.h"
#include "BuildMode.h"
#include "CPE.h"
#include "Utils.h"

void PacketHandlers::HandleHandshake(const std::shared_ptr<NetworkClient>& client) {
    client->InputAddOffset(1);
    char clientVersion = client->InputReadByte();
    std::string clientName = client->InputReadString();
    std::string mppass = client->InputReadString();
    char isCpe = client->InputReadByte();
        Utils::TrimString(clientName);

    if (!client->LoggedIn && client->DisconnectTime == 0 && isCpe != 66) {
        Client::Login(client->Id, clientName, mppass, clientVersion);
    } else if (isCpe == 66 && !client->LoggedIn && client->DisconnectTime == 0) { // -- CPE capable Client
        Logger::LogAdd("Network", "CPE Client Detected", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
        Client::LoginCpe(client->Id, clientName, mppass, clientVersion); 
    }
}

void PacketHandlers::HandlePing(const std::shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
    client->Ping = time(nullptr) - client->PingSentTime;
}

void PacketHandlers::HandleBlockChange(const std::shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
    short X = client->InputReadShort();
    short Z = client->InputReadShort();
    short Y = client->InputReadShort();
    char Mode = (client->InputReadByte() & 255);
    char Type = (client->InputReadByte() & 255);

    if (!client->LoggedIn || !client->player->tEntity)
        return;

    BuildModeMain* bmm = BuildModeMain::GetInstance();
    bmm->Distribute(client->Id, client->player->tEntity->MapID, X, Y, Z, (Mode > 0), Type);
}

void PacketHandlers::HandlePlayerTeleport(const std::shared_ptr<NetworkClient> &client) {
    // -- CPE :)
    if (CPE::GetClientExtVersion(client, HELDBLOCK_EXT_NAME) == 1) {
        client->InputAddOffset(1);
        if (client->player->tEntity != nullptr)
            client->player->tEntity->heldBlock = client->InputReadByte();
        else
            client->InputAddOffset(1);
    } else {
        client->InputAddOffset(2);
    }

    unsigned short X = (unsigned short)client->InputReadShort();
    unsigned short Z = (unsigned short)client->InputReadShort();
    unsigned short Y = (unsigned short)client->InputReadShort();
    char R = client->InputReadByte();
    char L = client->InputReadByte();

    if (!client->LoggedIn || !client->player->tEntity)
        return;

    if (client->player->tEntity->MapID == client->player->MapId)
        client->player->tEntity->PositionSet(client->player->tEntity->MapID, X/32.0, Y/32.0, (Z-51)/32.0, R*360/256.0, L*360/256.0, 1, false);
}

void PacketHandlers::HandleChatPacket(const std::shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
    char playerId = client->InputReadByte();
    std::string message = client->InputReadString();
    Utils::TrimString(message);
    if (client->LoggedIn && client->player->tEntity) {
        Chat::HandleIncomingChat(client, message, playerId);
    }
}

void PacketHandlers::HandleExtInfo(const std::shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
    
    std::string appName = client->InputReadString();
    short extensions = client->InputReadShort();
    client->CPE = true;
    
    if (extensions == 0) {
        CPE::PreLoginExtensions(client);
    }
    
    client->CustomExtensions = extensions;
    Logger::LogAdd("CPE", "Client supports " + stringulate(extensions) + " extensions", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void PacketHandlers::HandleExtEntry(const std::shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
    std::string extName = client->InputReadString();
    int extVersion = client->InputReadInt();
    
    client->CustomExtensions--;
    client->Extensions.insert(std::make_pair(extName, extVersion));
    if (client->CustomExtensions == 0) {
        CPE::PreLoginExtensions(client);
    }
}

void PacketHandlers::HandleCustomBlockSupportLevel(const std::shared_ptr<NetworkClient> &client) {
    client->InputAddOffset(1);
    unsigned char supportLevel = client->InputReadByte();
    client->CustomBlocksLevel = supportLevel;

    Logger::LogAdd("CPE", "CPE Process complete.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    Client::Login(client->Id, client->player->LoginName, client->player->MPPass, client->player->ClientVersion);
}

//
// Created by Wande on 3/30/2021.
//

#include "PacketHandlers.h"

#include "Chat.h"
#include "Network.h"
#include "NetworkClient.h"
#include "Client.h"
#include "Player.h"
#include "Entity.h"
#include "Logger.h"
#include "BuildMode.h"
#include "CPE.h"
#include "Utils.h"
#include "common/ByteBuffer.h"
#include "Packets.h"

void PacketHandlers::HandleHandshake(const std::shared_ptr<NetworkClient>& client) {
    char clientVersion = client->ReceiveBuffer->ReadByte();
    std::string clientName = client->ReceiveBuffer->ReadString();
    std::string mppass = client->ReceiveBuffer->ReadString();
    char isCpe = client->ReceiveBuffer->ReadByte();
    Utils::TrimString(clientName);

    if (!client->LoggedIn && client->DisconnectTime == 0 && isCpe != 66) {
        Client::Login(client->Id, clientName, mppass, clientVersion);
    } else if (isCpe == 66 && !client->LoggedIn && client->DisconnectTime == 0) { // -- CPE capable Client
        Logger::LogAdd("Network", "CPE Client Detected", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
        Client::LoginCpe(client->Id, clientName, mppass, clientVersion); 
    }
}

void PacketHandlers::HandlePing(const std::shared_ptr<NetworkClient> &client) {
    client->Ping = static_cast<int>((std::chrono::steady_clock::now() - client->PingSentTime).count());
}

void PacketHandlers::HandleBlockChange(const std::shared_ptr<NetworkClient> &client) {
    short X = client->ReceiveBuffer->ReadShort();
    short Z = client->ReceiveBuffer->ReadShort();
    short Y = client->ReceiveBuffer->ReadShort();
    char Mode = (client->ReceiveBuffer->ReadByte() & 255);
    char Type = (client->ReceiveBuffer->ReadByte() & 255);

    if (!client->LoggedIn || !client->player->tEntity)
        return;

    BuildModeMain* bmm = BuildModeMain::GetInstance();
    bmm->Distribute(client->Id, client->player->tEntity->MapID, X, Y, Z, (Mode > 0), Type);
}

void PacketHandlers::HandlePlayerTeleport(const std::shared_ptr<NetworkClient> &client) {
    // -- CPE :)
    if (CPE::GetClientExtVersion(client, HELDBLOCK_EXT_NAME) == 1) {
        if (client->player->tEntity != nullptr)
            client->player->tEntity->heldBlock = client->ReceiveBuffer->ReadByte();
        else
            client->ReceiveBuffer->ReadByte();
    } else {
        client->ReceiveBuffer->ReadByte();
    }

    unsigned short X = (unsigned short)client->ReceiveBuffer->ReadShort();
    unsigned short Z = (unsigned short)client->ReceiveBuffer->ReadShort();
    unsigned short Y = (unsigned short)client->ReceiveBuffer->ReadShort();
    char R = client->ReceiveBuffer->ReadByte();
    char L = client->ReceiveBuffer->ReadByte();

    if (!client->LoggedIn || !client->player->tEntity)
        return;

    if (client->player->tEntity->MapID == client->player->MapId)
        client->player->tEntity->PositionSet(client->player->tEntity->MapID, X/32.0, Y/32.0, (Z-51)/32.0, R*360/256.0, L*360/256.0, 1, false);
}

void PacketHandlers::HandleChatPacket(const std::shared_ptr<NetworkClient> &client) {
    char playerId = client->ReceiveBuffer->ReadByte();
    std::string message = client->ReceiveBuffer->ReadString();
    Utils::TrimString(message);
    if (client->LoggedIn && client->player->tEntity) {
        Chat::HandleIncomingChat(client, message, playerId);
    }
}

void PacketHandlers::HandleExtInfo(const std::shared_ptr<NetworkClient> &client) {
    std::string appName = client->ReceiveBuffer->ReadString();
    short extensions = client->ReceiveBuffer->ReadShort();
    client->CPE = true;
    
    if (extensions == 0) {
        CPE::PreLoginExtensions(client);
    }
    
    client->CustomExtensions = extensions;
    Logger::LogAdd("CPE", "Client supports " + stringulate(extensions) + " extensions", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void PacketHandlers::HandleExtEntry(const std::shared_ptr<NetworkClient> &client) {
    std::string extName = client->ReceiveBuffer->ReadString();
    int extVersion = client->ReceiveBuffer->ReadInt();
    
    client->CustomExtensions--;
    client->Extensions.insert(std::make_pair(extName, extVersion));
    if (client->CustomExtensions == 0) {
        CPE::PreLoginExtensions(client);
    }
}

void PacketHandlers::HandleCustomBlockSupportLevel(const std::shared_ptr<NetworkClient> &client) {
    unsigned char supportLevel = client->ReceiveBuffer->ReadByte();
    client->CustomBlocksLevel = supportLevel;

    Logger::LogAdd("CPE", "CPE Process complete.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    Client::Login(client->Id, client->player->LoginName, client->player->MPPass, client->player->ClientVersion);
}

void PacketHandlers::HandleTwoWayPing(const std::shared_ptr<NetworkClient> &client) {
    unsigned char direction = client->ReceiveBuffer->ReadByte();
    short timeval = client->ReceiveBuffer->ReadShort();

    if (direction == 0) {
        Packets::SendTwoWayPing(client, direction, timeval);
        return;
    }

    short totalduration = static_cast<short>(clock() - timeval);
    float secondsTaken = (float)totalduration / static_cast<float>(CLOCKS_PER_SEC);
    client->Ping = secondsTaken;
}

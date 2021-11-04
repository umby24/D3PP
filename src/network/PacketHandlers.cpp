//
// Created by Wande on 3/30/2021.
//

#include "network/PacketHandlers.h"

#include "network/Chat.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "Client.h"
#include "world/Player.h"
#include "world/Entity.h"
#include "common/Logger.h"
#include "BuildMode.h"
#include "CPE.h"
#include "Utils.h"
#include "common/ByteBuffer.h"
#include "network/Packets.h"
#include "common/MinecraftLocation.h"

void PacketHandlers::HandleHandshake(const std::shared_ptr<NetworkClient>& client) {
    char clientVersion = client->ReceiveBuffer->ReadByte();
    std::string clientName = client->ReceiveBuffer->ReadString();
    std::string mppass = client->ReceiveBuffer->ReadString();
    char isCpe = client->ReceiveBuffer->ReadByte();
    Utils::TrimString(clientName);

    if (!client->LoggedIn && client->DisconnectTime == 0 && isCpe != 66) {
        Client::Login(client->GetId(), clientName, mppass, clientVersion);
    } else if (isCpe == 66 && !client->LoggedIn && client->DisconnectTime == 0) { // -- CPE capable Client
        Logger::LogAdd("Network", "CPE Client Detected", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
        Client::LoginCpe(client->GetId(), clientName, mppass, clientVersion);
    }
}

void PacketHandlers::HandlePing(const std::shared_ptr<NetworkClient> &client) {
    client->Ping = static_cast<float>((std::chrono::steady_clock::now() - client->PingSentTime).count());
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
    bmm->Distribute(client->GetId(), client->player->tEntity->MapID, X, Y, Z, (Mode > 0), Type);
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

    auto X = (unsigned short)client->ReceiveBuffer->ReadShort();
    auto Z = (unsigned short)client->ReceiveBuffer->ReadShort();
    auto Y = (unsigned short)client->ReceiveBuffer->ReadShort();
    char R = client->ReceiveBuffer->ReadByte();
    char L = client->ReceiveBuffer->ReadByte();
    float rot = (R / 255.0) * 360;
    float look = (L / 255.0) * 360;
    MinecraftLocation inputLocation { rot, look, X, Y, Z};

    if (!client->LoggedIn || !client->player->tEntity)
        return;

    if (client->player->tEntity->MapID == client->player->MapId)
        client->player->tEntity->PositionSet(client->player->tEntity->MapID, inputLocation, 1, false);
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
    Client::Login(client->GetId(), client->player->LoginName, client->player->MPPass, client->player->ClientVersion);
}

void PacketHandlers::HandleTwoWayPing(const std::shared_ptr<NetworkClient> &client) {
    unsigned char direction = client->ReceiveBuffer->ReadByte();
    short timeval = client->ReceiveBuffer->ReadShort();

    if (direction == 0) {
        Packets::SendTwoWayPing(client, direction, timeval);
        return;
    }

    auto totalDuration = static_cast<short>(clock() - timeval);
    float secondsTaken = (float)totalDuration / static_cast<float>(CLOCKS_PER_SEC);
    client->Ping = secondsTaken;
}

void PacketHandlers::HandlePlayerClicked(const std::shared_ptr<NetworkClient> &client) {
    unsigned char button = client->ReceiveBuffer->ReadByte();
    unsigned char action = client->ReceiveBuffer->ReadByte();
    short yaw = client->ReceiveBuffer->ReadShort();
    short pitch = client->ReceiveBuffer->ReadShort();
    char targetedEntity = static_cast<char>(client->ReceiveBuffer->ReadByte());
    short targetBlockX = client->ReceiveBuffer->ReadShort();
    short targetBlockY = client->ReceiveBuffer->ReadShort();
    short targetBlockZ = client->ReceiveBuffer->ReadShort();
    unsigned char targetedBlockFace = client->ReceiveBuffer->ReadByte();

    auto cb = static_cast<ClickButton>(button);
    auto ca = static_cast<ClickAction>(action);
    Vector3S targetBlock {targetBlockX, targetBlockY, targetBlockZ};
    auto bf = static_cast<ClickTargetBlockFace>(targetedBlockFace);

    client->player->PlayerClicked(cb, ca, yaw, pitch, targetedEntity, targetBlock, bf);
}

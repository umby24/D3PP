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
#include "common/Vectors.h"
#include "common/MinecraftLocation.h"

using namespace D3PP::Common;

void PacketHandlers::HandleHandshake(const std::shared_ptr<NetworkClient>& client) {
	const char clientVersion = static_cast<char>(client->ReceiveBuffer->ReadByte());
    std::string clientName = client->ReceiveBuffer->ReadString();
	const std::string mppass = client->ReceiveBuffer->ReadString();
	const char isCpe = static_cast<char>(client->ReceiveBuffer->ReadByte());
    Utils::TrimString(clientName);

    if (!client->LoggedIn && isCpe != 66) {
        Client::Login(client->GetId(), clientName, mppass, clientVersion);
    } else if (isCpe == 66 && !client->LoggedIn) { // -- CPE capable Client
        Logger::LogAdd("Network", "CPE Client Detected", LogType::DEBUG, GLF);
        Client::LoginCpe(client->GetId(), clientName, mppass, clientVersion);
    }
}

void PacketHandlers::HandlePing(const std::shared_ptr<NetworkClient> &client) {
    client->Ping = static_cast<float>((std::chrono::steady_clock::now() - client->PingSentTime).count());
}

void PacketHandlers::HandleBlockChange(const std::shared_ptr<NetworkClient> &client) {
	const short x = client->ReceiveBuffer->ReadShort();
	const short z = client->ReceiveBuffer->ReadShort();
	const short y = client->ReceiveBuffer->ReadShort();
	const char mode = static_cast<char>(client->ReceiveBuffer->ReadByte() & 255);
	const char type = static_cast<char>(client->ReceiveBuffer->ReadByte() & 255);

    if (!client->LoggedIn || !client->GetPlayerInstance()->GetEntity())
        return;

    BuildModeMain* bmm = BuildModeMain::GetInstance();
    bmm->Distribute(client->GetId(), client->GetPlayerInstance()->GetEntity()->MapID, x, y, z, (mode > 0), type);
}

void PacketHandlers::HandlePlayerTeleport(const std::shared_ptr<NetworkClient> &client) {
    // -- CPE :)
    if (CPE::GetClientExtVersion(client, HELDBLOCK_EXT_NAME) == 1) {
        if (client->GetPlayerInstance()->GetEntity() != nullptr)
            client->GetPlayerInstance()->GetEntity()->heldBlock = client->ReceiveBuffer->ReadByte();
        else
            client->ReceiveBuffer->ReadByte();
    } else {
        client->ReceiveBuffer->ReadByte();
    }

    const auto X = static_cast<unsigned short>(client->ReceiveBuffer->ReadShort());
    const auto Z = static_cast<unsigned short>(client->ReceiveBuffer->ReadShort());
    const auto Y = static_cast<unsigned short>(client->ReceiveBuffer->ReadShort());
    const char R = static_cast<char>(client->ReceiveBuffer->ReadByte());
    const char L = static_cast<char>(client->ReceiveBuffer->ReadByte());
    const auto rot = static_cast<float>((R / 255.0) * 360);
    const auto look = static_cast<float>((L / 255.0) * 360);

    const MinecraftLocation inputLocation { rot, look, Vector3S(X, Y, Z)};

    if (!client->LoggedIn || !client->GetPlayerInstance()->GetEntity())
        return;

    if (client->GetPlayerInstance()->GetEntity()->MapID == client->GetPlayerInstance()->GetEntity()->MapID)
        client->GetPlayerInstance()->GetEntity()->PositionSet(client->GetPlayerInstance()->GetEntity()->MapID, inputLocation, 1, false);
}

void PacketHandlers::HandleChatPacket(const std::shared_ptr<NetworkClient> &client) {
	const char playerId = client->ReceiveBuffer->ReadByte();
    std::string message = client->ReceiveBuffer->ReadString();
    Utils::TrimString(message);
    if (client->LoggedIn && client->GetPlayerInstance()->GetEntity()) {
        Chat::HandleIncomingChat(client, message, playerId);
    }
}

void PacketHandlers::HandleExtInfo(const std::shared_ptr<NetworkClient> &client) {
    std::string appName = client->ReceiveBuffer->ReadString();
    const short extensions = client->ReceiveBuffer->ReadShort();
    client->CPE = true;
    
    if (extensions == 0) {
        CPE::PreLoginExtensions(client);
    }
    
    client->CustomExtensions = extensions;
    Logger::LogAdd("CPE", "Client " + appName + " supports " + stringulate(extensions) + " extensions", LogType::DEBUG, GLF);
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
	const unsigned char supportLevel = client->ReceiveBuffer->ReadByte();
    client->CustomBlocksLevel = supportLevel;

    Logger::LogAdd("CPE", "CPE Process complete.", LogType::DEBUG, GLF);
    auto concrete = std::static_pointer_cast<D3PP::world::Player>(client->GetPlayerInstance());
    Client::Login(client->GetId(), client->GetLoginName(), concrete->MPPass, concrete->ClientVersion);
}

void PacketHandlers::HandleTwoWayPing(const std::shared_ptr<NetworkClient> &client) {
	const unsigned char direction = client->ReceiveBuffer->ReadByte();
	const short timeval = client->ReceiveBuffer->ReadShort();

    if (direction == 0) {
        Packets::SendTwoWayPing(client, direction, timeval);
        return;
    }

	const auto totalDuration = static_cast<short>(clock() - timeval);
	const float secondsTaken = static_cast<float>(totalDuration) / static_cast<float>(CLOCKS_PER_SEC);
    client->Ping = secondsTaken;
}

void PacketHandlers::HandlePlayerClicked(const std::shared_ptr<NetworkClient> &client) {
    unsigned char button = client->ReceiveBuffer->ReadByte();
    unsigned char action = client->ReceiveBuffer->ReadByte();
    const short yaw = client->ReceiveBuffer->ReadShort();
    const short pitch = client->ReceiveBuffer->ReadShort();
    const char targetedEntity = static_cast<char>(client->ReceiveBuffer->ReadByte());
    const short targetBlockX = client->ReceiveBuffer->ReadShort();
    const short targetBlockY = client->ReceiveBuffer->ReadShort();
    const short targetBlockZ = client->ReceiveBuffer->ReadShort();
    unsigned char targetedBlockFace = client->ReceiveBuffer->ReadByte();

    const auto cb = static_cast<ClickButton>(button);
    const auto ca = static_cast<ClickAction>(action);
    const Vector3S targetBlock {targetBlockX, targetBlockY, targetBlockZ};
    const auto bf = static_cast<ClickTargetBlockFace>(targetedBlockFace);
    auto concrete = std::static_pointer_cast<D3PP::world::Player>(client->GetPlayerInstance());
    concrete->PlayerClicked(cb, ca, yaw, pitch, targetedEntity, targetBlock, bf);
}

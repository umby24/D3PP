//
// Created by Wande on 3/17/2021.
//

#include <Packets.h>
#include "Network.h"
#include "NetworkClient.h"
#include "Utils.h"
#include "common/ByteBuffer.h"

static std::shared_ptr<NetworkClient> GetPlayer(int id) {
    auto network = Network::GetInstance();
    auto result = network->GetClient(id);
    return result;
}
void Packets::SendClientHandshake(int clientId, char protocolVersion, std::string serverName, std::string serverMotd,
                                  char userType) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(c->sendLock);
        c->SendBuffer->Write((unsigned char)0);
        c->SendBuffer->Write((unsigned char)protocolVersion);
        if (serverName.size() != 64) Utils::padTo(serverName, 64);
        if (serverMotd.size() != 64) Utils::padTo(serverName, 64);
        c->SendBuffer->Write(std::move(serverName));
        c->SendBuffer->Write(std::move(serverMotd));
        c->SendBuffer->Write((unsigned char)userType);
        c->SendBuffer->Purge();
    }
}

void Packets::SendMapInit(int clientId) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(c->sendLock);
        c->SendBuffer->Write((unsigned char)2);
        c->SendBuffer->Purge();
    }
}

void Packets::SendMapData(int clientId, short chunkSize, char *data, unsigned char percentComplete) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(c->sendLock);
        c->SendBuffer->Write((unsigned char)3);
        c->SendBuffer->Write((short)chunkSize);
        std::vector<unsigned char> vData(data, data+1024);
        c->SendBuffer->Write(vData, 1024);
        c->SendBuffer->Write(percentComplete);
        c->SendBuffer->Purge();
    }
}

void Packets::SendMapFinalize(int clientId, short sizeX, short sizeY, short sizeZ) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(c->sendLock);
        c->SendBuffer->Write((unsigned char)4);
        c->SendBuffer->Write((short)sizeX);
        c->SendBuffer->Write((short)sizeZ);
        c->SendBuffer->Write((short)sizeY);
        c->SendBuffer->Purge();
    }
}

void Packets::SendBlockChange(int clientId, short x, short y, short z, unsigned char type) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(c->sendLock);
        c->SendBuffer->Write((unsigned char)6);
        c->SendBuffer->Write((short)x);
        c->SendBuffer->Write((short)z);
        c->SendBuffer->Write((short)y);
        c->SendBuffer->Write((unsigned char)type);
        c->SendBuffer->Purge();
    }
}

void Packets::SendSpawnEntity(int clientId, char playerId, std::string name, short x, short y, short z, char rotation,
                              char look) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(c->sendLock);
        c->SendBuffer->Write((unsigned char)7);
        c->SendBuffer->Write((unsigned char)playerId);
        if (name.size() != 64) Utils::padTo(name, 64);
        c->SendBuffer->Write(std::move(name));
        c->SendBuffer->Write((short)x);
        c->SendBuffer->Write((short)z);
        c->SendBuffer->Write((short)y);
        c->SendBuffer->Write((unsigned char)rotation);
        c->SendBuffer->Write((unsigned char)look);
        c->SendBuffer->Purge();
    }
}

void Packets::SendPlayerTeleport(int clientId, char playerId, short x, short y, short z, char rotation, char look) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(c->sendLock);
        c->SendBuffer->Write((unsigned char)8);
        c->SendBuffer->Write((unsigned char)playerId);
        c->SendBuffer->Write((short)x);
        c->SendBuffer->Write((short)z);
        c->SendBuffer->Write((short)y);
        c->SendBuffer->Write((unsigned char)rotation);
        c->SendBuffer->Write((unsigned char)look);
        c->SendBuffer->Purge();
    }
}

void Packets::SendDespawnEntity(int clientId, char playerId) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(c->sendLock);
        c->SendBuffer->Write((unsigned char)12);
        c->SendBuffer->Write((unsigned char)playerId);
        c->SendBuffer->Purge();
    }
}

void Packets::SendChatMessage(int clientId, std::string message, char location) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(c->sendLock);
        c->SendBuffer->Write((unsigned char)13);
        c->SendBuffer->Write((unsigned char)location);
        if (message.size() != 64) Utils::padTo(message, 64);
        c->SendBuffer->Write(std::move(message));
        c->SendBuffer->Purge();
    }
}

void Packets::SendDisconnect(int clientId, std::string reason) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(c->sendLock);
        c->SendBuffer->Write((unsigned char)14);
        if (reason.size() != 64) Utils::padTo(reason, 64);
        c->SendBuffer->Write(std::move(reason));
        c->SendBuffer->Purge();
    }
}

void Packets::SendExtInfo(std::shared_ptr<NetworkClient> client, std::string serverName, int extensionCount) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)16);
        if (serverName.size() != 64) Utils::padTo(serverName, 64);
        client->SendBuffer->Write(std::move(serverName));
        client->SendBuffer->Write((short)extensionCount);
        client->SendBuffer->Purge();
    }
}

void Packets::SendExtEntry(std::shared_ptr<NetworkClient> client, std::string extensionName, int versionNumber) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)17);
        if (extensionName.size() != 64) Utils::padTo(extensionName, 64);
        client->SendBuffer->Write(std::move(extensionName));
        client->SendBuffer->Write(versionNumber);
        client->SendBuffer->Purge();
    }
}

void Packets::SendCustomBlockSupportLevel(std::shared_ptr<NetworkClient> client, unsigned char supportLevel) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)19);
        client->SendBuffer->Write((unsigned char)supportLevel);
        client->SendBuffer->Purge();
    }
}

void Packets::SendClickDistance(std::shared_ptr<NetworkClient> client, short distance) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)18);
        client->SendBuffer->Write((short)distance);
        client->SendBuffer->Purge();
    }
}

void Packets::SendHoldThis(std::shared_ptr<NetworkClient> client, unsigned char block, bool preventChange) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)20);
        client->SendBuffer->Write((unsigned char)block);
        client->SendBuffer->Write((unsigned char)preventChange);
        client->SendBuffer->Purge();
    }
}

void Packets::SendTextHotkeys(std::shared_ptr<NetworkClient> client, std::string label, std::string action, int keyCode,
                              char modifier) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)21);
        if (label.size() != 64) Utils::padTo(label, 64);
        if (action.size() != 64) Utils::padTo(action, 64);
        client->SendBuffer->Write(label);
        client->SendBuffer->Write(action);
        client->SendBuffer->Write(keyCode);
        client->SendBuffer->Write((unsigned char)modifier);
        client->SendBuffer->Purge();
    }
}

void Packets::SendExtAddPlayerName(std::shared_ptr<NetworkClient> client, short nameId, std::string playerName,
                                   std::string listName, std::string groupName, char groupRank) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)22);
        client->SendBuffer->Write((short)nameId);
        if (playerName.size() != 64) Utils::padTo(playerName, 64);
        if (listName.size() != 64) Utils::padTo(listName, 64);
        if (groupName.size() != 64) Utils::padTo(groupName, 64);
        client->SendBuffer->Write(playerName);
        client->SendBuffer->Write(listName);
        client->SendBuffer->Write(groupName);
        client->SendBuffer->Write((unsigned char)groupRank);
        client->SendBuffer->Purge();
    };
}

void Packets::SendExtRemovePlayerName(std::shared_ptr<NetworkClient> client, short nameId) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)24);
        client->SendBuffer->Write((short)nameId);
        client->SendBuffer->Purge();
    }
}

void Packets::SendSetEnvironmentColors(std::shared_ptr<NetworkClient> client, char type, short red, short green,
                                       short blue) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)25);
        client->SendBuffer->Write((unsigned char)type);
        client->SendBuffer->Write((short)red);
        client->SendBuffer->Write((short)green);
        client->SendBuffer->Write((short)blue);
        client->SendBuffer->Purge();
    }
}

void Packets::SendSelectionBoxAdd(std::shared_ptr<NetworkClient> client, unsigned char selectionId, std::string label,
                                  short startX, short startY, short startZ, short endX, short endY, short endZ,
                                  short red, short green, short blue, short opacity) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)26);
        client->SendBuffer->Write((unsigned char)selectionId);
        if (label.size() != 64) Utils::padTo(label, 64);
        client->SendBuffer->Write(label);
        client->SendBuffer->Write((short)startX);
        client->SendBuffer->Write((short)startZ);
        client->SendBuffer->Write((short)startY);
        client->SendBuffer->Write((short)endX);
        client->SendBuffer->Write((short)endZ);
        client->SendBuffer->Write((short)endY);
        client->SendBuffer->Write((short)red);
        client->SendBuffer->Write((short)green);
        client->SendBuffer->Write((short)blue);
        client->SendBuffer->Write((short)opacity);
        client->SendBuffer->Purge();
    }
}

void Packets::SendSelectionBoxDelete(std::shared_ptr<NetworkClient> client, unsigned char selectionId) {
    const std::scoped_lock<std::mutex> sLock(client->sendLock);
    client->SendBuffer->Write((unsigned char)27);
    client->SendBuffer->Write((unsigned char)selectionId);
    client->SendBuffer->Purge();
}

void Packets::SendBlockPermissions(std::shared_ptr<NetworkClient> client, unsigned char blockId, bool canPlace,
                                   bool canDelete) {
    const std::scoped_lock<std::mutex> sLock(client->sendLock);
    client->SendBuffer->Write((unsigned char)28);
    client->SendBuffer->Write((unsigned char)blockId);
    client->SendBuffer->Write((unsigned char)canPlace);
    client->SendBuffer->Write((unsigned char)canDelete);
    client->SendBuffer->Purge();
}

void Packets::SendChangeModel(std::shared_ptr<NetworkClient> client, unsigned char entityId, std::string modelName) {
    const std::scoped_lock<std::mutex> sLock(client->sendLock);
    client->SendBuffer->Write((unsigned char)29);
    client->SendBuffer->Write((unsigned char)entityId);
    if (modelName.size() != 64) Utils::padTo(modelName, 64);
    client->SendBuffer->Write(modelName);
    client->SendBuffer->Purge();
}

void Packets::SendEnvMapAppearance(std::shared_ptr<NetworkClient> client, std::string url, unsigned char sideBlock,
                                   unsigned char edgeBlock, short sideLevel) {
    const std::scoped_lock<std::mutex> sLock(client->sendLock);
    client->SendBuffer->Write((unsigned char)30);
    if (url.size() != 64) Utils::padTo(url, 64);
    client->SendBuffer->Write(url);
    client->SendBuffer->Write((unsigned char)sideBlock);
    client->SendBuffer->Write((unsigned char)edgeBlock);
    client->SendBuffer->Write((short)sideLevel);
    client->SendBuffer->Purge();
}

void Packets::SendSetWeather(std::shared_ptr<NetworkClient> client, unsigned char weatherType) {
    const std::scoped_lock<std::mutex> sLock(client->sendLock);
    client->SendBuffer->Write((unsigned char)31);
    client->SendBuffer->Write((unsigned char)weatherType);
    client->SendBuffer->Purge();
}

void
Packets::SendHackControl(std::shared_ptr<NetworkClient> client, bool flying, bool noClip, bool speeding, bool respawn,
                         bool thirdPerson, short jumpHeight) {
    const std::scoped_lock<std::mutex> sLock(client->sendLock);
    client->SendBuffer->Write((unsigned char)32);
    client->SendBuffer->Write((unsigned char)flying);
    client->SendBuffer->Write((unsigned char)noClip);
    client->SendBuffer->Write((unsigned char)speeding);
    client->SendBuffer->Write((unsigned char)respawn);
    client->SendBuffer->Write((unsigned char)thirdPerson);
    client->SendBuffer->Write((short)jumpHeight);
    client->SendBuffer->Purge();
}

void Packets::SendExtAddEntity2(std::shared_ptr<NetworkClient> client, unsigned char entityId, std::string name,
                                std::string skin, short X, short Y, short Z, unsigned char rotation,
                                unsigned char look) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock<std::mutex> sLock(client->sendLock);
        client->SendBuffer->Write((unsigned char)33);
        client->SendBuffer->Write((unsigned char)entityId);
        if (name.size() != 64) Utils::padTo(name, 64);
        if (skin.size() != 64) Utils::padTo(skin, 64);
        client->SendBuffer->Write(name);
        client->SendBuffer->Write(skin);
        client->SendBuffer->Write((short)X);
        client->SendBuffer->Write((short)Z);
        client->SendBuffer->Write((short)Y);
        client->SendBuffer->Write((unsigned char)rotation);
        client->SendBuffer->Write((unsigned char)look);
        client->SendBuffer->Purge();
    }
}

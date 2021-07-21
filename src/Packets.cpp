//
// Created by Wande on 3/17/2021.
//

#include <Packets.h>
#include "Network.h"

static std::shared_ptr<NetworkClient> GetPlayer(int id) {
    auto network = Network::GetInstance();
    auto result = network->GetClient(id);
    return result;
}
void Packets::SendClientHandshake(int clientId, char protocolVersion, std::string serverName, std::string serverMotd,
                                  char userType) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(0);
    c->OutputWriteByte(protocolVersion);
    c->OutputWriteString(std::move(serverName));
    c->OutputWriteString(std::move(serverMotd));
    c->OutputWriteByte(userType);
}

void Packets::SendMapInit(int clientId) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(2);
}

void Packets::SendMapData(int clientId, short chunkSize, char *data, char percentComplete) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(3);
    c->OutputWriteShort(chunkSize);
    c->OutputWriteBlob(data, static_cast<int>(chunkSize));
    c->OutputWriteByte(percentComplete);
}

void Packets::SendMapFinalize(int clientId, short sizeX, short sizeY, short sizeZ) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(4);
    c->OutputWriteShort(sizeX);
    c->OutputWriteShort(sizeZ);
    c->OutputWriteShort(sizeY);
}

void Packets::SendBlockChange(int clientId, short x, short y, short z, unsigned char type) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(6);
    c->OutputWriteShort(x);
    c->OutputWriteShort(z);
    c->OutputWriteShort(y);
    c->OutputWriteByte(type);
}

void Packets::SendSpawnEntity(int clientId, char playerId, std::string name, short x, short y, short z, char rotation,
                              char look) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(7);
    c->OutputWriteByte(playerId);
    c->OutputWriteString(std::move(name));
    c->OutputWriteShort(x);
    c->OutputWriteShort(z);
    c->OutputWriteShort(y);
    c->OutputWriteByte(rotation);
    c->OutputWriteByte(look);
}

void Packets::SendPlayerTeleport(int clientId, char playerId, short x, short y, short z, char rotation, char look) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(8);
    c->OutputWriteByte(playerId);
    c->OutputWriteShort(x);
    c->OutputWriteShort(z);
    c->OutputWriteShort(y);
    c->OutputWriteByte(rotation);
    c->OutputWriteByte(look);
}

void Packets::SendDespawnEntity(int clientId, char playerId) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(12);
    c->OutputWriteByte(playerId);
}

void Packets::SendChatMessage(int clientId, std::string message, char location) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(13);
    c->OutputWriteByte(location);
    c->OutputWriteString(std::move(message));
}

void Packets::SendDisconnect(int clientId, std::string reason) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(14);
    c->OutputWriteString(std::move(reason));
}

void Packets::SendExtInfo(std::shared_ptr<NetworkClient> client, std::string serverName, int extensionCount) {
    client->OutputWriteByte(16);
    client->OutputWriteString(std::move(serverName));
    client->OutputWriteShort(extensionCount);
}

void Packets::SendExtEntry(std::shared_ptr<NetworkClient> client, std::string extensionName, int versionNumber) {
    client->OutputWriteByte(17);
    client->OutputWriteString(std::move(extensionName));
    client->OutputWriteInt(versionNumber);
}

void Packets::SendCustomBlockSupportLevel(std::shared_ptr<NetworkClient> client, unsigned char supportLevel) {
    client->OutputWriteByte(19);
    client->OutputWriteByte(supportLevel);
}

void Packets::SendClickDistance(std::shared_ptr<NetworkClient> client, short distance) {
    client->OutputWriteByte(18);
    client->OutputWriteShort(distance);
}

void Packets::SendHoldThis(std::shared_ptr<NetworkClient> client, unsigned char block, bool preventChange) {
    client->OutputWriteByte(20);
    client->OutputWriteByte(block);
    client->OutputWriteByte(preventChange);
}

void Packets::SendTextHotkeys(std::shared_ptr<NetworkClient> client, std::string label, std::string action, int keyCode,
                              char modifier) {
    client->OutputWriteByte(21);
    client->OutputWriteString(label);
    client->OutputWriteString(action);
    client->OutputWriteInt(keyCode);
    client->OutputWriteByte(modifier);
}

void Packets::SendExtAddPlayerName(std::shared_ptr<NetworkClient> client, short nameId, std::string playerName,
                                   std::string listName, std::string groupName, char groupRank) {
    client->OutputWriteByte(22);
    client->OutputWriteShort(nameId);
    client->OutputWriteString(playerName);
    client->OutputWriteString(listName);
    client->OutputWriteString(groupName);
    client->OutputWriteByte(groupRank);
}

void Packets::SendExtRemovePlayerName(std::shared_ptr<NetworkClient> client, short nameId) {
    client->OutputWriteByte(24);
    client->OutputWriteShort(nameId);
}

void Packets::SendSetEnvironmentColors(std::shared_ptr<NetworkClient> client, char type, short red, short green,
                                       short blue) {
    client->OutputWriteByte(25);
    client->OutputWriteByte(type);
    client->OutputWriteShort(red);
    client->OutputWriteShort(green);
    client->OutputWriteShort(blue);
}

void Packets::SendSelectionBoxAdd(std::shared_ptr<NetworkClient> client, unsigned char selectionId, std::string label,
                                  short startX, short startY, short startZ, short endX, short endY, short endZ,
                                  short red, short green, short blue, short opacity) {
    client->OutputWriteByte(26);
    client->OutputWriteByte(selectionId);
    client->OutputWriteString(label);
    client->OutputWriteShort(startX);
    client->OutputWriteShort(startZ);
    client->OutputWriteShort(startY);
    client->OutputWriteShort(endX);
    client->OutputWriteShort(endZ);
    client->OutputWriteShort(endY);
    client->OutputWriteShort(red);
    client->OutputWriteShort(green);
    client->OutputWriteShort(blue);
    client->OutputWriteShort(opacity);
}

void Packets::SendSelectionBoxDelete(std::shared_ptr<NetworkClient> client, unsigned char selectionId) {
    client->OutputWriteByte(27);
    client->OutputWriteByte(selectionId);
}

void Packets::SendBlockPermissions(std::shared_ptr<NetworkClient> client, unsigned char blockId, bool canPlace,
                                   bool canDelete) {
    client->OutputWriteByte(28);
    client->OutputWriteByte(blockId);
    client->OutputWriteByte(canPlace);
    client->OutputWriteByte(canDelete);
}

void Packets::SendChangeModel(std::shared_ptr<NetworkClient> client, unsigned char entityId, std::string modelName) {
    client->OutputWriteByte(29);
    client->OutputWriteByte(entityId);
    client->OutputWriteString(modelName);
}

void Packets::SendEnvMapAppearance(std::shared_ptr<NetworkClient> client, std::string url, unsigned char sideBlock,
                                   unsigned char edgeBlock, short sideLevel) {
    client->OutputWriteByte(30);
    client->OutputWriteString(url);
    client->OutputWriteByte(sideBlock);
    client->OutputWriteByte(edgeBlock);
    client->OutputWriteShort(sideLevel);
}

void Packets::SendSetWeather(std::shared_ptr<NetworkClient> client, unsigned char weatherType) {
    client->OutputWriteByte(31);
    client->OutputWriteByte(weatherType);
}

void
Packets::SendHackControl(std::shared_ptr<NetworkClient> client, bool flying, bool noClip, bool speeding, bool respawn,
                         bool thirdPerson, short jumpHeight) {
    client->OutputWriteByte(32);
    client->OutputWriteByte(flying);
    client->OutputWriteByte(noClip);
    client->OutputWriteByte(speeding);
    client->OutputWriteByte(respawn);
    client->OutputWriteByte(thirdPerson);
    client->OutputWriteShort(jumpHeight);
}

void Packets::SendExtAddEntity2(std::shared_ptr<NetworkClient> client, unsigned char entityId, std::string name,
                                std::string skin, short X, short Y, short Z, unsigned char rotation,
                                unsigned char look) {
    client->OutputWriteByte(33);
    client->OutputWriteByte(entityId);
    client->OutputWriteString(name);
    client->OutputWriteString(skin);
    client->OutputWriteShort(X);
    client->OutputWriteShort(Z);
    client->OutputWriteShort(Y);
    client->OutputWriteByte(rotation);
    client->OutputWriteByte(look);
}

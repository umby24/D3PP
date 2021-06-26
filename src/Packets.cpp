//
// Created by Wande on 3/17/2021.
//

#include <Packets.h>
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

void Packets::SendBlockChange(int clientId, short x, short y, short z, char type) {
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
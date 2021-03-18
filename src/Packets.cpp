//
// Created by Wande on 3/17/2021.
//

#include <Packets.h>
static shared_ptr<NetworkClient> GetPlayer(int id) {
    auto network = Network::GetInstance();
    auto result = network->GetClient(id);
    return result;
}
void Packets::SendClientHandshake(int clientId, char protocolVersion, std::string serverName, std::string serverMotd,
                                  char userType) {
    shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(0);
    c->OutputWriteByte(protocolVersion);
    c->OutputWriteString(std::move(serverName));
    c->OutputWriteString(std::move(serverMotd));
    c->OutputWriteByte(userType);
}

void Packets::SendMapInit(int clientId) {
    shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(2);
}

void Packets::SendMapData(int clientId, short chunkSize, char *data, char percentComplete) {
    shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(3);
    c->OutputWriteShort(chunkSize);
    c->OutputWriteBlob(data, static_cast<int>(chunkSize));
    c->OutputWriteByte(percentComplete);
}

void Packets::SendMapFinalize(int clientId, short sizeX, short sizeY, short sizeZ) {
    shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(4);
    c->OutputWriteShort(sizeX);
    c->OutputWriteShort(sizeZ);
    c->OutputWriteShort(sizeY);
}

void Packets::SendBlockChange(int clientId, short x, short y, short z, char type) {
    shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(6);
    c->OutputWriteShort(x);
    c->OutputWriteShort(z);
    c->OutputWriteShort(y);
    c->OutputWriteByte(type);
}

void Packets::SendSpawnEntity(int clientId, char playerId, std::string name, short x, short y, short z, char rotation,
                              char look) {
    shared_ptr<NetworkClient> c = GetPlayer(clientId);
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
    shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(8);
    c->OutputWriteByte(playerId);
    c->OutputWriteShort(x);
    c->OutputWriteShort(z);
    c->OutputWriteShort(y);
    c->OutputWriteByte(rotation);
    c->OutputWriteByte(look);
}

void Packets::SendDespawnEntity(int clientId, char playerId) {
    shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(12);
    c->OutputWriteByte(playerId);
}

void Packets::SendChatMessage(int clientId, std::string message, char location) {
    shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(13);
    c->OutputWriteByte(location);
    c->OutputWriteString(std::move(message));
}

void Packets::SendDisconnect(int clientId, std::string reason) {
    shared_ptr<NetworkClient> c = GetPlayer(clientId);
    c->OutputWriteByte(14);
    c->OutputWriteString(std::move(reason));
}


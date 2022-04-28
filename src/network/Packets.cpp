//
// Created by Wande on 3/17/2021.
//

#include <network/Packets.h>
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "Utils.h"
#include "common/ByteBuffer.h"
#include "CPE.h"

static std::shared_ptr<NetworkClient> GetPlayer(int id) {
    auto network = Network::GetInstance();
    auto result = std::static_pointer_cast<NetworkClient>(network->GetClient(id));
    return result;
}
void Packets::SendClientHandshake(int clientId, char protocolVersion, std::string serverName, std::string serverMotd,
                                  char userType) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock sLock(c->sendLock);
        c->SendBuffer->Write(static_cast<unsigned char>(0));
        c->SendBuffer->Write(static_cast<unsigned char>(protocolVersion));
        if (serverName.size() != 64) Utils::padTo(serverName, 64);
        if (serverMotd.size() != 64) Utils::padTo(serverName, 64);
        c->SendBuffer->Write(std::move(serverName));
        c->SendBuffer->Write(std::move(serverMotd));
        c->SendBuffer->Write(static_cast<unsigned char>(userType));
        c->SendBuffer->Purge();
    }
}

void Packets::SendMapInit(int clientId) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock sLock(c->sendLock);
        c->SendBuffer->Write(static_cast<unsigned char>(2));
        c->SendBuffer->Purge();
    }
}

void Packets::SendMapData(int clientId, short chunkSize, char *data, unsigned char percentComplete) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock sLock(c->sendLock);
        c->SendBuffer->Write(static_cast<unsigned char>(3));
        c->SendBuffer->Write(chunkSize);
        std::vector<unsigned char> vData(data, data+1024);
        c->SendBuffer->Write(vData, 1024);
        c->SendBuffer->Write(percentComplete);
        c->SendBuffer->Purge();
    }
}

void Packets::SendMapFinalize(int clientId, short sizeX, short sizeY, short sizeZ) {
	if (const std::shared_ptr<NetworkClient> c = GetPlayer(clientId); c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock sLock(c->sendLock);
        c->SendBuffer->Write(static_cast<unsigned char>(4));
        c->SendBuffer->Write(sizeX);
        c->SendBuffer->Write(sizeZ);
        c->SendBuffer->Write(sizeY);
        c->SendBuffer->Purge();
    }
}

void Packets::SendBlockChange(int clientId, short x, short y, short z, unsigned char type) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock sLock(c->sendLock);
        c->SendBuffer->Write(static_cast<unsigned char>(6));
        c->SendBuffer->Write(x);
        c->SendBuffer->Write(z);
        c->SendBuffer->Write(y);
        c->SendBuffer->Write(type);
        c->SendBuffer->Purge();
    }
}

void Packets::SendSpawnEntity(int clientId, char playerId, std::string name, short x, short y, short z, char rotation,
                              char look) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c != nullptr && c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock sLock(c->sendLock);
        c->SendBuffer->Write(static_cast<unsigned char>(7));
        c->SendBuffer->Write(static_cast<unsigned char>(playerId));
        if (name.size() != 64) Utils::padTo(name, 64);
        c->SendBuffer->Write(std::move(name));
        c->SendBuffer->Write(x);
        c->SendBuffer->Write(z);
        c->SendBuffer->Write(y);
        c->SendBuffer->Write(static_cast<unsigned char>((rotation*255)/360));
        c->SendBuffer->Write(static_cast<unsigned char>((look*255)/360));
        c->SendBuffer->Purge();
    }
}

void Packets::SendPlayerTeleport(int clientId, char playerId, short x, short y, short z, char rotation, char look) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c != nullptr && c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock sLock(c->sendLock);
        c->SendBuffer->Write(static_cast<unsigned char>(8));
        c->SendBuffer->Write(static_cast<unsigned char>(playerId));
        c->SendBuffer->Write(x);
        c->SendBuffer->Write(z);
        c->SendBuffer->Write(y);
        c->SendBuffer->Write(static_cast<unsigned char>(rotation));
        c->SendBuffer->Write(static_cast<unsigned char>(look));
        c->SendBuffer->Purge();
    }
}

void Packets::SendDespawnEntity(int clientId, char playerId) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock sLock(c->sendLock);
        c->SendBuffer->Write(static_cast<unsigned char>(12));
        c->SendBuffer->Write(static_cast<unsigned char>(playerId));
        c->SendBuffer->Purge();
    }
}

void Packets::SendChatMessage(int clientId, std::string message, char location) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);

	if (c == nullptr || c == NULL)
        return;

    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock sLock(c->sendLock);
        c->SendBuffer->Write(static_cast<unsigned char>(13));
        c->SendBuffer->Write(static_cast<unsigned char>(location));
        if (message.size() != 64) Utils::padTo(message, 64);
        c->SendBuffer->Write(std::move(message));
        c->SendBuffer->Purge();
    }
}

void Packets::SendDisconnect(int clientId, std::string reason) {
    std::shared_ptr<NetworkClient> c = GetPlayer(clientId);
    if (c->canSend && c->SendBuffer != nullptr) {
        const std::scoped_lock sLock(c->sendLock);
        c->SendBuffer->Write(static_cast<unsigned char>(14));
        if (reason.size() != 64) Utils::padTo(reason, 64);
        c->SendBuffer->Write(std::move(reason));
        c->SendBuffer->Purge();
    }
}

void Packets::SendExtInfo(std::shared_ptr<NetworkClient> client, std::string serverName, int extensionCount) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(16));
        if (serverName.size() != 64) Utils::padTo(serverName, 64);
        client->SendBuffer->Write(std::move(serverName));
        client->SendBuffer->Write(static_cast<short>(extensionCount));
        client->SendBuffer->Purge();
    }
}

void Packets::SendExtEntry(std::shared_ptr<NetworkClient> client, std::string extensionName, int versionNumber) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(17));
        if (extensionName.size() != 64) Utils::padTo(extensionName, 64);
        client->SendBuffer->Write(std::move(extensionName));
        client->SendBuffer->Write(versionNumber);
        client->SendBuffer->Purge();
    }
}

void Packets::SendCustomBlockSupportLevel(std::shared_ptr<NetworkClient> client, unsigned char supportLevel) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(19));
        client->SendBuffer->Write(supportLevel);
        client->SendBuffer->Purge();
    }
}

void Packets::SendClickDistance(std::shared_ptr<NetworkClient> client, short distance) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(18));
        client->SendBuffer->Write(distance);
        client->SendBuffer->Purge();
    }
}

void Packets::SendHoldThis(std::shared_ptr<NetworkClient> client, unsigned char block, bool preventChange) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(20));
        client->SendBuffer->Write(block);
        client->SendBuffer->Write(static_cast<unsigned char>(preventChange));
        client->SendBuffer->Purge();
    }
}

void Packets::SendTextHotkeys(std::shared_ptr<NetworkClient> client, std::string label, std::string action, int keyCode,
                              char modifier) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(21));
        if (label.size() != 64) Utils::padTo(label, 64);
        if (action.size() != 64) Utils::padTo(action, 64);
        client->SendBuffer->Write(label);
        client->SendBuffer->Write(action);
        client->SendBuffer->Write(keyCode);
        client->SendBuffer->Write(static_cast<unsigned char>(modifier));
        client->SendBuffer->Purge();
    }
}

void Packets::SendExtAddPlayerName(std::shared_ptr<NetworkClient> client, short nameId, std::string playerName,
                                   std::string listName, std::string groupName, char groupRank) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(22));
        client->SendBuffer->Write(nameId);
        if (playerName.size() != 64) Utils::padTo(playerName, 64);
        if (listName.size() != 64) Utils::padTo(listName, 64);
        if (groupName.size() != 64) Utils::padTo(groupName, 64);
        client->SendBuffer->Write(playerName);
        client->SendBuffer->Write(listName);
        client->SendBuffer->Write(groupName);
        client->SendBuffer->Write(static_cast<unsigned char>(groupRank));
        client->SendBuffer->Purge();
    };
}

void Packets::SendExtRemovePlayerName(std::shared_ptr<NetworkClient> client, short nameId) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(24));
        client->SendBuffer->Write(nameId);
        client->SendBuffer->Purge();
    }
}

void Packets::SendSetEnvironmentColors(std::shared_ptr<NetworkClient> client, char type, short red, short green,
                                       short blue) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(25));
        client->SendBuffer->Write(static_cast<unsigned char>(type));
        client->SendBuffer->Write(red);
        client->SendBuffer->Write(green);
        client->SendBuffer->Write(blue);
        client->SendBuffer->Purge();
    }
}

void Packets::SendSelectionBoxAdd(std::shared_ptr<NetworkClient> client, unsigned char selectionId, std::string label,
                                  short startX, short startY, short startZ, short endX, short endY, short endZ,
                                  short red, short green, short blue, short opacity) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(26));
        client->SendBuffer->Write(selectionId);
        if (label.size() != 64) Utils::padTo(label, 64);
        client->SendBuffer->Write(label);
        client->SendBuffer->Write(startX);
        client->SendBuffer->Write(startZ);
        client->SendBuffer->Write(startY);
        client->SendBuffer->Write(endX);
        client->SendBuffer->Write(endZ);
        client->SendBuffer->Write(endY);
        client->SendBuffer->Write(red);
        client->SendBuffer->Write(green);
        client->SendBuffer->Write(blue);
        client->SendBuffer->Write(opacity);
        client->SendBuffer->Purge();
    }
}

void Packets::SendSelectionBoxDelete(std::shared_ptr<NetworkClient> client, unsigned char selectionId) {
    const std::scoped_lock sLock(client->sendLock);
    client->SendBuffer->Write(static_cast<unsigned char>(27));
    client->SendBuffer->Write(selectionId);
    client->SendBuffer->Purge();
}

void Packets::SendBlockPermissions(std::shared_ptr<NetworkClient> client, unsigned char blockId, bool canPlace,
                                   bool canDelete) {
    const std::scoped_lock sLock(client->sendLock);
    client->SendBuffer->Write(static_cast<unsigned char>(28));
    client->SendBuffer->Write(blockId);
    client->SendBuffer->Write(static_cast<unsigned char>(canPlace));
    client->SendBuffer->Write(static_cast<unsigned char>(canDelete));
    client->SendBuffer->Purge();
}

void Packets::SendChangeModel(std::shared_ptr<NetworkClient> client, unsigned char entityId, std::string modelName) {
    const std::scoped_lock sLock(client->sendLock);
    client->SendBuffer->Write(static_cast<unsigned char>(29));
    client->SendBuffer->Write(entityId);
    if (modelName.size() != 64) Utils::padTo(modelName, 64);
    client->SendBuffer->Write(modelName);
    client->SendBuffer->Purge();
}

void Packets::SendChangeModel(std::shared_ptr<IMinecraftClient> client, unsigned char entityId, std::string modelName) {
    auto concrete = std::static_pointer_cast<NetworkClient>(client);
    const std::scoped_lock sLock(concrete->sendLock);
    concrete->SendBuffer->Write(static_cast<unsigned char>(29));
    concrete->SendBuffer->Write(entityId);
    if (modelName.size() != 64) Utils::padTo(modelName, 64);
    concrete->SendBuffer->Write(modelName);
    concrete->SendBuffer->Purge();
}

void Packets::SendEnvMapAppearance(std::shared_ptr<NetworkClient> client, std::string url, unsigned char sideBlock,
                                   unsigned char edgeBlock, short sideLevel) {
    const std::scoped_lock sLock(client->sendLock);
    client->SendBuffer->Write(static_cast<unsigned char>(30));
    if (url.size() != 64) Utils::padTo(url, 64);
    client->SendBuffer->Write(url);
    client->SendBuffer->Write(sideBlock);
    client->SendBuffer->Write(edgeBlock);
    client->SendBuffer->Write(sideLevel);
    client->SendBuffer->Purge();
}

void Packets::SendSetWeather(std::shared_ptr<NetworkClient> client, unsigned char weatherType) {
    const std::scoped_lock sLock(client->sendLock);
    client->SendBuffer->Write(static_cast<unsigned char>(31));
    client->SendBuffer->Write(weatherType);
    client->SendBuffer->Purge();
}

void
Packets::SendHackControl(std::shared_ptr<NetworkClient> client, bool flying, bool noClip, bool speeding, bool respawn,
                         bool thirdPerson, short jumpHeight) {
    const std::scoped_lock sLock(client->sendLock);
    client->SendBuffer->Write(static_cast<unsigned char>(32));
    client->SendBuffer->Write(static_cast<unsigned char>(flying));
    client->SendBuffer->Write(static_cast<unsigned char>(noClip));
    client->SendBuffer->Write(static_cast<unsigned char>(speeding));
    client->SendBuffer->Write(static_cast<unsigned char>(respawn));
    client->SendBuffer->Write(static_cast<unsigned char>(thirdPerson));
    client->SendBuffer->Write(jumpHeight);
    client->SendBuffer->Purge();
}

void Packets::SendExtAddEntity2(std::shared_ptr<NetworkClient> client, unsigned char entityId, std::string name,
                                std::string skin, short X, short Y, short Z, unsigned char rotation,
                                unsigned char look) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(33));
        client->SendBuffer->Write(entityId);
        if (name.size() != 64) Utils::padTo(name, 64);
        if (skin.size() != 64) Utils::padTo(skin, 64);
        client->SendBuffer->Write(name);
        client->SendBuffer->Write(skin);
        client->SendBuffer->Write(X);
        client->SendBuffer->Write(Z);
        client->SendBuffer->Write(Y);
        client->SendBuffer->Write(rotation);
        client->SendBuffer->Write(look);
        client->SendBuffer->Purge();
    }
}

void Packets::SendTwoWayPing(const std::shared_ptr<NetworkClient>& client, unsigned char direction, short timeval) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(43));
        client->SendBuffer->Write(direction);
        client->SendBuffer->Write(timeval);
        client->SendBuffer->Purge();
    }
}

void Packets::SendDefineBlock(const std::shared_ptr<NetworkClient> &client, BlockDefinition def) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(35));
        client->SendBuffer->Write(def.blockId);
        client->SendBuffer->Write(def.name);
        client->SendBuffer->Write(static_cast<unsigned char>(def.solidity));
        client->SendBuffer->Write(static_cast<unsigned char>(def.movementSpeed));

        if (CPE::GetClientExtVersion(client, EXTENDED_TEXTURES_EXT_NAME) != 1) {
            client->SendBuffer->Write(static_cast<unsigned char>(def.topTexture & 0xFF));
            client->SendBuffer->Write(static_cast<unsigned char>(def.leftTexture & 0xFF));
            client->SendBuffer->Write(static_cast<unsigned char>(def.bottomTexture & 0xFF));
        }
        else {
            client->SendBuffer->Write(def.topTexture);
            client->SendBuffer->Write(def.leftTexture);
            client->SendBuffer->Write(def.bottomTexture);
        }

        client->SendBuffer->Write(static_cast<unsigned char>(def.transmitsLight));
        client->SendBuffer->Write(static_cast<unsigned char>(def.walkSound));
        client->SendBuffer->Write(static_cast<unsigned char>(def.fullBright));
        client->SendBuffer->Write(static_cast<unsigned char>(def.shape));
        client->SendBuffer->Write(static_cast<unsigned char>(def.drawType));
        client->SendBuffer->Write(static_cast<unsigned char>(def.fogDensity));
        client->SendBuffer->Write(static_cast<unsigned char>(def.fogR));
        client->SendBuffer->Write(static_cast<unsigned char>(def.fogG));
        client->SendBuffer->Write(static_cast<unsigned char>(def.fogB));
        client->SendBuffer->Purge();
    }
}

void Packets::SendRemoveBlock(const std::shared_ptr<NetworkClient> &client, unsigned char blockId) {
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(36));
        client->SendBuffer->Write(blockId);
        client->SendBuffer->Purge();
    }
}

void Packets::SendDefineBlockExt(const std::shared_ptr<NetworkClient>& client, BlockDefinition def)
{
    if (client->canSend && client->SendBuffer != nullptr) {
        const std::scoped_lock sLock(client->sendLock);
        client->SendBuffer->Write(static_cast<unsigned char>(37));
        client->SendBuffer->Write(def.blockId);
        client->SendBuffer->Write(def.name);
        client->SendBuffer->Write(static_cast<unsigned char>(def.solidity));
        client->SendBuffer->Write(static_cast<unsigned char>(def.movementSpeed));

        if (CPE::GetClientExtVersion(client, EXTENDED_TEXTURES_EXT_NAME) != 1) {
            client->SendBuffer->Write(static_cast<unsigned char>(def.topTexture & 0xFF));
            client->SendBuffer->Write(static_cast<unsigned char>(def.leftTexture & 0xFF));
            client->SendBuffer->Write(static_cast<unsigned char>(def.rightTexture & 0xFF));
            client->SendBuffer->Write(static_cast<unsigned char>(def.frontTexture & 0xFF));
            client->SendBuffer->Write(static_cast<unsigned char>(def.backTexture & 0xFF));
            client->SendBuffer->Write(static_cast<unsigned char>(def.bottomTexture & 0xFF));
        }
        else {
            client->SendBuffer->Write(def.topTexture);
            client->SendBuffer->Write(def.leftTexture);
            client->SendBuffer->Write(def.rightTexture);
            client->SendBuffer->Write(def.frontTexture);
            client->SendBuffer->Write(def.backTexture);
            client->SendBuffer->Write(def.bottomTexture);
        }

        client->SendBuffer->Write(static_cast<unsigned char>(def.transmitsLight));
        client->SendBuffer->Write(static_cast<unsigned char>(def.walkSound));
        client->SendBuffer->Write(static_cast<unsigned char>(def.fullBright));
        client->SendBuffer->Write(static_cast<unsigned char>(def.minX));
        client->SendBuffer->Write(static_cast<unsigned char>(def.minZ));
        client->SendBuffer->Write(static_cast<unsigned char>(def.minY));
        client->SendBuffer->Write(static_cast<unsigned char>(def.maxX));
        client->SendBuffer->Write(static_cast<unsigned char>(def.maxZ));
        client->SendBuffer->Write(static_cast<unsigned char>(def.maxY));
        client->SendBuffer->Write(static_cast<unsigned char>(def.drawType));
        client->SendBuffer->Write(static_cast<unsigned char>(def.fogDensity));
        client->SendBuffer->Write(static_cast<unsigned char>(def.fogR));
        client->SendBuffer->Write(static_cast<unsigned char>(def.fogG));
        client->SendBuffer->Write(static_cast<unsigned char>(def.fogB));
        client->SendBuffer->Purge();
    }
}

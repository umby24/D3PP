//
// Created by Wande on 3/17/2021.
//

#ifndef D3PP_PACKETS_H
#define D3PP_PACKETS_H

#include <string>
#include <memory>
#include "CustomBlocks.h"

class IMinecraftClient;
class NetworkClient;

class Packets {
public:
    static void SendClientHandshake(int clientId, char protocolVersion, std::string serverName, std::string serverMotd, char userType);
    static void SendMapInit(int clientId);
    static void SendMapData(int clientId, short chunkSize, char* data, unsigned char percentComplete);
    static void SendMapFinalize(int clientId, short sizeX, short sizeY, short sizeZ);
    static void SendBlockChange(int clientId, short x, short y, short z, unsigned char type);
    static void SendSpawnEntity(int clientId, char playerId, std::string name, short x, short y, short z, char rotation, char look);
    static void SendPlayerTeleport(int clientId, char playerId, short x, short y, short z, char rotation, char look);
    static void SendDespawnEntity(int clientId, char playerId);
    static void SendChatMessage(int clientId, std::string message, char location);
    static void SendDisconnect(int clientId, std::string reason);
    // -- CPE:
    static void SendExtInfo(std::shared_ptr<NetworkClient> client, std::string serverName, int extensionCount);
    static void SendExtEntry(std::shared_ptr<NetworkClient> client, std::string extensionName, int versionNumber);
    static void SendClickDistance(std::shared_ptr<NetworkClient> client, short distance);
    static void SendCustomBlockSupportLevel(std::shared_ptr<NetworkClient> client, unsigned char supportLevel);
    static void SendHoldThis(std::shared_ptr<NetworkClient> client, unsigned char block, bool preventChange);
    static void SendTextHotkeys(std::shared_ptr<NetworkClient> client, std::string label, std::string action, int keyCode, char modifier);
    static void SendExtAddPlayerName(std::shared_ptr<NetworkClient> client, short nameId, std::string playerName, std::string listName, std::string groupName, char groupRank);
    static void SendExtRemovePlayerName(std::shared_ptr<NetworkClient> client, short nameId);
    static void SendSetEnvironmentColors(std::shared_ptr<NetworkClient> client, char type, short red, short green, short blue);
    static void SendSelectionBoxAdd(std::shared_ptr<NetworkClient> client, unsigned char selectionId, std::string label, short startX, short startY, short startZ, short endX, short endY, short endZ, short red, short green, short blue, short opacity);
    static void SendSelectionBoxDelete(std::shared_ptr<NetworkClient> client, unsigned char selectionId);
    static void SendBlockPermissions(std::shared_ptr<NetworkClient> client, unsigned char blockId, bool canPlace, bool canDelete);
    static void SendChangeModel(std::shared_ptr<NetworkClient> client, unsigned char entityId, std::string modelName);
    static void SendChangeModel(std::shared_ptr<IMinecraftClient> client, unsigned char entityId, std::string modelName);
    static void SendEnvMapAppearance(std::shared_ptr<NetworkClient> client, std::string url, unsigned char sideBlock, unsigned char edgeBlock, short sideLevel);
    static void SendSetWeather(std::shared_ptr<NetworkClient> client, unsigned char weatherType);
    static void SendHackControl(std::shared_ptr<NetworkClient> client, bool flying, bool noClip, bool speeding, bool respawn, bool thirdPerson, short jumpHeight);
    static void SendExtAddEntity2(std::shared_ptr<NetworkClient> client, unsigned char entityId, std::string name, std::string skin, short X, short Y, short Z, unsigned char rotation, unsigned char look);
    static void SendTwoWayPing(const std::shared_ptr<NetworkClient>& client, unsigned char direction, short timeVal);
    static void SendDefineBlock(const std::shared_ptr<NetworkClient>& client, BlockDefinition def);
    static void SendRemoveBlock(const std::shared_ptr<NetworkClient>& client, unsigned char blockId);
    static void SendDefineBlockExt(const std::shared_ptr<NetworkClient>& client, BlockDefinition def);
};
#endif //D3PP_PACKETS_H

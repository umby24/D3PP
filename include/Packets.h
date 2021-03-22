//
// Created by Wande on 3/17/2021.
//

#ifndef D3PP_PACKETS_H
#define D3PP_PACKETS_H

#include <string>
#include "Network.h"

class Packets {
public:
    static void SendClientHandshake(int clientId, char protocolVersion, std::string serverName, std::string serverMotd, char userType);
    static void SendMapInit(int clientId);
    static void SendMapData(int clientId, short chunkSize, char* data, char percentComplete);
    static void SendMapFinalize(int clientId, short sizeX, short sizeY, short sizeZ);
    static void SendBlockChange(int clientId, short x, short y, short z, char type);
    static void SendSpawnEntity(int clientId, char playerId, std::string name, short x, short y, short z, char rotation, char look);
    static void SendPlayerTeleport(int clientId, char playerId, short x, short y, short z, char rotation, char look);
    static void SendDespawnEntity(int clientId, char playerId);
    static void SendChatMessage(int clientId, std::string message, char location);
    static void SendDisconnect(int clientId, std::string reason);
    // -- CPE:
    static void SendExtInfo();
    static void SendExtEntry();
    static void SendClickDistance();
    static void SendCustomBlockSupportLevel();
    static void SendHoldThis();
    static void SendTextHotkeys();
    static void SendExtAddPlayerName();
    static void SendExtRemovePlayerName();
    static void SendSetEnvironmentColors();
    static void SendSelectionBoxAdd();
    static void SendSelectionBoxDelete();
    static void SendBlockPermissions();
    static void SendChangeModel();
    static void SendEnvMapAppearance();
    static void SendSetWeather();
    static void SendHackControl();
    static void SendExtAddEntity2();
};
#endif //D3PP_PACKETS_H

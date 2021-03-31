//
// Created by Wande on 3/30/2021.
//

#include "PacketHandlers.h"

void PacketHandlers::HandleHandshake(const std::shared_ptr<NetworkClient>& client) {
    client->InputAddOffset(1);
    char clientVersion = client->InputReadByte();
    std::string clientName = client->InputReadString();
    std::string mppass = client->InputReadString();
    char isCpe = client->InputReadByte();

    if (!client->LoggedIn && client->DisconnectTime == 0 && isCpe != 66) {
        Logger::LogAdd("PacketHandler", "Womg, incoming: " + clientName, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    } else if (isCpe == 66 && !client->LoggedIn && client->DisconnectTime == 0) {
        Logger::LogAdd("PacketHandler", "Womg, CPE incoming: " + clientName, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
}

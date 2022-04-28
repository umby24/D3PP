//
// Created by Wande on 3/30/2021.
//

#ifndef D3PP_PACKETHANDLERS_H
#define D3PP_PACKETHANDLERS_H
#include <string>
#include <memory>

class NetworkClient;

class PacketHandlers {
public:
    static void HandleHandshake(const std::shared_ptr<NetworkClient>& client);
    static void HandlePing(const std::shared_ptr<NetworkClient>& client);
    static void HandleBlockChange(const std::shared_ptr<NetworkClient>& client);
    static void HandlePlayerTeleport(const std::shared_ptr<NetworkClient>& client);
    static void HandleChatPacket(const std::shared_ptr<NetworkClient>& client);
    static void HandleExtInfo(const std::shared_ptr<NetworkClient>& client);
    static void HandleExtEntry(const std::shared_ptr<NetworkClient>& client);
    static void HandleCustomBlockSupportLevel(const std::shared_ptr<NetworkClient>& client);
    static void HandlePlayerClicked(const std::shared_ptr<NetworkClient>& client);
    static void HandleTwoWayPing(const std::shared_ptr<NetworkClient>& client);
};


#endif //D3PP_PACKETHANDLERS_H

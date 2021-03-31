//
// Created by Wande on 3/30/2021.
//

#ifndef D3PP_PACKETHANDLERS_H
#define D3PP_PACKETHANDLERS_H
#include <string>
#include <memory>

#include "Network.h"
#include "Logger.h"


class NetworkClient;

class PacketHandlers {
public:
    static void HandleHandshake(const std::shared_ptr<NetworkClient>& client);
};


#endif //D3PP_PACKETHANDLERS_H

//
// Created by unknown on 2/18/2021.
//

#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/PacketHandlers.h"
#include "world/Entity.h"
#include "world/Player.h"
#include "Client.h"
#include "EventSystem.h"
#include "common/ByteBuffer.h"
#include "network/Packets.h"
#include "network/Server.h"
#include "ConsoleClient.h"

const std::string MODULE_NAME = "Network";
Network* Network::singleton_ = nullptr;

std::shared_ptr<IMinecraftClient> Network::GetClient(int id) {
    if (id == -200) {
        return ConsoleClient::GetInstance();
    }

    std::shared_ptr<IMinecraftClient> result = nullptr;

    for(auto const &nc : D3PP::network::Server::roClients) {
        if (nc->GetId() == id) {
            result = std::static_pointer_cast<IMinecraftClient>(nc);
            break;
        }
    }

    return result;
}

Network *Network::GetInstance() {
    if (singleton_ == nullptr)
        singleton_ = new Network();

    return singleton_;
}

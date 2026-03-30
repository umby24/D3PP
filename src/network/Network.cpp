//
// Created by unknown on 2/18/2021.
//

#include "network/Network.h"
#include "network/NetworkClient.h"
#include "world/Player.h"
#include "network/Server.h"
#include "ConsoleClient.h"


std::shared_ptr<IMinecraftClient> Network::GetClient(int id) {
    if (id == -200)
        return ConsoleClient::GetInstance();

    std::shared_ptr<IMinecraftClient> result = nullptr;
    std::shared_lock lock(D3PP::network::Server::roMutex);
    for(auto const &nc : D3PP::network::Server::roClients) {
        if (nc->GetId() == id) {
            result = std::static_pointer_cast<IMinecraftClient>(nc);
            break;
        }
    }

    return result;
}
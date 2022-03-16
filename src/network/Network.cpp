//
// Created by unknown on 2/18/2021.
//

#include "network/Network.h"
#include "network/NetworkClient.h"

#ifndef __linux__
#include "network/WindowsServerSockets.h"
#include "network/WindowsSockets.h"
#else
#include "network/LinuxServerSockets.h"
#include "network/LinuxSockets.h"
#endif

#include "network/PacketHandlers.h"
#include "world/Entity.h"
#include "world/Player.h"
#include "common/Files.h"
#include "watchdog.h"
#include "Client.h"
#include "common/Logger.h"
#include "Utils.h"
#include "EventSystem.h"
#include "common/ByteBuffer.h"
#include "events/EventClientAdd.h"
#include "events/EventClientDelete.h"
#include "CPE.h"
#include "network/Packets.h"
#include "network/Server.h"
#include "common/Configuration.h"
#include "ConsoleClient.h"

const std::string MODULE_NAME = "Network";
Network* Network::singleton_ = nullptr;

Network::Network()  {
    TimerRate = 0;
    SaveFile = false;

    TaskItem networkOutputProcessor;
    networkOutputProcessor.Interval = std::chrono::milliseconds(0);
    networkOutputProcessor.Main = [this] { NetworkOutput(); };
    TaskScheduler::RegisterTask("Network_Output_Do", networkOutputProcessor);
}

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

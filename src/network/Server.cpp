//
// Created by Wande on 1/28/2022.
//

#include <network/Server.h>

#include <chrono>
#include <thread>
#include <vector>
#include <map>

#ifndef __linux__
#include "network/WindowsServerSockets.h"
#include "network/WindowsSockets.h"
#else
#include "network/LinuxServerSockets.h"
#include "network/LinuxSockets.h"
#endif

#include "common/Logger.h"
#include "common/Configuration.h"
#include "System.h"
#include "network/NetworkClient.h"
#include "network/IPacket.h"
#include "Utils.h"
#include "CPE.h"

int D3PP::network::Server::SentIncrement = 0;
float D3PP::network::Server::BytesSent = 0;
float D3PP::network::Server::BytesReceived = 0;
int D3PP::network::Server::ReceivedIncrement = 0;
std::vector<std::shared_ptr<IMinecraftClient>> D3PP::network::Server::roClients;
std::map<int,std::shared_ptr<IMinecraftClient>> D3PP::network::Server::m_clients;
std::mutex D3PP::network::Server::m_ClientMutex;

D3PP::network::Server::Server() {
    Interval = std::chrono::seconds(5);
    Main = [this](){ this->MainFunc(); };
    TaskScheduler::RegisterTask("Bandwidth", *this);
    m_serverSocket = std::make_unique<ServerSocket>(25566);
    m_serverSocket->Listen();
}

void D3PP::network::Server::MainFunc() {
    BytesReceived = ReceivedIncrement/1024.0f;
    BytesSent = SentIncrement/1024.0f;
    ReceivedIncrement = 0;
    SentIncrement = 0;
    Logger::LogAdd("Server", "Recv: " + stringulate(BytesReceived) + " KB/s, Sent: " + stringulate(BytesSent) + " KB/s.", DEBUG, GLF);
}

void D3PP::network::Server::Shutdown() {
    m_serverSocket->Stop();
    for (auto const& c : roClients) {
        c->Kick("Server shutting down", false);
    }
}

void D3PP::network::Server::HandleClientData() {
    while (System::IsRunning) {
        for(auto const& c : roClients) {
            if (c->IsDataAvailable())
                c->SendQueued();

            c->HandleData();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void D3PP::network::Server::RegisterClient(const std::shared_ptr<IMinecraftClient>& client) {
    std::scoped_lock<std::mutex> clientLock(m_ClientMutex);
    m_clients.insert(std::make_pair(client->GetId(), client));
    RebuildRoClients();
}

void D3PP::network::Server::UnregisterClient(const std::shared_ptr<IMinecraftClient>& client) {
    std::scoped_lock<std::mutex> clientLock(m_ClientMutex);
    m_clients.erase(client->GetId());
    RebuildRoClients();
}

void D3PP::network::Server::RebuildRoClients() {
    std::vector<std::shared_ptr<IMinecraftClient>> newRo;
    newRo.reserve(m_clients.size());
    for(auto const &nc : m_clients) {
        newRo.push_back(nc.second);
    }
    std::swap(newRo, roClients); // -- Should happen in an instant, but I suppose edge cases could happen \_(o_o)_/
}

void D3PP::network::Server::SendToAll(const IPacket& packet, std::string extension, int extVersion) {
    for ( auto const & c : roClients ) {
        if (!extension.empty()) {
            int currentVer = CPE::GetClientExtVersion(c, extension);

            if (extVersion == currentVer)
                c->SendPacket(packet);
        }

        c->SendPacket(packet);
    }
}

void D3PP::network::Server::SendAllExcept(const IPacket& packet, std::shared_ptr<IMinecraftClient> toNot) {
    for ( auto const & c : roClients ) {
        if (c == toNot)
            continue;

        c->SendPacket(packet);
    }
}


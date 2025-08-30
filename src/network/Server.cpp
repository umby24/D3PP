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
#include "events/EventClientAdd.h"
#include "events/EventClientDelete.h"

std::atomic<int> D3PP::network::Server::SentIncrement = 0;
float D3PP::network::Server::BytesSent = 0;
float D3PP::network::Server::BytesReceived = 0;
std::atomic<int> D3PP::network::Server::ReceivedIncrement = 0;

std::vector<std::shared_ptr<IMinecraftClient>> D3PP::network::Server::roClients;
std::map<int,std::shared_ptr<IMinecraftClient>> D3PP::network::Server::m_clients;
std::mutex D3PP::network::Server::m_ClientMutex;
std::shared_mutex D3PP::network::Server::roMutex;
D3PP::network::Server* D3PP::network::Server::m_Instance = nullptr;

D3PP::network::Server::Server() {
    m_port = Configuration::NetSettings.ListenPort;

    Interval = std::chrono::milliseconds(1);
    Main = [this](){ this->MainFunc(); };
    Teardown = [this](){ this->TeardownFunc(); };
    TaskScheduler::RegisterTask("Bandwidth", *this);
    m_needsUpdate = false;
    m_serverSocket = std::make_unique<ServerSocket>(m_port);
    m_serverSocket->Listen();
    
    std::thread handleThread([this]() { this->HandleClientData(); });
    std::swap(m_handleThread, handleThread);
    Logger::LogAdd("Server", "Network server started on port " + stringulate(this->m_port), NORMAL, GLF);
}

void D3PP::network::Server::TeardownFunc() {
    TaskScheduler::UnregisterTask("Bandwidth");

    if (m_handleThread.joinable())
        m_handleThread.join();
}

void D3PP::network::Server::TeardownFunc() {
    TaskScheduler::UnregisterTask("Bandwidth");

    if (m_handleThread.joinable())
        m_handleThread.join();
}

void D3PP::network::Server::Start() {
    if (m_Instance == nullptr) {
        m_Instance = new Server();
    }
}


void D3PP::network::Server::Stop() {
    if (m_Instance == nullptr)
        return;

    m_Instance->Shutdown();
    free(m_Instance);
}

void D3PP::network::Server::MainFunc() {
    BytesReceived = ReceivedIncrement/1024.0f;
    BytesSent = SentIncrement/1024.0f;
    ReceivedIncrement = 0;
    SentIncrement = 0;
    //Logger::LogAdd("Server", "Recv: " + stringulate(BytesReceived) + " KB/s, Sent: " + stringulate(BytesSent) + " KB/s.", DEBUG, GLF);
    HandleClientData();
}

void D3PP::network::Server::Shutdown() {
    m_serverSocket->Stop();
    {
        std::shared_lock lock(roMutex);
        for (auto const &c: roClients) {
            c->Kick("Server shutting down", false);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_handleThread.join();

    TaskScheduler::UnregisterTask("Bandwidth");
}

void D3PP::network::Server::HandleClientData() {
    HandleEvents();
    {
        std::shared_lock lock(roMutex);
        for (auto const &c: roClients) {
            if (c->IsDataAvailable())
                c->SendQueued();

            c->HandleData();
        }
    }
}

void D3PP::network::Server::HandleEvents() {
    auto e = m_serverSocket->CheckEvents();

    if (e.contains(SOCKET_EVENT_CONNECT)) {
        HandleIncomingClient();
    }
    if (e.contains(SOCKET_EVENT_DATA)) {
        for(auto &s : e[SOCKET_EVENT_DATA]) {
            int clientId = static_cast<int>(s);
            m_clients[clientId]->NotifyDataAvailable();
        }
    }

    if (m_needsUpdate)
        RebuildRoClients();
}

void D3PP::network::Server::HandleIncomingClient() {
    std::unique_ptr<Sockets> newClient = m_serverSocket->Accept();

    if (newClient != nullptr && newClient->GetSocketFd() != -1) {
        NetworkClient newNcClient(std::move(newClient));
        int clientId = newNcClient.GetId();
        RegisterClient(newNcClient);

        EventClientAdd eca;
        eca.clientId = clientId;
        Dispatcher::post(eca);
    }
}

void D3PP::network::Server::RegisterClient(NetworkClient client) {
    std::scoped_lock<std::mutex> clientLock(m_ClientMutex);
    m_clients.insert(std::make_pair(client.GetId(), std::make_shared<NetworkClient>(client)));
    RebuildRoClients();
}

void D3PP::network::Server::UnregisterClient(const std::shared_ptr<IMinecraftClient>& client) {
    EventClientDelete ecd;
    ecd.clientId = client->GetId();
    Dispatcher::post(ecd);
    m_Instance->m_serverSocket->Unaccept(client->GetId());
    m_Instance->m_needsUpdate = true;
    std::scoped_lock<std::mutex> clientLock(m_ClientMutex);
    m_clients.erase(client->GetId());
}

void D3PP::network::Server::RebuildRoClients() {
    std::vector<std::shared_ptr<IMinecraftClient>> newRo;
    newRo.reserve(m_clients.size());
    for(auto const &nc : m_clients) {
        newRo.push_back(nc.second);
    }

    std::unique_lock lock(roMutex);
    std::swap(newRo, roClients); // -- Should happen in an instant, but I suppose edge cases could happen \_(o_o)_/
}

void D3PP::network::Server::SendToAll(IPacket& packet, std::string extension, int extVersion) {
    std::shared_lock lock(roMutex, std::defer_lock);
    for ( auto const & c : roClients ) {
        if (!extension.empty()) {
            int currentVer = CPE::GetClientExtVersion(c, extension);

            if (extVersion == currentVer)
                c->SendPacket(packet);
        }

        c->SendPacket(packet);
    }
}

void D3PP::network::Server::SendAllExcept(IPacket& packet, std::shared_ptr<IMinecraftClient> toNot) {
    std::shared_lock lock(roMutex);
    for ( auto const & c : roClients ) {
        if (c == toNot)
            continue;

        c->SendPacket(packet);
    }
}






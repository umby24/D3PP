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
#include <events/EventClientAdd.h>
#include <events/EventClientDelete.h>

using namespace D3PP::network;

std::atomic<int> D3PP::network::Server::SentIncrement = 0;
float D3PP::network::Server::BytesSent = 0;
float D3PP::network::Server::BytesReceived = 0;
std::atomic<int> D3PP::network::Server::ReceivedIncrement = 0;

std::vector<std::shared_ptr<IMinecraftClient>> D3PP::network::Server::roClients;
std::map<int,std::shared_ptr<IMinecraftClient>> D3PP::network::Server::m_clients;
std::mutex D3PP::network::Server::m_ClientMutex;
Server* Server::m_Instance = nullptr;

D3PP::network::Server::Server() {
    this->m_Port = Configuration::NetSettings.ListenPort;

    Interval = std::chrono::seconds(5);
    Main = [this](){ this->MainFunc(); };

    TaskScheduler::RegisterTask("Bandwidth", *this);
    
    m_serverSocket = std::make_unique<ServerSocket>(m_Port);
    m_serverSocket->Listen();

    std::thread handleThread([this]() { this->HandleClientData(); });
    std::swap(m_handleThread, handleThread);

    Logger::LogAdd("Server", "Network server started on port " + stringulate(this->m_Port), LogType::NORMAL, GLF);
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
    Logger::LogAdd("Server", "Recv: " + stringulate(BytesReceived) + " KB/s, Sent: " + stringulate(BytesSent) + " KB/s.", DEBUG, GLF);
}

void D3PP::network::Server::Shutdown() {
    m_serverSocket->Stop();

    for (auto const& c : roClients) {
        c->Kick("Server shutting down", false);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m_handleThread.join();

    TaskScheduler::UnregisterTask("Bandwidth");
}

void D3PP::network::Server::HandleClientData() {
    while (System::IsRunning) {
        HandleEvents();

        for(auto const& c : roClients) {
            if (c->IsDataAvailable())
                c->SendQueued();

            c->HandleData();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void Server::HandleEvents() {
    ServerSocketEvent e = m_serverSocket->CheckEvents();

    if (e == ServerSocketEvent::SOCKET_EVENT_CONNECT) { 
        HandleIncomingClient();
    }

    if (e == ServerSocketEvent::SOCKET_EVENT_DATA) {
        int clientId = static_cast<int>(m_serverSocket->GetEventSocket());
        m_clients[clientId]->NotifyDataAvailable();
    }
}

void Server::HandleIncomingClient() {
    std::unique_ptr<Sockets> newClient = m_serverSocket->Accept();

    if (newClient != nullptr && newClient->GetSocketFd() != -1) {
        NetworkClient newNcClient(std::move(newClient));
        int clientId = newNcClient.GetId();

        RegisterClient(newNcClient.GetSelfPointer());

        EventClientAdd eca;
        eca.clientId = clientId;
        Dispatcher::post(eca);
    }
}

void D3PP::network::Server::RegisterClient(const std::shared_ptr<IMinecraftClient>& client) {
    std::scoped_lock<std::mutex> clientLock(m_ClientMutex);
    m_clients.insert(std::make_pair(client->GetId(), client));
    RebuildRoClients();
}

void D3PP::network::Server::UnregisterClient(const std::shared_ptr<IMinecraftClient>& client) {
    EventClientDelete ecd;
    ecd.clientId = client->GetId();
    Dispatcher::post(ecd);

    std::scoped_lock<std::mutex> clientLock(m_ClientMutex);
    m_clients.erase(client->GetId());
    m_Instance->m_serverSocket->Unaccept(client->GetId());
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

void D3PP::network::Server::SendToAll(IPacket& packet, std::string extension, int extVersion) {
    for ( auto const & c : roClients ) {
        if (!extension.empty()) {
            int currentVer = CPE::GetClientExtVersion(c, extension);

            if (extVersion == currentVer)
                c->SendPacket(const_cast<IPacket &>(packet));
        }

        c->SendPacket(packet);
    }
}

void D3PP::network::Server::SendAllExcept(IPacket& packet, std::shared_ptr<IMinecraftClient> toNot) {
    for ( auto const & c : roClients ) {
        if (c == toNot)
            continue;

        c->SendPacket(packet);
    }
}


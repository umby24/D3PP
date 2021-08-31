//
// Created by unknown on 2/18/2021.
//

#include "Network.h"
#include "NetworkClient.h"
#include <iomanip>

#include "json.hpp"
using json = nlohmann::json;

#ifndef __linux__
#include "network/WindowsServerSockets.h"
#include "network/WindowsSockets.h"
#else
#include "network/LinuxServerSockets.h"
#include "network/LinuxSockets.h"
#endif

#include "PacketHandlers.h"
#include "Entity.h"
#include "Player.h"
#include "Mem.h"
#include "Files.h"
#include "watchdog.h"
#include "Client.h"
#include "Logger.h"
#include "Utils.h"
#include "EventSystem.h"
#include "common/ByteBuffer.h"
#include "events/EventClientAdd.h"
#include "events/EventClientDelete.h"

const std::string MODULE_NAME = "Network";
Network* Network::singleton_ = nullptr;

Network::Network() : clientMutex() {
    isListening = false;
    TimerRate = 0;
    UploadRate = 0;
    UploadRateCounter = 0;
    DownloadRate = 0;
    DownloadRateCounter = 0;
    Port = 25565;
    lastModifiedTime = 0;
    SaveFile = false;

    TaskItem networkMain;
    networkMain.Interval = std::chrono::seconds(1);
    networkMain.Main = [this] { MainFunc(); };
    networkMain.Teardown = [this] { Stop(); };
    TaskScheduler::RegisterTask("Network_Main", networkMain);

    TaskItem networkStats;
    networkStats.Interval = std::chrono::seconds(5);
    networkStats.Main = [this] { UpdateNetworkStats(); };
    TaskScheduler::RegisterTask("Network_Stats", networkStats);

    TaskItem networkEvents;
    networkEvents.Interval = std::chrono::milliseconds(1);
    networkEvents.Main = [this] { NetworkEvents(); };
    TaskScheduler::RegisterTask("Network_Events", networkEvents);

    TaskItem networkOutputSender;
    networkOutputSender.Interval = std::chrono::milliseconds(0);
    networkOutputSender.Main = [this] { NetworkOutputSend(); };
    TaskScheduler::RegisterTask("Network_Output_Send", networkOutputSender);

    TaskItem networkOutputProcessor;
    networkOutputProcessor.Interval = std::chrono::milliseconds(0);
    networkOutputProcessor.Main = [this] { NetworkOutput(); };
    TaskScheduler::RegisterTask("Network_Output_Do", networkOutputProcessor);

    TaskItem networkInputProcessor;
    networkInputProcessor.Interval = std::chrono::milliseconds(0);
    networkInputProcessor.Main = [this] { NetworkInput(); };
    TaskScheduler::RegisterTask("Network_Input_Do", networkInputProcessor);
}

void Network::Load() {
    json j;
    Files* f = Files::GetInstance();
    std::string fileName = f->GetFile(MODULE_NAME);
    std::ifstream iFile(fileName);

    if (!iFile.is_open()) {
        this->Port = 25565;
        Logger::LogAdd(MODULE_NAME, "File could not be loaded", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    iFile >> j;
    iFile.close();

    this->Port = j["port"];

    Logger::LogAdd(MODULE_NAME, "File loaded", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    lastModifiedTime = Utils::FileModTime(fileName);
}

void Network::Save() {
    json j;
    Files* f = Files::GetInstance();
    std::string fileName = f->GetFile(MODULE_NAME);
    j["port"] = this->Port;

    std::ofstream oFile(fileName);
    if (!oFile.is_open()) {
        Logger::LogAdd(MODULE_NAME, "File could not be saved", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }
    oFile << std::setw(4) << j;
    oFile.close();

    Logger::LogAdd(MODULE_NAME, "File Saved", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    lastModifiedTime = Utils::FileModTime(fileName);
}

void Network::Start() {
    if (isListening)
        Stop();

    if (listenSocket == nullptr) {
        listenSocket = std::make_unique<ServerSocket>(this->Port);
    }

    listenSocket->Listen();
    isListening = true;

    Logger::LogAdd(MODULE_NAME, "Network server started on port " + stringulate(this->Port), LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Network::Stop() {
    if (!isListening)
        return;
    std::vector<int> toDelete;

    { // -- For scoped lock release.
        std::scoped_lock<std::mutex> sLock(clientMutex);
        for(auto const &nc : _clients) { // -- Because this list is going to be modified.
            toDelete.push_back(nc.first);
        }
    }

    for (auto const id : toDelete) {
        DeleteClient(id, "Client Disconnected", true);
    }

    listenSocket->Stop();
    isListening = false;
    Logger::LogAdd(MODULE_NAME, "Network server stopped", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

std::shared_ptr<NetworkClient> Network::GetClient(int id) {
    std::shared_ptr<NetworkClient> result = nullptr;
    for(auto const &nc : roClients) {
        if (nc->Id == id) {
            result = nc;
            break;
        }
    }

    return result;
}

void Network::MainFunc() {
    watchdog::Watch("Main", "Before: Network MainFunc()", 1);

    if (SaveFile) {
        Save();
        SaveFile = false;
    }

    Files* f = Files::GetInstance();
    std::string networkFile = f->GetFile(MODULE_NAME);
    time_t modTime = Utils::FileModTime(networkFile);

    if (modTime != lastModifiedTime) {
        Load();
        lastModifiedTime = modTime;
    }
}

void Network::HtmlStats() {
    time_t startTime = time(nullptr);
    std::string result = NETWORK_HTML;

    Utils::replaceAll(result, "[PORT]", stringulate(this->Port));
    Utils::replaceAll(result, "[DRATE]", stringulate(this->DownloadRate));
    Utils::replaceAll(result, "[URATE]", stringulate(this->UploadRate));

    // -- Client Table Generation
    std::string clientTable;
    for (auto const& nc : roClients) {
        if (nc->LoggedIn && nc->player != nullptr) {
            clientTable += "<tr>";
            clientTable += "<td>" + stringulate(nc->Id) + "</td>";
            clientTable += "<td>" + nc->player->LoginName + "</td>";
            clientTable += "<td>" + stringulate(nc->player->ClientVersion) + "</td>";
            clientTable += "<td>" + nc->IP + "</td>";
            clientTable += "<td>" + stringulate(nc->DownloadRate / 1000) + "</td>";
            clientTable += "<td>" + stringulate(nc->UploadRate / 1000) + "</td>";
            if (nc->player->tEntity) {
                clientTable += "<td>" + stringulate(nc->player->tEntity->Id) + "</td>";
            }
            clientTable += "</tr>";
        }
    }
    // --
    time_t finishTime = time(nullptr);
    long duration = finishTime - startTime;
    char buffer[255];
    strftime(buffer, sizeof(buffer), "%H:%M:%S  %m-%d-%Y", localtime(reinterpret_cast<const time_t *>(&finishTime)));
    std::string meh(buffer);
    Utils::replaceAll(result, "[GEN_TIME]", stringulate(duration));
    Utils::replaceAll(result, "[GEN_TIMESTAMP]", meh);

    Files* files = Files::GetInstance();
    std::string memFile = files->GetFile(NETWORK_HTML_FILENAME);

    std::ofstream oStream(memFile, std::ios::out | std::ios::trunc);
    if (oStream.is_open()) {
        oStream << result;
        oStream.close();
    } else {
        Logger::LogAdd(MODULE_NAME, "Couldn't open file :<" + memFile, LogType::WARNING, __FILE__, __LINE__, __FUNCTION__ );
    }
}

void Network::UpdateNetworkStats() {
    for (auto const& nc : roClients) {
        nc->DownloadRate = nc->DownloadRateCounter / 5;
        nc->UploadRate = nc->UploadRateCounter / 5;
        nc->DownloadRateCounter = 0;
        nc->UploadRateCounter = 0;
    }

    DownloadRate = DownloadRateCounter / 5;
    UploadRate = UploadRateCounter / 5;
    DownloadRateCounter = 0;
    UploadRateCounter = 0;

    HtmlStats();
}

void Network::NetworkEvents() {
    watchdog::Watch("Network", "Begin events", 0);

    while (isListening) {
        ServerSocketEvent e = listenSocket->CheckEvents();

        if (e == ServerSocketEvent::SOCKET_EVENT_CONNECT) { 
            std::unique_ptr<Sockets> newClient = listenSocket->Accept();

            if (newClient != nullptr && newClient->GetSocketFd() != -1) {
                NetworkClient newNcClient(std::move(newClient));
                int clientId = newNcClient.Id;
                {
                    std::scoped_lock<std::mutex> sLock(clientMutex);

                    _clients.insert(std::make_pair(clientId, std::make_shared<NetworkClient>(newNcClient)));
                    std::vector<std::shared_ptr<NetworkClient>> newRo;
                    for(auto const &nc : _clients) {
                        newRo.push_back(nc.second);
                    }
                    std::swap(newRo, roClients);
                }

                EventClientAdd eca;
                eca.clientId = clientId;
                Dispatcher::post(eca);
                _clients.at(clientId)->canReceive = true;
                _clients.at(clientId)->canSend = true;
            }
        } else if (e == ServerSocketEvent::SOCKET_EVENT_DATA) {
            int clientId = static_cast<int>(listenSocket->GetEventSocket());
            std::shared_ptr<NetworkClient> client = GetClient(clientId);
            if (client->canReceive) {
                auto* receiveBuf = new char[1026];
                receiveBuf[1024] = 99;
                int dataRead = client->clientSocket->Read(receiveBuf, 1024);
                if (dataRead > 0) {
                    std::vector<unsigned char> receive(receiveBuf, receiveBuf+dataRead);
                    delete[] receiveBuf;
                    unsigned long mySize = receive.size();
                    client->ReceiveBuffer->Write(receive, dataRead);
                    client->DownloadRateCounter += dataRead;
                    DownloadRateCounter += dataRead;
                } else {
                    delete[] receiveBuf;
                    DeleteClient(clientId, "Disconnected", true);
                    break;
                }
            }
        } else {
            break;
        }

    }
    for(auto const &nc : roClients) {
        if (nc->DisconnectTime > 0 && nc->DisconnectTime < time(nullptr)) {
            DeleteClient(nc->Id, "Forced Disconnect", true);
            break;
        }
//        } else if (nc.second->LastTimeEvent + NETWORK_CLIENT_TIMEOUT < time(nullptr)) {
//            DeleteClient(nc.first, "Timeout", true);
//            break;
//        }
    }
    watchdog::Watch("Network", "End Events", 2);
}

void Network::NetworkOutputSend() {
    for(auto const &nc : roClients) {
        if (nc->DataAvailable && nc->canSend) {
            int sendSize = nc->SendBuffer->Size();
            std::vector<unsigned char> allBytes = nc->SendBuffer->GetAllBytes();

            int bytesSent = nc->clientSocket->Send(reinterpret_cast<char *>(allBytes.data()), sendSize);

            nc->DataAvailable = false;
            nc->UploadRateCounter += bytesSent;
            UploadRateCounter += bytesSent;
        }
    }
}

void Network::NetworkOutput() {
    for (auto const &nc : roClients) {
        if (nc->PingTime < time(nullptr))  {
            nc->PingTime = time(nullptr) + 5;
            nc->PingSentTime = time(nullptr);
            nc->OutputPing();
        }
    }
}

void Network::NetworkInput() {
    // -- Packet handling
    for(auto const &nc : roClients) {
        int maxRepeat = 10;
        while (nc->ReceiveBuffer->Size() > 0 && maxRepeat > 0 && nc->canReceive) {
            unsigned char commandByte = nc->ReceiveBuffer->PeekByte();
            //nc->InputAddOffset(-1);
            nc->LastTimeEvent = time(nullptr);

            switch(commandByte) {
                case 0: // -- Login
                    if (nc->ReceiveBuffer->Size() >= 1 + 1 + 64 + 64 + 1) {
                        nc->ReceiveBuffer->ReadByte();
                        PacketHandlers::HandleHandshake(nc);
                        nc->ReceiveBuffer->Shift(1 + 1 + 64 + 64 + 1);
                    }
                    break;
                case 1: // -- Ping
                    if (nc->ReceiveBuffer->Size() >= 1) {
                        nc->ReceiveBuffer->ReadByte();
                        PacketHandlers::HandlePing(nc);
                        nc->ReceiveBuffer->Shift(1);
                    }
                    break;
                case 5: // -- Block Change
                    if (nc->ReceiveBuffer->Size() >= 9) {
                        nc->ReceiveBuffer->ReadByte();
                        PacketHandlers::HandleBlockChange(nc);
                        nc->ReceiveBuffer->Shift(9);
                    }
                    break;
                case 8: // -- Player Movement
                    if (nc->ReceiveBuffer->Size() >= 10) {
                        nc->ReceiveBuffer->ReadByte();
                        PacketHandlers::HandlePlayerTeleport(nc);
                        nc->ReceiveBuffer->Shift(10);
                    }
                    break;
                case 13: // -- Chat Message
                    if (nc->ReceiveBuffer->Size() >= 66) {
                        nc->ReceiveBuffer->ReadByte();
                        PacketHandlers::HandleChatPacket(nc);
                        nc->ReceiveBuffer->Shift(66);
                    }
                    break;
                case 16: // -- CPe ExtInfo
                    if (nc->ReceiveBuffer->Size() >= 67) {
                        nc->ReceiveBuffer->ReadByte();
                        PacketHandlers::HandleExtInfo(nc);
                        nc->ReceiveBuffer->Shift(67);
                    }
                    break;
                case 17: // -- CPE ExtEntry
                    if (nc->ReceiveBuffer->Size() >= 1 + 64 + 4) {
                        nc->ReceiveBuffer->ReadByte();
                        PacketHandlers::HandleExtEntry(nc);
                        nc->ReceiveBuffer->Shift(69);
                    }
                    break;
                case 19: // -- CPE Custom Block Support
                    if (nc->ReceiveBuffer->Size() >= 2) {
                        nc->ReceiveBuffer->ReadByte();
                        PacketHandlers::HandleCustomBlockSupportLevel(nc);
                        nc->ReceiveBuffer->Shift(2);
                    }
                    break;
                default:
                    Logger::LogAdd(MODULE_NAME, "Unknown Packet Received [" + stringulate(commandByte) + "]", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
                    nc->Kick("Invalid Packet", true);
            }

            maxRepeat--;
        } // -- /While

    } // -- /For
}

void Network::DeleteClient(int clientId, const std::string& message, bool sendToAll) {
    std::shared_ptr<NetworkClient> client = GetClient(clientId);
    if (client == nullptr)
        return;

    Client::Logout(clientId, message, sendToAll);
    client->canSend = false;
    client->canReceive = false;
    listenSocket->Unaccept(client->clientSocket->GetSocketFd());
    
    EventClientDelete ecd;
    ecd.clientId = clientId;
    Dispatcher::post(ecd);

    Logger::LogAdd(MODULE_NAME, "Client deleted [" + stringulate(clientId) + "] [" + message + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    client->clientSocket->Disconnect();

    std::scoped_lock<std::mutex> sLock(clientMutex);
    _clients.erase(clientId);
    std::vector<std::shared_ptr<NetworkClient>> newRo;
    for(auto const &nc : _clients) {
        newRo.push_back(nc.second);
    }
    std::swap(newRo, roClients);
}

Network *Network::GetInstance() {
    if (singleton_ == nullptr)
        singleton_ = new Network();

    return singleton_;
}

//
// Created by unknown on 2/18/2021.
//

#include "Network.h"
const std::string MODULE_NAME = "Network";

Network::Network() {
    TempBuffer = Mem::Allocate(NETWORK_TEMP_BUFFER_SIZE, __FILE__, __LINE__, "Network\\TempBuffer");
}

void Network::Load() {
    json j;
    Files* f = Files::GetInstance();
    std::string fileName = f->GetFile(MODULE_NAME);
    ifstream iFile(fileName);

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

    ofstream oFile(fileName);
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

    ServerSocket mSock(this->Port);
    swap(listenSocket, mSock);

    listenSocket.Listen();
    isListening = true;
    Logger::LogAdd(MODULE_NAME, "Network server started on port " + stringulate(this->Port), LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Network::Stop() {
    if (!isListening)
        return;
    // -- TODO: Disconnect all clients
    listenSocket.Stop();
    isListening = false;
    Logger::LogAdd(MODULE_NAME, "Network server stopped", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

NetworkClient *Network::GetClient(int id) {
    if (this->_clients.find(id) == this->_clients.end())
        return nullptr;

    return &_clients.at(id);
}

void Network::AddClient(Sockets clientSocket) {
    NetworkClient newClient(clientSocket);
    _clients[newClient.Id] = newClient;
}

void Network::meh() {
    Sockets newClient = listenSocket.Accept();
    AddClient(newClient);
}

NetworkClient::NetworkClient(const Sockets& socket) : clientSocket(socket) {
    Id= reinterpret_cast<uintptr_t>(&clientSocket);
    InputBuffer = Mem::Allocate(NETWORK_BUFFER_SIZE, __FILE__, __LINE__, "NetworkClient(" + stringulate(Id) + ")\\InputBuffer");
    OutputBuffer = Mem::Allocate(NETWORK_BUFFER_SIZE, __FILE__, __LINE__, "NetworkClient(" + stringulate(Id) + ")\\OutputBuffer");
    InputBufferAvailable = 0;
    InputBufferOffset = 0;
    OutputBufferOffset = 0;
    OutputBufferAvailable = 0;
    LastTimeEvent = time(nullptr);

    Logger::LogAdd(MODULE_NAME, "Client Created [" + stringulate(Id) + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

NetworkClient::NetworkClient() {

}

//
// Created by unknown on 2/18/2021.
//

#include "Network.h"
const std::string MODULE_NAME = "Network";
Network* Network::singleton_ = nullptr;

Network::Network() {
    TempBuffer = Mem::Allocate(NETWORK_TEMP_BUFFER_SIZE, __FILE__, __LINE__, "Network\\TempBuffer");
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

    ServerSocket mSock(this->Port);
    std::swap(listenSocket, mSock);

    listenSocket.Listen();
    isListening = true;

    Logger::LogAdd(MODULE_NAME, "Network server started on port " + stringulate(this->Port), LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Network::Stop() {
    if (!isListening)
        return;
    std::vector<int> toDelete;

    for(auto const &nc : _clients) { // -- Because this list is going to be modified.
        toDelete.push_back(nc.first);
    }

    for (auto const id : toDelete) {
        DeleteClient(id, "Client Disconnected", true);
    }

    listenSocket.Stop();
    isListening = false;
    Logger::LogAdd(MODULE_NAME, "Network server stopped", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

std::shared_ptr<NetworkClient> Network::GetClient(int id) {
    if (this->_clients.find(id) == this->_clients.end())
        return nullptr;

    return _clients.at(id);
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
    for (auto const& nc : _clients) {
        if (nc.second->LoggedIn && nc.second->player != nullptr) {
            clientTable += "<tr>";
            clientTable += "<td>" + stringulate(nc.first) + "</td>";
            clientTable += "<td>" + nc.second->player->LoginName + "</td>";
            clientTable += "<td>" + stringulate(nc.second->player->ClientVersion) + "</td>";
            clientTable += "<td>" + nc.second->IP + "</td>";
            clientTable += "<td>" + stringulate(nc.second->DownloadRate / 1000) + "</td>";
            clientTable += "<td>" + stringulate(nc.second->UploadRate / 1000) + "</td>";
            if (nc.second->player->tEntity) {
                clientTable += "<td>" + stringulate(nc.second->player->tEntity->Id) + "</td>";
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
    for (auto const& nc : _clients) {
        nc.second->DownloadRate = nc.second->DownloadRateCounter / 5;
        nc.second->UploadRate = nc.second->UploadRateCounter / 5;
        nc.second->DownloadRateCounter = 0;
        nc.second->UploadRateCounter = 0;
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
        ServerSocketEvent e = listenSocket.CheckEvents();

        if (e == ServerSocketEvent::SOCKET_EVENT_CONNECT) { 
            std::unique_ptr<Sockets> newClient = listenSocket.Accept();

            if (newClient != nullptr) {
                NetworkClient newNcClient(std::move(newClient));
                int clientId = newNcClient.Id;
                _clients.insert(std::make_pair(clientId, std::make_shared<NetworkClient>(std::move(newNcClient))));
            }
        } else if (e == ServerSocketEvent::SOCKET_EVENT_DATA) {
            int clientId = static_cast<int>(listenSocket.GetEventSocket());
            std::shared_ptr<NetworkClient> client = GetClient(clientId);
            int dataRead = client->clientSocket->Read(TempBuffer, NETWORK_TEMP_BUFFER_SIZE);

            if (dataRead > 0) {
                client->InputWriteBuffer(TempBuffer, dataRead);
                client->DownloadRateCounter += dataRead;
                DownloadRateCounter += dataRead;
            } else {
                DeleteClient(clientId, "Disconnected", true);                        
                break;
            }

        } else {
            break;
        }

    }
    for(auto const &nc : _clients) {
        if (nc.second->DisconnectTime > 0 && nc.second->DisconnectTime < time(nullptr)) {
            DeleteClient(nc.first, "Forced Disconnect", true);
            nc.second->clientSocket->Disconnect();
        } else if (nc.second->LastTimeEvent + NETWORK_CLIENT_TIMEOUT < time(nullptr)) {
            DeleteClient(nc.first, "Timeout", true);
            nc.second->clientSocket->Disconnect();
        }
    }
    watchdog::Watch("Network", "End Events", 2);
}

void Network::NetworkOutputSend() {
    for(auto const &nc : _clients) {
        auto availData = nc.second->OutputBufferAvailable;
        while (availData > 0) {
            int dataSize = availData;

            if (dataSize > NETWORK_PACKET_SIZE)
                dataSize = NETWORK_PACKET_SIZE;
            else if (dataSize > NETWORK_TEMP_BUFFER_SIZE)
                dataSize = NETWORK_TEMP_BUFFER_SIZE;

            nc.second->OutputReadBuffer(TempBuffer, dataSize);
            nc.second->OutputAddOffset(-dataSize);
            int bytesSent = nc.second->clientSocket->Send(TempBuffer, dataSize);

            if (bytesSent > 0) {
                nc.second->OutputAddOffset(bytesSent);
                nc.second->UploadRateCounter += bytesSent;
                UploadRateCounter += bytesSent;
                availData = nc.second->OutputBufferAvailable;
            }
        }
    }
}

void Network::NetworkOutput() {
    for (auto const &nc : _clients) {
        if (nc.second->PingTime < time(nullptr))  {
            nc.second->PingTime = time(nullptr) + 5;
            nc.second->PingSentTime = time(nullptr);
            nc.second->OutputPing();
        }
    }
}

void Network::NetworkInput() {
    // -- Packet handling
    for(auto const &nc : _clients) {
        int maxRepeat = 10;
        while (nc.second->InputBufferAvailable >= 1 && maxRepeat > 0) {
            char commandByte = nc.second->InputReadByte();
            nc.second->InputAddOffset(-1);
            nc.second->LastTimeEvent = time(nullptr);

            switch(commandByte) {
                case 0: // -- Login
                    if (nc.second->InputBufferAvailable >= 1 + 1 + 64 + 64 + 1) {
                        PacketHandlers::HandleHandshake(nc.second);
                    }
                    break;
                case 1: // -- Ping
                    if (nc.second->InputBufferAvailable >= 1) {
                        PacketHandlers::HandlePing(nc.second);
                    }
                    break;
                case 5: // -- Block Change
                    if (nc.second->InputBufferAvailable >= 9) {
                        PacketHandlers::HandleBlockChange(nc.second);
                    }
                    break;
                case 8: // -- Player Movement
                    if (nc.second->InputBufferAvailable >= 10) {
                        PacketHandlers::HandlePlayerTeleport(nc.second);
                    }
                    break;
                case 13: // -- Chat Message
                    if (nc.second->InputBufferAvailable >= 66) {
                        PacketHandlers::HandleChatPacket(nc.second);
                    }
                    break;
                case 16: // -- CPe ExtInfo
                    if (nc.second->InputBufferAvailable >= 67) {
                        PacketHandlers::HandleExtInfo(nc.second);
                    }
                    break;
                case 17: // -- CPE ExtEntry
                    if (nc.second->InputBufferAvailable >= 1 + 64 + 4) {
                        PacketHandlers::HandleExtEntry(nc.second);
                    }
                    break;
                case 19: // -- CPE Custom Block Support
                    if (nc.second->InputBufferAvailable >= 2) {
                        PacketHandlers::HandleCustomBlockSupportLevel(nc.second);
                    }
                    break;
                default:
                    Logger::LogAdd(MODULE_NAME, "Unknown Packet Received [" + stringulate(commandByte) + "]", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
                    nc.second->Kick("Invalid Packet", true);
            }

            maxRepeat--;
        } // -- /While

    } // -- /For
}

NetworkClient::NetworkClient(std::unique_ptr<Sockets> socket) {
    Id= static_cast<int>(socket->GetSocketFd());
    InputBuffer = Mem::Allocate(NETWORK_BUFFER_SIZE, __FILE__, __LINE__, "NetworkClient(" + stringulate(Id) + ")\\InputBuffer");
    OutputBuffer = Mem::Allocate(NETWORK_BUFFER_SIZE, __FILE__, __LINE__, "NetworkClient(" + stringulate(Id) + ")\\OutputBuffer");
    InputBufferAvailable = 0;
    InputBufferOffset = 0;
    CustomExtensions = 0;
    OutputBufferOffset = 0;
    OutputBufferAvailable = 0;
    LastTimeEvent = time(nullptr);
    clientSocket = std::move(socket);
    PingTime = time(nullptr) + 5;
    DisconnectTime = 0;
    LoggedIn = false;
    UploadRate = 0;
    DownloadRate = 0;
    UploadRateCounter = 0;
    DownloadRateCounter = 0;
    CPE = false;
    PingSentTime = PingTime;
    Ping = 0;
    CustomBlocksLevel = 0;
    GlobalChat = false;
    //IP = socket->GetSocketIp();

    Logger::LogAdd(MODULE_NAME, "Client Created [" + stringulate(Id) + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

NetworkClient::NetworkClient() {
}

void NetworkClient::OutputReadBuffer(char* dataBuffer, int size) {
    int dataRead = 0;
    while (dataRead < size) {
        int ringbufferMaxData = NETWORK_BUFFER_SIZE - (OutputBufferOffset);
        char* bufferAddress = OutputBuffer + OutputBufferOffset;
        int dataTempSize = size - dataRead;

        if (dataTempSize > ringbufferMaxData)
            dataTempSize = ringbufferMaxData;

        memcpy(dataBuffer + dataRead, bufferAddress, dataTempSize);
        dataRead += dataTempSize;
        OutputBufferOffset += dataTempSize;
        OutputBufferAvailable -= dataTempSize;

        if (OutputBufferOffset >= NETWORK_BUFFER_SIZE) {
            OutputBufferOffset -= NETWORK_BUFFER_SIZE;
        }
    }
}

void NetworkClient::OutputAddOffset(int bytes) {
    OutputBufferOffset += bytes;
    OutputBufferAvailable -= bytes;
    if (OutputBufferOffset < 0)
        OutputBufferOffset += NETWORK_BUFFER_SIZE;


    if (OutputBufferOffset >= NETWORK_BUFFER_SIZE)
        OutputBufferOffset -= NETWORK_BUFFER_SIZE;
}

void NetworkClient::OutputPing() {
    OutputWriteByte(1);
}

void NetworkClient::OutputWriteByte(char value) {
    int finalOffset = ((OutputBufferOffset + OutputBufferAvailable) % NETWORK_BUFFER_SIZE);
    OutputBuffer[finalOffset] = value;
    OutputBufferAvailable += 1;
}

// -- Writes data into a clients input buffer after being received off a socket.
void NetworkClient::InputWriteBuffer(char *data, int size) {
    int dataWrote = 0;

    while (dataWrote < size) {
        int writeOffset = (InputBufferOffset + InputBufferAvailable) % NETWORK_BUFFER_SIZE;
        int bufferMaxData = NETWORK_BUFFER_SIZE - (writeOffset);
        int dataTempSize = size - dataWrote;

        if (dataTempSize > bufferMaxData)
            dataTempSize = bufferMaxData;

        memcpy(InputBuffer + writeOffset, data+ dataWrote, dataTempSize);
        dataWrote += dataTempSize;
        InputBufferAvailable += dataTempSize;
    }
}

void NetworkClient::Kick(std::string message, bool hide) {
    NetworkFunctions::SystemRedScreen(this->Id, message);

    if (DisconnectTime == 0) {
        DisconnectTime = time(nullptr) + 1;
        LoggedIn = false;
        player->LogoutHide = hide;
        Logger::LogAdd(MODULE_NAME, "Client Kicked [" + message + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
}

void NetworkClient::InputAddOffset(int bytes) {
    InputBufferOffset += bytes;
    InputBufferAvailable -= bytes;

    if (InputBufferOffset < 0)
        InputBufferOffset += NETWORK_BUFFER_SIZE;

    if (InputBufferOffset >= NETWORK_BUFFER_SIZE)
        InputBufferOffset -= NETWORK_BUFFER_SIZE;
}

char NetworkClient::InputReadByte() {
    char result;
    if (InputBufferAvailable >= 1) {
        result = InputBuffer[InputBufferOffset];
        InputBufferOffset += 1;
        InputBufferAvailable -= 1;

        if (InputBufferOffset >= NETWORK_BUFFER_SIZE)
            InputBufferOffset -= NETWORK_BUFFER_SIZE;
    }
    return result;
}

void NetworkClient::OutputWriteShort(short value) {
    int location = ((OutputBufferOffset + OutputBufferAvailable) % NETWORK_BUFFER_SIZE);
    OutputBuffer[location++] = (unsigned char) (value >> 8);
    OutputBuffer[location++] = (unsigned char) value;
    OutputBufferAvailable += 2;
}

void NetworkClient::OutputWriteInt(int value) {
    int location = ((OutputBufferOffset + OutputBufferAvailable) % NETWORK_BUFFER_SIZE);
    OutputBuffer[location++] = (unsigned char) (value >> 24);
    OutputBuffer[location++] = (unsigned char) (value >> 16);
    OutputBuffer[location++] = (unsigned char) (value >> 8);
    OutputBuffer[location++] = (unsigned char) value;
    OutputBufferAvailable += 4;
}

void NetworkClient::OutputWriteString(std::string value) {
    if (value.size() < 64) {
        Utils::padTo(value, 64);
    }
    const char *meh = value.c_str();
    OutputWriteBlob(meh, value.size());
}

void NetworkClient::OutputWriteBlob(const char *data, int dataSize) {
    int dataWrote = 0;
    while (dataWrote < dataSize) {
        int writeOffset = ((OutputBufferOffset + OutputBufferAvailable) % NETWORK_BUFFER_SIZE);
        int maxOffset = NETWORK_BUFFER_SIZE - writeOffset;
        int dataTempSize = dataSize - dataWrote;
        if (dataTempSize > maxOffset)
            dataTempSize = maxOffset;
        memcpy(OutputBuffer + writeOffset, data + dataWrote, dataTempSize);
        dataWrote += dataTempSize;
        OutputBufferAvailable += dataTempSize;
    }
}

int NetworkClient::InputReadInt() {
    int result = 0;

    if (InputBufferAvailable >= 4) {
        //__builtin_bswap32
        result = InputBuffer[InputBufferOffset++] << 24;
        result |= (InputBuffer[InputBufferOffset++] <<  16) & 0x00ff0000;
        result |= (InputBuffer[InputBufferOffset++] <<  8) & 0x0000ff00;
        result |= (InputBuffer[InputBufferOffset++]) & 0x000000ff;

        InputBufferAvailable -= 4;

        if (InputBufferOffset >= NETWORK_BUFFER_SIZE)
            InputBufferOffset -= NETWORK_BUFFER_SIZE;

        return result;
    }

    return -1;
}

std::string NetworkClient::InputReadString() {
    char* tempBuffer = Mem::Allocate(64, __FILE__, __LINE__, "Temp_Buffer");

    if (InputBufferAvailable >= 64) {
        InputReadBytes(tempBuffer, 64);
    }

    std::string result(tempBuffer, 64);
    Mem::Free(tempBuffer);
    Utils::TrimString(result);
    return result;
}

void NetworkClient::InputReadBytes(char *data, int datalen) {
    int dataRead = 0;

    while (dataRead < datalen) {
        int ringbufferMaxData = NETWORK_BUFFER_SIZE - (InputBufferOffset);
        int tempDataSize = datalen - dataRead;

        if (tempDataSize > ringbufferMaxData)
            tempDataSize = ringbufferMaxData;

        memcpy(data + dataRead, InputBuffer + InputBufferOffset, tempDataSize);
        dataRead += tempDataSize;
        InputBufferOffset += tempDataSize;
        InputBufferAvailable -= tempDataSize;

        if (InputBufferOffset >= NETWORK_BUFFER_SIZE)
            InputBufferOffset -= NETWORK_BUFFER_SIZE;
    }
}

short NetworkClient::InputReadShort() {
    short result = 0;

    if (InputBufferAvailable >= 2) {
        result = InputBuffer[InputBufferOffset++] * 256;
        result += InputBuffer[InputBufferOffset++]&255;
        InputBufferAvailable -= 2;

        if (InputBufferOffset >= NETWORK_BUFFER_SIZE)
            InputBufferOffset -= NETWORK_BUFFER_SIZE;
        return result;
    }

    return -1;
}

void Network::DeleteClient(int clientId, std::string message, bool sendToAll) {
    if (_clients.find(clientId) == _clients.end())
        return;

    // -- Plugin event client delete
    Client::Logout(clientId, message, sendToAll);
    Mem::Free(_clients[clientId]->InputBuffer);
    Mem::Free(_clients[clientId]->OutputBuffer);
    listenSocket.Unaccept(_clients[clientId]->clientSocket->GetSocketFd());
    Logger::LogAdd(MODULE_NAME, "Client deleted [" + stringulate(clientId) + "] [" + message + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    _clients[clientId]->clientSocket->Disconnect();
    _clients.erase(clientId);
}

Network *Network::GetInstance() {
    if (singleton_ == nullptr)
        singleton_ = new Network();

    return singleton_;
}

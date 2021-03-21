//
// Created by unknown on 2/18/2021.
//

#include "Network.h"
const std::string MODULE_NAME = "Network";
Network* Network::singleton_ = nullptr;

Network::Network() {
    TempBuffer = Mem::Allocate(NETWORK_TEMP_BUFFER_SIZE, __FILE__, __LINE__, "Network\\TempBuffer");

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

    // -- In D3, this is normally performed on the main thread because PureBasic has a non-blocking thread model.
    // -- However, due to the nature of our socket implementation being blocking and the possiblility
    // -- of future performance implications, client acceptance will be performed on a separate thread.

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

    std::thread watcher(&Network::ClientAcceptance, this);
    swap(watcher, _acceptThread);

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
        clientTable += "<tr>";
        clientTable += "<td>" + stringulate(nc.first) + "</td>";
        // -- TODO: Player name
        //clientTable += "<td>" + stringulate(nc.first) + "</td>";
        // -- TODO: Player Version
        //clientTable += "<td>" + stringulate(nc.first) + "</td>";
        clientTable += "<td>" + nc.second->IP + "</td>";
        clientTable += "<td>" + stringulate(nc.second->DownloadRate / 1000) + "</td>";
        clientTable += "<td>" + stringulate(nc.second->UploadRate / 1000) + "</td>";
        // -- TODO: Entity ID
        //clientTable += "<td>" + stringulate(nc.first) + "</td>";
        clientTable += "</tr>";
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

    ofstream oStream(memFile, std::ios::out | std::ios::trunc);
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
    // -- Search for incoming connections [Handled in a different thread]
    // -- search for incoming data from clients
    watchdog::Watch("Network", "Begin events", 0);

    for(auto const &nc : _clients) {
        int dataRead = nc.second->clientSocket->Read(TempBuffer, NETWORK_TEMP_BUFFER_SIZE);
        if (dataRead > 0) {
            nc.second->InputWriteBuffer(TempBuffer, dataRead);
            nc.second->DownloadRateCounter += dataRead;
            DownloadRateCounter += dataRead;
        }

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
            nc.second->PingTime = time(nullptr) + 5000;
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

                    }
                    break;
                case 1: // -- Ping
                    if (nc.second->InputBufferAvailable >= 1) {

                    }
                    break;
                case 5: // -- Block Change
                    if (nc.second->InputBufferAvailable >= 9) {

                    }
                    break;
                case 8: // -- Player Movement
                    if (nc.second->InputBufferAvailable >= 10) {

                    }
                    break;
                case 13: // -- Chat Message
                    if (nc.second->InputBufferAvailable >= 66) {

                    }
                    break;
                case 16: // -- CPe ExtInfo
                    if (nc.second->InputBufferAvailable >= 67) {

                    }
                    break;
                case 17: // -- CPE ExtEntry
                    if (nc.second->InputBufferAvailable >= 1 + 64 + 4) {

                    }
                    break;
                case 19: // -- CPE Custom Block Support
                    if (nc.second->InputBufferAvailable >= 2) {

                    }
                    break;
                default:
                    Logger::LogAdd(MODULE_NAME, "Unknown Packet Recieved [" + stringulate(commandByte) + "]", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
                    // -- Kick
            }

            maxRepeat--;
        }
    }
}

void Network::ClientAcceptance() {
    while (isListening) {
        try {
            unique_ptr<Sockets> newClient = listenSocket.Accept();

            if (newClient != nullptr) {
                NetworkClient newNcClient(std::move(newClient));
                _clients.insert(std::make_pair(newNcClient.Id, std::make_shared<NetworkClient>(std::move(newNcClient))));
            }
        } catch (...) {
            continue;
        }

    }
}

NetworkClient::NetworkClient(unique_ptr<Sockets> socket) {
    Id= reinterpret_cast<uintptr_t>(&clientSocket);
    InputBuffer = Mem::Allocate(NETWORK_BUFFER_SIZE, __FILE__, __LINE__, "NetworkClient(" + stringulate(Id) + ")\\InputBuffer");
    OutputBuffer = Mem::Allocate(NETWORK_BUFFER_SIZE, __FILE__, __LINE__, "NetworkClient(" + stringulate(Id) + ")\\OutputBuffer");
    InputBufferAvailable = 0;
    InputBufferOffset = 0;
    OutputBufferOffset = 0;
    OutputBufferAvailable = 0;
    LastTimeEvent = time(nullptr);
    clientSocket = std::move(socket);

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
    // -- System_red_screen
    if (DisconnectTime == 0) {
        DisconnectTime = time(nullptr) + 1000;
        LoggedIn = false;
        // -- LogoutHide = hide
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

}

void NetworkClient::OutputWriteInt(int value) {

}

void NetworkClient::OutputWriteString(std::string value) {

}

void NetworkClient::OutputWriteBlob(char *data, int dataSize) {

}

void Network::DeleteClient(int clientId, std::string message, bool sendToAll) {
    if (_clients.find(clientId) == _clients.end())
        return;

    // -- Plugin event client delete
    // -- Client_Logout
    Mem::Free(_clients[clientId]->InputBuffer);
    Mem::Free(_clients[clientId]->OutputBuffer);
    Logger::LogAdd(MODULE_NAME, "Client deleted [" + stringulate(clientId) + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    _clients.erase(clientId);
}

Network *Network::GetInstance() {
    if (singleton_ == nullptr)
        singleton_ = new Network();

    return singleton_;
}

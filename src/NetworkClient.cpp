//
// Created by unknown on 8/24/21.
//

#include "NetworkClient.h"
#include "Utils.h"
#include <memory>
#include <utility>
#include "common/ByteBuffer.h"
#include "Logger.h"
#include "Network_Functions.h"
#include "Player.h"
#include "Network.h"
#include "CPE.h"
#include "Packets.h"
#include "Entity.h"
#include "MinecraftLocation.h"

#ifndef __linux__
#include "network/WindowsSockets.h"
#else
#include "network/LinuxSockets.h"
#endif

const std::string MODULE_NAME = "NetworkClient";

NetworkClient::NetworkClient(std::unique_ptr<Sockets> socket) : Selections(MAX_SELECTION_BOXES) {
    if (socket->GetSocketFd() == -1) {
        return;
    }

    Id= static_cast<int>(socket->GetSocketFd());
    CustomExtensions = 0;
    SendBuffer = std::make_unique<ByteBuffer>([this]{this->DataReady();});
    ReceiveBuffer = std::make_unique<ByteBuffer>(nullptr);
    DataAvailable = false;
    canReceive = true;
    canSend = true;
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
    IP = clientSocket->GetSocketIp();

    Logger::LogAdd(MODULE_NAME, "Client Created [" + stringulate(Id) + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

NetworkClient::NetworkClient() : Selections(MAX_SELECTION_BOXES) {
    Id = -1;
    CustomExtensions = 0;
    DataAvailable = false;
    canReceive = true;
    canSend = true;
    LastTimeEvent = time(nullptr);
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
}

void NetworkClient::OutputPing() const {
    SendBuffer->Write((unsigned char)1);
}

void NetworkClient::Kick(const std::string& message, bool hide) {
    NetworkFunctions::SystemRedScreen(this->Id, message);

    if (DisconnectTime == 0) {
        DisconnectTime = time(nullptr) + 1;
        LoggedIn = false;
        player->LogoutHide = hide;
        Logger::LogAdd(MODULE_NAME, "Client Kicked [" + message + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
}

void NetworkClient::HoldThis(unsigned char blockType, bool canChange) const {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> selfPointer = nm->GetClient(this->Id);
    if (CPE::GetClientExtVersion(selfPointer, HELDBLOCK_EXT_NAME) != 1) {
        return;
    }
    if (blockType > 49 && CPE::GetClientExtVersion(selfPointer, CUSTOM_BLOCKS_EXT_NAME) <= 0) {
        return;
    }
    Packets::SendHoldThis(selfPointer, blockType, canChange);
    player->tEntity->heldBlock = blockType;
}

void NetworkClient::CreateSelection(unsigned char selectionId, std::string label, short startX, short startY, short startZ, short endX, short endY, short endZ, short red, short green, short blue, short opacity) {
    if (startX > endX || startY > endY || startZ > endZ)
        return;

    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> selfPointer = nm->GetClient(this->Id);

    if (CPE::GetClientExtVersion(selfPointer, SELECTION_CUBOID_EXT_NAME) <= 0)
        return;

    if (Selections[selectionId] > 0)
        return;

    Selections[selectionId] = 1;
    Packets::SendSelectionBoxAdd(selfPointer, selectionId, std::move(label), startX, startY, startZ, endX, endY, endZ, red, green, blue, opacity);
}

void NetworkClient::DeleteSelection(unsigned char selectionId) {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> selfPointer = nm->GetClient(this->Id);

    if (CPE::GetClientExtVersion(selfPointer, SELECTION_CUBOID_EXT_NAME) <= 0)
        return;

    if (Selections[selectionId] <= 0)
        return;

    Selections[selectionId] = 0;
    Packets::SendSelectionBoxDelete(selfPointer, selectionId);
}

void NetworkClient::SetWeather(int weatherType) {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> selfPointer = nm->GetClient(this->Id);

    if (CPE::GetClientExtVersion(selfPointer, EXT_WEATHER_CONTROL_EXT_NAME) <= 0)
        return;
    
    if (weatherType != 0 && weatherType != 1 && weatherType != 2)
        return;

    Packets::SendSetWeather(selfPointer, weatherType);
}

void NetworkClient::SendHackControl(bool canFly, bool noclip, bool speeding, bool spawnControl, bool thirdperson, int jumpHeight) {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> selfPointer = nm->GetClient(this->Id);

    if (CPE::GetClientExtVersion(selfPointer, HACKCONTROL_EXT_NAME) <= 0)
        return;
    
    Packets::SendHackControl(selfPointer, canFly, noclip, speeding, spawnControl, thirdperson, jumpHeight);
}

void NetworkClient::SetBlockPermissions(int blockId, bool canPlace, bool canDelete) {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> selfPointer = nm->GetClient(this->Id);

    if (CPE::GetClientExtVersion(selfPointer, BLOCK_PERMISSIONS_EXT_NAME) <= 0)
        return;

    Packets::SendBlockPermissions(selfPointer, blockId, canPlace, canDelete);
}

void NetworkClient::DataReady() {
    DataAvailable = true;
}

NetworkClient::NetworkClient(NetworkClient &client) : Selections(MAX_SELECTION_BOXES) {
    Id= client.Id;
    SendBuffer = std::make_unique<ByteBuffer>([this]{ this->DataReady(); });//
    ReceiveBuffer = std::move(client.ReceiveBuffer);
    DataAvailable = false;
    canReceive = true;
    canSend = true;
    CustomExtensions = 0;
    LastTimeEvent = time(nullptr);
    clientSocket = std::move(client.clientSocket);
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
    IP = clientSocket->GetSocketIp();
}

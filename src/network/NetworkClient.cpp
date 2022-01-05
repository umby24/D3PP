//
// Created by unknown on 8/24/21.
//

#include "network/NetworkClient.h"
#include "Utils.h"
#include <memory>
#include <utility>
#include <events/EventEntityAdd.h>
#include "common/ByteBuffer.h"
#include "common/Logger.h"
#include "network/Network_Functions.h"
#include "world/Player.h"
#include "network/Network.h"
#include "CPE.h"
#include "network/Packets.h"
#include "world/Entity.h"
#include "common/MinecraftLocation.h"
#include "EventSystem.h"
#include "events/EntityEventArgs.h"
#include "events/EventEntityAdd.h"
#include "events/EventEntityDelete.h"
#include "common/Player_List.h"

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
    PingTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
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
    SubEvents();
    Logger::LogAdd(MODULE_NAME, "Client Created [" + stringulate(Id) + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

NetworkClient::NetworkClient() : Selections(MAX_SELECTION_BOXES) {
    Id = -1;
    CustomExtensions = 0;
    DataAvailable = false;
    canReceive = true;
    canSend = true;
    LastTimeEvent = time(nullptr);
    PingTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
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
    SubEvents();
}

void NetworkClient::SubEvents() {
    eventSubId = Dispatcher::subscribe(EntityEventArgs::moveDescriptor, [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityAdd::descriptor, [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityDelete::descriptor, [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
}

void NetworkClient::HandleEvent(const Event& e) {
    if (!LoggedIn)
        return;

    if (stringulate(e.type()) == ENTITY_EVENT_MOVED) {
        const EntityEventArgs& ea = static_cast<const EntityEventArgs&>(e);
        std::shared_ptr<Entity> eventEntity = Entity::GetPointer(ea.entityId);
        if (eventEntity->Id == player->tEntity->Id && !eventEntity->SendPosOwn)
            return;
        
        if (eventEntity->Id != player->tEntity->Id)
            NetworkFunctions::NetworkOutEntityPosition(Id, eventEntity->ClientId, eventEntity->Location);
        else
            NetworkFunctions::NetworkOutEntityPosition(Id, -1, eventEntity->Location);
    } else if (stringulate(e.type()) == ENTITY_EVENT_SPAWN) {
        const EventEntityAdd& ea = static_cast<const EventEntityAdd&>(e);
        std::shared_ptr<Entity> eventEntity = Entity::GetPointer(ea.entityId);
        if (eventEntity == nullptr) {
            return;
        }
        if (eventEntity->MapID != player->tEntity->MapID)
            return;
        if (eventEntity->Id != player->tEntity->Id)
            NetworkFunctions::NetworkOutEntityAdd(Id, eventEntity->ClientId, Entity::GetDisplayname(ea.entityId), eventEntity->Location);
        else
            NetworkFunctions::NetworkOutEntityAdd(Id, -1, Entity::GetDisplayname(ea.entityId), eventEntity->Location);
    } else if (stringulate(e.type()) == ENTITY_EVENT_DESPAWN) {
        const EventEntityDelete& ea = static_cast<const EventEntityDelete&>(e);
        std::shared_ptr<Entity> eventEntity = Entity::GetPointer(ea.entityId);
        
        if (eventEntity->MapID != player->tEntity->MapID)
            return;

        NetworkFunctions::NetworkOutEntityDelete(Id, eventEntity->ClientId);
    }
}

void NetworkClient::OutputPing() {
    const std::scoped_lock<std::mutex> sLock(sendLock);
    SendBuffer->Write((unsigned char)1);
}

void NetworkClient::Kick(const std::string& message, bool hide) {
    NetworkFunctions::SystemRedScreen(this->Id, message);

    if (DisconnectTime == 0) {
        DisconnectTime = time(nullptr) + 1;
        LoggedIn = false;
        if (player != nullptr) {
            player->LogoutHide = hide;
        }
        Logger::LogAdd(MODULE_NAME, "Client Kicked [" + message + "]", LogType::NORMAL, GLF);
    }
}

void NetworkClient::SpawnEntity(std::shared_ptr<Entity> e) {
    std::shared_ptr<NetworkClient> selfPointer = GetSelfPointer();
    if (e->Id != player->tEntity->Id) {
        EntityShort s{};
        s.Id = e->Id;
        s.ClientId = e->ClientId;
        player->Entities.push_back(s); // -- track the new client
        // -- spawn them :)
        NetworkFunctions::NetworkOutEntityAdd(Id, s.ClientId, Entity::GetDisplayname(s.Id), e->Location);
        CPE::PostEntityActions(selfPointer, e);
    } else if (e->SpawnSelf){
        NetworkFunctions::NetworkOutEntityAdd(Id, -1, Entity::GetDisplayname(e->Id), e->Location);
        CPE::PostEntityActions(selfPointer, e);
        e->SpawnSelf = false;
    }
}

void NetworkClient::DespawnEntity(std::shared_ptr<Entity> e) {
    std::shared_ptr<NetworkClient> selfPointer = GetSelfPointer();

    int iterator = 0;
    for(auto const &vEntity : player->Entities) {
        if (vEntity.Id == e->Id) {
            player->Entities.erase(player->Entities.begin() + iterator);
            break;
        }
        iterator++;
    }

    // -- spawn them :)
    NetworkFunctions::NetworkOutEntityDelete(Id, e->ClientId);
}

void NetworkClient::HoldThis(unsigned char blockType, bool canChange) {
    std::shared_ptr<NetworkClient> selfPointer = GetSelfPointer();
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

    std::shared_ptr<NetworkClient> selfPointer = GetSelfPointer();

    if (CPE::GetClientExtVersion(selfPointer, SELECTION_CUBOID_EXT_NAME) <= 0)
        return;

    if (Selections[selectionId] > 0)
        return;

    Selections[selectionId] = 1;
    Packets::SendSelectionBoxAdd(selfPointer, selectionId, std::move(label), startX, startY, startZ, endX, endY, endZ, red, green, blue, opacity);
}

void NetworkClient::DeleteSelection(unsigned char selectionId) {
    std::shared_ptr<NetworkClient> selfPointer = GetSelfPointer();

    if (CPE::GetClientExtVersion(selfPointer, SELECTION_CUBOID_EXT_NAME) <= 0)
        return;

    if (Selections[selectionId] <= 0)
        return;

    Selections[selectionId] = 0;
    Packets::SendSelectionBoxDelete(selfPointer, selectionId);
}

void NetworkClient::SetWeather(int weatherType) {
    std::shared_ptr<NetworkClient> selfPointer = GetSelfPointer();

    if (CPE::GetClientExtVersion(selfPointer, EXT_WEATHER_CONTROL_EXT_NAME) <= 0)
        return;
    
    if (weatherType != 0 && weatherType != 1 && weatherType != 2)
        return;

    Packets::SendSetWeather(selfPointer, weatherType);
}

void NetworkClient::SendHackControl(bool canFly, bool noclip, bool speeding, bool spawnControl, bool thirdperson, int jumpHeight) {
    std::shared_ptr<NetworkClient> selfPointer = GetSelfPointer();

    if (CPE::GetClientExtVersion(selfPointer, HACKCONTROL_EXT_NAME) <= 0)
        return;
    
    Packets::SendHackControl(selfPointer, canFly, noclip, speeding, spawnControl, thirdperson, jumpHeight);
}

void NetworkClient::SetBlockPermissions(int blockId, bool canPlace, bool canDelete) {
    std::shared_ptr<NetworkClient> selfPointer = GetSelfPointer();

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
    PingTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
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
    SubEvents();
}

NetworkClient::~NetworkClient() {
    SendBuffer = nullptr;
    ReceiveBuffer = nullptr;

    if (clientSocket != nullptr) {
        if (clientSocket->GetConnected()) {
            clientSocket->Disconnect();
        }
        clientSocket = nullptr;
    }

    Dispatcher::unsubscribe(eventSubId);
}

void NetworkClient::SendChat(std::string message) {
    NetworkFunctions::SystemMessageNetworkSend(Id, message);
}

std::shared_ptr<NetworkClient> NetworkClient::GetSelfPointer() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> selfPointer = std::static_pointer_cast<NetworkClient>(nm->GetClient(this->Id));
    return selfPointer;
}

int NetworkClient::GetRank() {
    if (!LoggedIn || !player)
        return 0;

    return player->tEntity->playerList->PRank;
}

bool NetworkClient::IsStopped() {
    if (!LoggedIn || !player)
        return false;

    return player->tEntity->playerList->Stopped;
}

int NetworkClient::GetPing() {
    return Ping;
}

std::string NetworkClient::GetLoginName() {
    return player->LoginName;
}

bool NetworkClient::GetGlobalChat() {
    return GlobalChat;
}

void NetworkClient::SetGlobalChat(bool active) {
    GlobalChat = active;
    player->tEntity->playerList->SetGlobal(active);
}

void NetworkClient::SendDefineBlock(BlockDefinition newBlock) {
    if (CPE::GetClientExtVersion(GetSelfPointer(), BLOCK_DEFS_EXT_NAME) == 0)
        return;

    if (CPE::GetClientExtVersion(GetSelfPointer(), BLOCK_DEFS_EXTENDED_EXT_NAME) != 2)
        Packets::SendDefineBlock(GetSelfPointer(), newBlock);
    else
        Packets::SendDefineBlockExt(GetSelfPointer(), newBlock);
}

void NetworkClient::SendDeleteBlock(unsigned char blockId) {
    if (CPE::GetClientExtVersion(GetSelfPointer(), BLOCK_DEFS_EXT_NAME) == 0)
        return;

    Packets::SendRemoveBlock(GetSelfPointer(), blockId);
}

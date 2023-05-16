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
#include "common/UndoItem.h"

#include "network/Network_Functions.h"
#include "world/Map.h"
#include "world/Player.h"
#include "world/MapMain.h"
#include "network/Network.h"
#include "CPE.h"
#include "network/Packets.h"
#include "network/IPacket.h"
#include "world/Entity.h"
#include "EventSystem.h"
#include "events/EntityEventArgs.h"
#include "events/EventEntityDelete.h"
#include "common/Player_List.h"
#include "network/Server.h"
#include "network/PacketHandlers.h"
#include "Client.h"

#ifndef __linux__
#include "network/WindowsSockets.h"
#else
#include "network/LinuxSockets.h"
#endif

const std::string MODULE_NAME = "NetworkClient";
using namespace D3PP::world;

NetworkClient::NetworkClient(std::unique_ptr<Sockets> socket) : Selections(MAX_SELECTION_BOXES) {
    if (socket->GetSocketFd() == -1) {
        return;
    }

    Id= static_cast<int>(socket->GetSocketFd());
    CustomExtensions = 0;
    m_currentUndoIndex = 0;
    SendBuffer = std::make_shared<ByteBuffer>([this]{this->DataReady();});
    ReceiveBuffer = std::make_shared<ByteBuffer>(nullptr);
    DataAvailable = false;
    canReceive = true;
    canSend = true;
    LastTimeEvent = time(nullptr);
    clientSocket = std::move(socket);
    PingTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    DisconnectTime = 0;
    LoggedIn = false;
    CPE = false;
    PingSentTime = PingTime;
    Ping = 0;
    CustomBlocksLevel = 0;
    GlobalChat = false;
    IP = clientSocket->GetSocketIp();
    PingVal = 0;
    eventSubId = 0;
    addSubId = 0;
    removeSubId = 0;
    SubEvents();
    Logger::LogAdd(MODULE_NAME, "Client Created [" + stringulate(Id) + "]", LogType::NORMAL, GLF);
}

NetworkClient::NetworkClient() : Selections(MAX_SELECTION_BOXES) {
    Id = -1;
    CustomExtensions = 0;
    m_currentUndoIndex = 0;
    DataAvailable = false;
    canReceive = true;
    canSend = true;
    LastTimeEvent = time(nullptr);
    PingTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    DisconnectTime = 0;
    LoggedIn = false;
    CPE = false;
    PingSentTime = PingTime;
    Ping = 0;
    CustomBlocksLevel = 0;
    GlobalChat = false;
    PingVal = 0;
    eventSubId = 0;
    addSubId = 0;
    removeSubId = 0;
    SubEvents();
}

void NetworkClient::MainFunc() {
    if (DisconnectTime > 0 && DisconnectTime < time(nullptr)) {
        Shutdown("Forced Disconnect");
        return;
    }
    if (LastTimeEvent + NETWORK_CLIENT_TIMEOUT < time(nullptr)) {
        Kick("Timeout", false);
        return;
    }

    if (PingTime >= std::chrono::steady_clock::now()) return;

    PingTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    PingSentTime = std::chrono::steady_clock::now();
    PingVal = static_cast<short>(clock());

    if (CPE::GetClientExtVersion(GetSelfPointer(), TWOWAY_PING_EXT_NAME) == 0)
        OutputPing();
    else
        Packets::SendTwoWayPing(GetSelfPointer(), 1, PingVal);
}

void NetworkClient::SubEvents() {
    eventSubId = Dispatcher::subscribe(EntityEventArgs::moveDescriptor, [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    addSubId = Dispatcher::subscribe(EventEntityAdd{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    removeSubId = Dispatcher::subscribe(EventEntityDelete{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
}

void NetworkClient::HandleEvent(const Event& e) {
    if (!LoggedIn)
        return;

    if (stringulate(e.type()) == ENTITY_EVENT_MOVED) {
        const auto& ea = static_cast<const EntityEventArgs&>(e);
        std::shared_ptr<Entity> eventEntity = Entity::GetPointer(ea.entityId);
        if (eventEntity->Id == player->GetEntity()->Id && !eventEntity->SendPosOwn)
            return;

        if (eventEntity->MapID != player->GetEntity()->MapID)
            return;
        if (eventEntity->Id != player->GetEntity()->Id)
            NetworkFunctions::NetworkOutEntityPosition(Id, eventEntity->ClientId, eventEntity->Location);
        else
            NetworkFunctions::NetworkOutEntityPosition(Id, -1, eventEntity->Location);
    } else if (stringulate(e.type()) == ENTITY_EVENT_SPAWN) {
        const auto& ea = static_cast<const EventEntityAdd&>(e);
        std::shared_ptr<Entity> eventEntity = Entity::GetPointer(ea.entityId);
        if (eventEntity == nullptr) {
            return;
        }
        if (eventEntity->MapID != player->GetEntity()->MapID)
            return;
        std::shared_ptr<NetworkClient> selfPoint = GetSelfPointer();
        if (eventEntity->Id != player->GetEntity()->Id) {
            NetworkFunctions::NetworkOutEntityAdd(Id, eventEntity->ClientId, Entity::GetDisplayname(ea.entityId),
                                                  eventEntity->Location);
            EntityShort s{};
            s.Id = eventEntity->Id;
            s.ClientId = eventEntity->ClientId;
            if (eventEntity->model != "" && eventEntity->model != "humanoid" && CPE::GetClientExtVersion(selfPoint, CHANGE_MODEL_EXT_NAME) > 0) {
                Packets::SendChangeModel(selfPoint, eventEntity->ClientId, eventEntity->model);
            }
        } else {
            NetworkFunctions::NetworkOutEntityAdd(Id, -1, Entity::GetDisplayname(ea.entityId), eventEntity->Location);
            if (eventEntity->model != "" && eventEntity->model != "humanoid" && CPE::GetClientExtVersion(selfPoint, CHANGE_MODEL_EXT_NAME) > 0) {
                Packets::SendChangeModel(selfPoint, -1, eventEntity->model);
            }
        }
    } else if (stringulate(e.type()) == ENTITY_EVENT_DESPAWN) {
        const EventEntityDelete& ea = static_cast<const EventEntityDelete&>(e);
        std::shared_ptr<Entity> eventEntity = Entity::GetPointer(ea.entityId);

        if (LoggedIn && this->player != nullptr && this->player->GetEntity() != nullptr) {
            if (eventEntity->MapID != player->GetEntity()->MapID)
                return;

            NetworkFunctions::NetworkOutEntityDelete(Id, eventEntity->ClientId);
        }
    }
}

void NetworkClient::OutputPing() {
    const std::scoped_lock<std::mutex> sLock(sendLock);
    SendBuffer->Write((unsigned char)1);
}

void NetworkClient::Kick(const std::string& message, bool hide) {
    canReceive = false;
    NetworkFunctions::SystemRedScreen(this->Id, message);

    if (DisconnectTime == 0) {
        DisconnectTime = time(nullptr) + 1;
        LoggedIn = false;
        Logger::LogAdd(MODULE_NAME, "Client Kicked [" + message + "]", LogType::NORMAL, GLF);
    }
}

void NetworkClient::SpawnEntity(std::shared_ptr<Entity> e) {
    std::shared_ptr<NetworkClient> selfPointer = GetSelfPointer();
    if (e->Id != player->GetEntity()->Id) {
        EntityShort s{};
        s.Id = e->Id;
        s.ClientId = e->ClientId;
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
    player->GetEntity()->heldBlock = blockType;
}

void NetworkClient::CreateSelection(unsigned char selectionId, std::string label, D3PP::Common::Vector3S start, D3PP::Common::Vector3S end, D3PP::Common::Vector3S color, short opacity) {
    if (start.X > end.X || start.Y > end.Y || start.Z > end.Z)
        return;

    std::shared_ptr<NetworkClient> selfPointer = GetSelfPointer();

    if (CPE::GetClientExtVersion(selfPointer, SELECTION_CUBOID_EXT_NAME) <= 0)
        return;

    if (Selections[selectionId] > 0)
        return;

    Selections[selectionId] = 1;
    Packets::SendSelectionBoxAdd(selfPointer, selectionId, std::move(label), start.X, start.Y, start.Z, end.X, end.Y, end.Z, color.X, color.Y, color.Z, opacity);
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

    Packets::SendHackControl(selfPointer, canFly, noclip, speeding, spawnControl, thirdperson, static_cast<short>(jumpHeight));
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
    SendBuffer = std::make_shared<ByteBuffer>([this]{ this->DataReady(); });//
    ReceiveBuffer = std::move(client.ReceiveBuffer);
    DataAvailable = false;
    canReceive = true;
    m_currentUndoIndex = 0;
    canSend = true;
    CustomExtensions = 0;
    LastTimeEvent = time(nullptr);
    clientSocket = std::move(client.clientSocket);
    PingTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    DisconnectTime = 0;
    LoggedIn = false;
    CPE = false;
    PingSentTime = PingTime;
    Ping = 0;
    CustomBlocksLevel = 0;
    GlobalChat = false;
    IP = clientSocket->GetSocketIp();
    PingVal = 0;
    eventSubId = 0;
    addSubId = 0;
    removeSubId = 0;
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
    Dispatcher::unsubscribe(addSubId);
    Dispatcher::unsubscribe(removeSubId);
}

void NetworkClient::SendChat(std::string message) {
    NetworkFunctions::SystemMessageNetworkSend(Id, message);
}

std::shared_ptr<NetworkClient> NetworkClient::GetSelfPointer() const {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> selfPointer = std::static_pointer_cast<NetworkClient>(nm->GetClient(this->Id));
    return selfPointer;
}

int NetworkClient::GetRank() {
    if (!LoggedIn || !player)
        return 0;

    auto *pll = Player_List::GetInstance();
    auto pli = pll->GetPointer(GetLoginName());

    return pli->PRank;
}

bool NetworkClient::IsStopped() {
    if (!LoggedIn || !player)
        return false;

    auto *pll = Player_List::GetInstance();
    auto pli = pll->GetPointer(GetLoginName());

    return pli->Stopped;
}

int NetworkClient::GetPing() {
    return Ping;
}

std::string NetworkClient::GetLoginName() {
    if (player != nullptr) {
        return player->GetLoginName();
    }
    return "--";
}

bool NetworkClient::GetGlobalChat() {
    return GlobalChat;
}

int NetworkClient::GetMapId() {
    if (!LoggedIn || !player || !player->GetEntity())
        return -1;

    return player->GetEntity()->MapID;
}

void NetworkClient::SetGlobalChat(bool active) {
    GlobalChat = active;
    player->GetEntity()->playerList->SetGlobal(active);
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

bool NetworkClient::IsDataAvailable() {
    return DataAvailable;
}

void NetworkClient::SendQueued() {
    const std::scoped_lock<std::mutex> sLock(sendLock);
    int sendSize = SendBuffer->Size();
    std::vector<unsigned char> allBytes = SendBuffer->GetAllBytes();

    int bytesSent = clientSocket->Send(reinterpret_cast<char *>(allBytes.data()), sendSize);

    DataAvailable = false;
    D3PP::network::Server::SentIncrement += bytesSent;
}

bool NetworkClient::ReadData() {
    if (canReceive) {
        auto *receiveBuf = new char[1026];
        receiveBuf[1024] = 99;
        int dataRead = clientSocket->Read(receiveBuf, 1024);
        LastTimeEvent = time(nullptr);
        DataWaiting = false;

        if (dataRead > 0) {
            std::vector<unsigned char> receive(receiveBuf, receiveBuf + dataRead);
            ReceiveBuffer->Write(receive, dataRead);
            D3PP::network::Server::ReceivedIncrement += dataRead;
            delete[] receiveBuf;
            return true;
        } else {
            delete[] receiveBuf;
            D3PP::network::Server::UnregisterClient(GetSelfPointer());
            Shutdown("Connection lost");
            return false;
        }
    }
}
void NetworkClient::HandleData() {
    if (DataWaiting) {
        if (!ReadData())
            return;
    }

    int maxRepeat = 10;
    while (ReceiveBuffer->Size() > 0 && maxRepeat > 0 && canReceive) {
        unsigned char commandByte = ReceiveBuffer->PeekByte();
        LastTimeEvent = time(nullptr);

        if (!LoggedIn) {
            bool isAllowedPacket = (commandByte == 0) || (commandByte == 1) || commandByte == 16 || commandByte == 17 || commandByte == 19;
            if (CPE) {
                isAllowedPacket = isAllowedPacket || (commandByte == 43);
            }
            if (!isAllowedPacket) {
                Logger::LogAdd(MODULE_NAME, "Disconnecting " + this->IP + ": Unexpected handshake opcode.", WARNING, GLF);
                Kick("Invalid Packet", true);
                return;
            }
        }

        switch(commandByte) {
            case 0: // -- Login
                if (ReceiveBuffer->Size() >= 1 + 1 + 64 + 64 + 1) {
                    ReceiveBuffer->ReadByte();
                    PacketHandlers::HandleHandshake(GetSelfPointer());
                    ReceiveBuffer->Shift(1 + 1 + 64 + 64 + 1);
                }
                break;
            case 1: // -- Ping
                if (ReceiveBuffer->Size() >= 1) {
                    ReceiveBuffer->ReadByte();
                    PacketHandlers::HandlePing(GetSelfPointer());
                    ReceiveBuffer->Shift(1);
                }
                break;
            case 5: // -- Block Change
                if (ReceiveBuffer->Size() >= 9) {
                    ReceiveBuffer->ReadByte();
                    PacketHandlers::HandleBlockChange(GetSelfPointer());
                    ReceiveBuffer->Shift(9);
                }
                break;
            case 8: // -- Player Movement
                if (ReceiveBuffer->Size() >= 10) {
                    ReceiveBuffer->ReadByte();
                    PacketHandlers::HandlePlayerTeleport(GetSelfPointer());
                    ReceiveBuffer->Shift(10);
                }
                break;
            case 13: // -- Chat Message
                if (ReceiveBuffer->Size() >= 66) {
                    ReceiveBuffer->ReadByte();
                    PacketHandlers::HandleChatPacket(GetSelfPointer());
                    ReceiveBuffer->Shift(66);
                }
                break;
            case 16: // -- CPe ExtInfo
                if (ReceiveBuffer->Size() >= 67) {
                    ReceiveBuffer->ReadByte();
                    PacketHandlers::HandleExtInfo(GetSelfPointer());
                    ReceiveBuffer->Shift(67);
                }
                break;
            case 17: // -- CPE ExtEntry
                if (ReceiveBuffer->Size() >= 1 + 64 + 4) {
                    ReceiveBuffer->ReadByte();
                    PacketHandlers::HandleExtEntry(GetSelfPointer());
                    ReceiveBuffer->Shift(69);
                }
                break;
            case 19: // -- CPE Custom Block Support
                if (ReceiveBuffer->Size() >= 2) {
                    ReceiveBuffer->ReadByte();
                    PacketHandlers::HandleCustomBlockSupportLevel(GetSelfPointer());
                    ReceiveBuffer->Shift(2);
                }
                break;
            case 34: // -- CPE Player Clicked.
                if (ReceiveBuffer->Size() >= 15) {
                    ReceiveBuffer->ReadByte();
                    PacketHandlers::HandlePlayerClicked(GetSelfPointer());
                    ReceiveBuffer->Shift(15);
                }
                break;
            case 43:
                if (ReceiveBuffer->Size() >= 4) {
                    ReceiveBuffer->ReadByte();
                    PacketHandlers::HandleTwoWayPing(GetSelfPointer());
                    ReceiveBuffer->Shift(4);
                }
                break;

            default:
                Logger::LogAdd(MODULE_NAME, "Unknown Packet Received [" + stringulate((int)commandByte) + "]", LogType::WARNING, GLF);
                Kick("Invalid Packet", true);
        }

        maxRepeat--;
    } // -- /While
}

void NetworkClient::SendPacket(D3PP::network::IPacket &p) {
    const std::scoped_lock sLock(sendLock);
    p.Write(SendBuffer);
}

void NetworkClient::Undo(int steps) { 
    if (m_undoItems.empty())
        return;

    if (steps-1 > m_currentUndoIndex)
        steps = m_currentUndoIndex+1;

    if (m_currentUndoIndex == -1)
        return;

    MapMain* mm = MapMain::GetInstance();
    int currentMapId = GetMapId();
    std::shared_ptr<Map> currentMap = mm->GetPointer(currentMapId);
    
    for(int i = m_currentUndoIndex; i > (m_currentUndoIndex - steps); i--)
        currentMap->BlockChange(player->GetEntity()->playerList->Number, m_undoItems[i].Location.X, m_undoItems[i].Location.Y,m_undoItems[i].Location.Z, m_undoItems[i].OldBlock, false, false, true, 100);

    m_currentUndoIndex -= (steps - 1);
}

void NetworkClient::Redo(int steps) {
    if (m_undoItems.empty())
        return;

    if (steps > (m_undoItems.size() - m_currentUndoIndex))
        steps = (m_undoItems.size() - m_currentUndoIndex);

    if (m_currentUndoIndex == m_undoItems.size()-1)
        return;

    if (m_currentUndoIndex == -1)
        m_currentUndoIndex = 0;

    MapMain* mm = MapMain::GetInstance();
    int currentMapId = GetMapId();
    std::shared_ptr<Map> currentMap = mm->GetPointer(currentMapId);
    
    for(int i = m_currentUndoIndex; i < (m_currentUndoIndex + steps); i++)
        currentMap->BlockChange(player->GetEntity()->playerList->Number, m_undoItems[i].Location.X, m_undoItems[i].Location.Y,m_undoItems[i].Location.Z, m_undoItems[i].NewBlock, false, false, true, 100);

    m_currentUndoIndex += (steps - 1);
}

void NetworkClient::AddUndoItem(const D3PP::Common::UndoItem &item) {
    if (m_currentUndoIndex == -1)
        m_currentUndoIndex = 0;

    if (m_currentUndoIndex != (m_undoItems.size()-1)) {
        // -- Remove everything forward of this.
        m_undoItems.erase(m_undoItems.end()-m_currentUndoIndex, m_undoItems.end());
    }

    if (m_undoItems.size() >= 50000) {
        // -- Remove one item off the top.
        m_undoItems.erase(m_undoItems.begin());
    }
    m_undoItems.push_back(item);
    m_currentUndoIndex = m_undoItems.size() - 1;
}

void NetworkClient::NotifyDataAvailable() {
    DataWaiting = true;
}

void NetworkClient::Shutdown(const std::string& reason) {
    Client::Logout(Id, reason, true);
    canSend = false;
    canReceive = false;
    D3PP::network::Server::UnregisterClient(GetSelfPointer());
    Logger::LogAdd(MODULE_NAME, "Client deleted [" + stringulate(Id) + "] [" + reason + "]", LogType::NORMAL, GLF);
    clientSocket->Disconnect();
    D3PP::network::Server::UnregisterClient(GetSelfPointer());
}

bool NetworkClient::GetLoggedIn() {
    return this->LoggedIn;
}

std::shared_ptr<D3PP::world::IMinecraftPlayer> NetworkClient::GetPlayerInstance() {
    return this->player;
}

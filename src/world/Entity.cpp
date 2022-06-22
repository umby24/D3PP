//
// Created by Wande on 3/17/2021.
//

#include "world/Entity.h"

#include <utility>
#include "Block.h"

#include "network/Network.h"
#include "network/NetworkClient.h"
#include "world/Player.h"
#include "common/Player_List.h"

#include "world/Teleporter.h"
#include "world/Map.h"
#include "world/MapMain.h"
#include "network/Network_Functions.h"
#include "common/Logger.h"
#include "common/Vectors.h"
#include "Utils.h"
#include "CPE.h"
#include "network/Packets.h"
#include "EventSystem.h"
#include "events/EventEntityAdd.h"
#include "events/EventEntityDelete.h"
#include "events/EventEntityDie.h"
#include "events/EventEntityPositionSet.h"
#include "events/EventEntityMapChange.h"
#include "events/EntityEventArgs.h"

const std::string MODULE_NAME = "Entity";
std::map<int, std::shared_ptr<Entity>> Entity::AllEntities;
std::mutex Entity::entityMutex{};

using namespace D3PP::Common;
using namespace D3PP::world;

EntityMain::EntityMain() {
}

void EntityMain::MainFunc() {

}

int Entity::GetFreeId() {
    int id = 0;
    bool found;
    while (true) {
        found = (AllEntities.find(id) != AllEntities.end());

        if (found)
            id++;
        else {
            return id;
        }
    }

}

void Entity::SetDisplayName(int id, std::string prefix, std::string name, std::string suffix) {
    std::shared_ptr<Entity> e = GetPointer(id);
    if (e == nullptr)
        return;

    e->Prefix = std::move(prefix);
    e->Name = std::move(name);
    e->Suffix = std::move(suffix);
    e->resend = true;
}

int Entity::GetFreeIdClient(int mapId) {
    int id;
    bool found = false;
    for (id = 0; id < 128; id++) {
        for(auto const &e : AllEntities) {
            if (e.second->ClientId == id && e.second->MapID == mapId) {
                found = true;
                continue;
            }
        }

        if (!found) {
            return id;
        } else {
            found = false; // -- try again
        }
    }

    Logger::LogAdd(MODULE_NAME, "No free map clientID", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__ );
    return -1;
}


Entity::Entity(std::string name, int mapId, float X, float Y, float Z, float rotation, float look) : variables{}, Location{rotation, look} {
    Prefix = "";
    Name = std::move(name);
    Suffix = "";
    Id = GetFreeId();
    ClientId = GetFreeIdClient(mapId);
    MapID = mapId;
    resend = false;
    SendPosOwn = true;
    SendPos = false;
    timeMessageDeath = 0;
    timeMessageOther = 0;
    heldBlock = 0;
    lastMaterial = 0;
    buildMaterial = -1;
    SpawnSelf = false;
    BuildState = 0;
    BuildMode = "Normal";
    playerList = nullptr;
    Vector3F locAsBlocks {X, Y, Z};
    Location.SetAsPlayerCoords(locAsBlocks);
    associatedClient = nullptr;
}

Entity::Entity(std::string name, int mapId, MinecraftLocation loc, std::shared_ptr<NetworkClient> c) : variables{}, Location(loc) {
    Prefix = "";
    Name = name;
    Suffix = "";
    Id = GetFreeId();
    ClientId = GetFreeIdClient(mapId);
    MapID = mapId;
    resend = false;
    SendPosOwn = true;
    SendPos = false;
    timeMessageDeath = 0;
    timeMessageOther = 0;
    heldBlock = 0;
    lastMaterial = 0;
    buildMaterial = -1;
    SpawnSelf = false;
    BuildState = 0;
    BuildMode = "Normal";
    playerList = nullptr;
    associatedClient = c;
}

std::shared_ptr<Entity> Entity::GetPointer(int id, bool isClientId) {
    if (!isClientId) {
        if (AllEntities.find(id) == AllEntities.end())
            return nullptr;

        return AllEntities.at(id);
    }

    std::shared_ptr<Entity> result = nullptr;

    for (auto const &e : AllEntities) {
        if (e.second == nullptr || e.second->associatedClient == nullptr)
            continue;
            
        if (e.second->associatedClient->GetId() == id) {
            result = e.second;
            break;
        }
    }

    return result;
}

void Entity::MessageToClients(int id, const std::string& message) {
    Network* n = Network::GetInstance();

    for(auto const &nc : n->roClients) {
        if (nc->player->tEntity->Id == id) {
            NetworkFunctions::SystemMessageNetworkSend(nc->GetId(), message);
        }
    }
}

std::string Entity::GetDisplayname(int id) {
    std::shared_ptr<Entity> e = GetPointer(id);

    if (e == nullptr)
        return "";

    return e->Prefix + e->Name + e->Suffix;
}

std::shared_ptr<Entity> Entity::GetPointer(const std::string& name) {
    for(auto const &e : AllEntities) {
        if (Utils::InsensitiveCompare(e.second->Name, name)) {
            return e.second;
        }
    }

    return nullptr;
}

void Entity::Delete(int id) {
    std::shared_ptr<Entity> e = GetPointer(id);
    if (e == nullptr)
        return;
    Network* n = Network::GetInstance();

    for(auto const &nc : n->roClients) {
        if (nc->player == nullptr || nc->player->tEntity == nullptr)
            continue;
            
        if (nc->player->tEntity == e) {
            nc->player->tEntity = nullptr;
        }
    }

    EventEntityDelete ed;
    ed.entityId = id;
    Dispatcher::post(ed);

    AllEntities.erase(id);
}

void Entity::Spawn() {
    // -- Entity::Add(); should be called..
    Network* n = Network::GetInstance();
    std::shared_ptr<Entity> selfPointer = GetPointer(Id);

    for(auto const &nc : n->roClients) {
        if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
            continue;

        if (nc->player->MapId != MapID)
            continue;

        nc->SpawnEntity(selfPointer);
    }
}

void Entity::Despawn() {
    Network* n = Network::GetInstance();
    std::shared_ptr<Entity> selfPointer = GetPointer(Id);

    for(auto const &nc : n->roClients) {
        if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
            continue;

        if (nc->player->MapId != MapID)
            continue;

        nc->DespawnEntity(selfPointer);
    }
}

void Entity::Kill() {
    MapMain *mm = MapMain::GetInstance();
    std::shared_ptr<Map> cm = mm->GetPointer(MapID);

    if (timeMessageDeath < time(nullptr)) {
        EventEntityDie ed;
        ed.entityId = this->Id;
        Dispatcher::post(ed);
        if (ed.isCancelled())
            return;

        timeMessageDeath = time(nullptr) + 2;
        

        NetworkFunctions::SystemMessageNetworkSend2All(MapID, "&c" + Name + " died.");

        MinecraftLocation spawnLoc = cm->GetSpawn();
        PositionSet(MapID, spawnLoc, 5, true);
    }
}

void Entity::HandleMove() {
    EntityEventArgs moveEvent(&EntityEventArgs::moveDescriptor);
    moveEvent.entityId = Id;
    Dispatcher::post(moveEvent);

    SendPosOwn = false;
}

void Entity::PositionSet(int mapId, MinecraftLocation location, unsigned char priority, bool sendOwn) {
    if (location == Location) {
        return;
    }
    if (mapId < 0) {
        return;
    }
    MapMain* mm = MapMain::GetInstance();
    
    if (mapId != MapID && associatedClient != nullptr) {
        associatedClient->player->ChangeMap(mm->GetPointer(mapId));
        return;
    }

    


    std::shared_ptr<Map> currentMap = mm->GetPointer(MapID);

    EventEntityPositionSet eps;
    eps.entityId = Id;
    eps.mapId = mapId;
    eps.x = location.X() / 32.0f;
    eps.y = location.Y() / 32.0f;
    eps.z = (location.Z() -51.0f) / 32.0f;
    eps.rotation = location.Rotation;
    eps.look = location.Look;
    eps.priority = priority;
    eps.sendOwnClient = sendOwn;
    Dispatcher::post(eps);

    if (eps.isCancelled()) {
        SendPosOwn = true;
        HandleMove();
        return;
    }

    Location = location;
    PositionCheck();

    if (currentMap != nullptr) {
        if (sendOwn || !SendPosOwn) {
            SendPos = static_cast<char>(priority);
            if (sendOwn)
                SendPosOwn = true;
        }
    }

    HandleMove();
}

void Entity::PositionCheck() {
    MapMain* mm = MapMain::GetInstance();
    Block* bm = Block::GetInstance();
    int mapId = MapID;
    Vector3S blockLocation = Location.GetAsBlockCoords();
    std::shared_ptr<Map> theMap = mm->GetPointer(mapId);
    
    if (theMap == nullptr) {
        return;
    }
    // -- do a teleporter check..
//    for(auto const &tp : theMap->Portals) {
//        D3PP::Common::Vector3S startLoc = tp.OriginStart.GetAsBlockCoords();
//        D3PP::Common::Vector3S endLoc = tp.OriginEnd.GetAsBlockCoords();
//
//        if (blockLocation.X >= startLoc.X && blockLocation.X <= endLoc.X && blockLocation.Y >= startLoc.Y && blockLocation.Y<= endLoc.Y && blockLocation.Z>= startLoc.Z && blockLocation.Z <= endLoc.Z) {
//            int destMapId = MapID;
//
//            if (!tp.DestinationMap.empty()) {
//                std::shared_ptr<Map> mapInstance = mm->GetPointerUniqueId(tp.second.DestMapUniqueId);
//                if (mapInstance != nullptr) {
//                    destMapId = mapInstance->data.ID;
//                }
//            } else if (tp.second.DestMapId != -1) {
//                destMapId = tp.second.DestMapId;
//            }
//
//            PositionSet(destMapId, tp.Destination, 10, true);
//            break;
//        }
//    }

    // -- check if the block we're touching is a killing block, if so call kill.
    for (int i = 0; i < 2; i++) {
        unsigned char blockType = theMap->GetBlockType(blockLocation.X, blockLocation.Y, blockLocation.Z+i);
        if (bm->GetBlock(blockType).Kills)
            Kill();
    }
}

void Entity::Add(const std::shared_ptr<Entity> &e) {
    if (e == nullptr) {
        return;
    }

    {
        std::scoped_lock<std::mutex> pLock(entityMutex);
        auto myPair = std::make_pair(e->Id, e);
        AllEntities.insert(myPair);
    }
    EventEntityAdd ea;
    ea.entityId = e->Id;
    Dispatcher::post(ea);
}

void Entity::SetModel(std::string modelName) {
    Network* nm = Network::GetInstance();

    model = std::move(modelName);
    std::shared_ptr<NetworkClient> myClient = nullptr;
    for(auto const &nc : nm->roClients) {
        if (!nc->LoggedIn)
            continue;

        if (CPE::GetClientExtVersion(nc, CHANGE_MODEL_EXT_NAME) <= 0 || nc->player->MapId != MapID)
            continue;

        if (nc->player->tEntity->Id == Id) {
            myClient = nc;
            continue;
        }
        Packets::SendChangeModel(nc, this->ClientId, model);
    }
    if (myClient != nullptr) {
        Packets::SendChangeModel(myClient, -1, model);
    }
}

void Entity::Resend(int id) {
    EventEntityDelete ed;
    ed.entityId = id;
    Dispatcher::post(ed);

    EventEntityAdd ea;
    ea.entityId = id;
    Dispatcher::post(ea);
}

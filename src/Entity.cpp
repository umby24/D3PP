//
// Created by Wande on 3/17/2021.
//

#include "Entity.h"

#include "Block.h"

#include "Network.h"
#include "NetworkClient.h"
#include "Player.h"
#include "Player_List.h"

#include "Map.h"
#include "Network_Functions.h"
#include "Logger.h"
#include "Utils.h"
#include "CPE.h"
#include "Packets.h"
#include "EventSystem.h"
#include "events/EventEntityAdd.h"
#include "events/EventEntityDelete.h"
#include "events/EventEntityDie.h"
#include "events/EventEntityPositionSet.h"
#include "events/EventEntityMapChange.h"

const std::string MODULE_NAME = "Entity";
std::map<int, std::shared_ptr<Entity>> Entity::_entities;

EntityMain::EntityMain() {
    this->Interval = std::chrono::milliseconds(100);
    this->Main = [this] { MainFunc(); };
    TaskScheduler::RegisterTask("Entity", *this);
}

void EntityMain::MainFunc() {
    for(auto const &e : Entity::_entities) {
        e.second->PositionCheck();
    }

    Entity::Send();
}

int Entity::GetFreeId() {
    int id = 0;
    bool found = false;
    while (true) {
        found = (_entities.find(id) != _entities.end());

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

    e->Prefix = prefix;
    e->Name = name;
    e->Suffix = suffix;
    e->resend = true;
}

int Entity::GetFreeIdClient(int mapId) {
    int id = 0;
    bool found = false;
    for (id = 0; id < 128; id++) {
        for(auto const &e : _entities) {
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
    Name = name;
    Suffix = "";
    Id = GetFreeId();
    ClientId = GetFreeIdClient(mapId);
    MapID = mapId;
    resend = false;
    SendPosOwn = false;
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
    Vector3S locAsBlocks {X*32, Y*32, Z*32-51};
    Location.SetAsPlayerCoords(locAsBlocks);
}

std::shared_ptr<Entity> Entity::GetPointer(int id) {
    if (_entities.find(id) == _entities.end())
        return nullptr;

    return _entities.at(id);
}

void Entity::MessageToClients(int id, const std::string& message) {
    Network* n = Network::GetInstance();

    for(auto const &nc : n->roClients) {
        if (nc->player->tEntity->Id == id) {
            NetworkFunctions::SystemMessageNetworkSend(nc->Id, message);
        }
    }
}

std::string Entity::GetDisplayname(int id) {
    std::shared_ptr<Entity> e = GetPointer(id);

    if (e == nullptr)
        return "";

    return e->Prefix + e->Name + e->Suffix;
}

std::shared_ptr<Entity> Entity::GetPointer(std::string name) {
    for(auto const &e : _entities) {
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

    _entities.erase(id);
}

void Entity::Kill() {
    MapMain *mm = MapMain::GetInstance();
    std::shared_ptr<Map> cm = mm->GetPointer(MapID);

    if (timeMessageDeath < time(nullptr)) {
        timeMessageDeath = time(nullptr) + 2;
        
        EventEntityDie ed;
        ed.entityId = this->Id;
        Dispatcher::post(ed);

        NetworkFunctions::SystemMessageNetworkSend2All(MapID, "&c" + Name + " died.");
        MinecraftLocation spawnLoc {cm->data.SpawnRot, cm->data.SpawnLook, cm->data.SpawnX, cm->data.SpawnY, cm->data.SpawnZ};
        PositionSet(MapID, spawnLoc, 5, true);
    }
}

std::shared_ptr<NetworkClient> getEntityClient(int entityId) {
    Network* n = Network::GetInstance();

    for(auto const &nc : n->roClients) {
        if (nc->player->tEntity->Id == entityId) {
            return nc;
        }
    }

    return nullptr;
}

void Entity::PositionSet(int mapId, MinecraftLocation location, unsigned char priority, bool sendOwn) {
    if (location == Location) {
        return;
    }

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> currentMap = mm->GetPointer(MapID);
    // -- Do entity position check here? TODO:
    if (SendPos <= priority) {
        Location = location;

        EventEntityPositionSet eps;
        eps.entityId = Id;
        eps.mapId = mapId;
        eps.x = Location.X() / 32.0;
        eps.y = Location.Y() / 32.0;
        eps.z = (Location.Z() +51) / 32.0;
        eps.rotation = Location.Rotation;
        eps.look = Location.Look;
        eps.priority = priority;
        eps.sendOwnClient = sendOwn;
        Dispatcher::post(eps);

        if (mapId != MapID) { // -- Changing map
            std::shared_ptr<Map> nm = mm->GetPointer(mapId);
            if (playerList == nullptr || playerList->PRank >= nm->data.RankJoin) {
                std::string entityName = GetDisplayname(Id);
                std::string mapChangeMessage = "&ePlayer '" + entityName + "&e' changed to map '" + nm->data.Name + "'";
                NetworkFunctions::SystemMessageNetworkSend2All(MapID, mapChangeMessage);
                NetworkFunctions::SystemMessageNetworkSend2All(mapId, mapChangeMessage);
                nm->data.Clients += 1;
                int oldMapId = MapID;
                
                if (currentMap != nullptr)
                    currentMap->data.Clients -= 1;

                MapID = mapId;
                ClientId = GetFreeIdClient(mapId);

                EventEntityMapChange emc;
                emc.entityId = Id;
                emc.oldMapId = oldMapId;
                emc.newMapId = MapID;
                Dispatcher::post(emc);
                auto myClient = getEntityClient(Id);
                myClient->player->MapId = MapID;
                myClient->player->SendMap();

                if (sendOwn)
                    SendPosOwn = true;
            } else {
                MessageToClients(Id, "&eYou are not allowed to join map '" + nm->data.Name + "'");
            }
        } else {
            if (currentMap != nullptr) {
                if (sendOwn || !SendPosOwn) {
                    SendPos = priority;
                    if (sendOwn)
                        SendPosOwn = true;
                }
            }
        }
    }
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
    for(auto const &tp : theMap->data.Teleporter) {
        if (blockLocation.X >= tp.second.X0 && blockLocation.X <= tp.second.X1 && blockLocation.Y >= tp.second.Y0 && blockLocation.Y<= tp.second.Y1 && blockLocation.Z>= tp.second.Z0 && blockLocation.Z <= tp.second.Z1) {
            int destMapId = MapID;
            
            if (!tp.second.DestMapUniqueId.empty()) {
                std::shared_ptr<Map> mapInstance = mm->GetPointerUniqueId(tp.second.DestMapUniqueId);
                if (mapInstance != nullptr) {
                    destMapId = mapInstance->data.ID;
                }
            } else if (tp.second.DestMapId != -1) {
                destMapId = tp.second.DestMapId;
            }
            MinecraftLocation tpDest {tp.second.DestRot, tp.second.DestLook, tp.second.DestX, tp.second.DestY, tp.second.DestZ};

            PositionSet(destMapId, tpDest, 10, true);
            break;
        }
    }

    // -- check if the block we're touching is a killing block, if so call kill.
    for (int i = 0; i < 2; i++) {
        unsigned char blockType = theMap->GetBlockType(blockLocation.X, blockLocation.Y, blockLocation.Z+i);
        if (bm->GetBlock(blockType).Kills)
            Kill();
    }
}

void Entity::Send() {
    Network *n = Network::GetInstance();
    for(auto const &nc : n->roClients) {
        if (!nc->LoggedIn)
            continue;

        std::vector<int> toRemove;
        for(auto const &vEntity : nc->player->Entities) {
            std::shared_ptr<Entity> fullEntity = GetPointer(vEntity.Id);
            if (fullEntity == nullptr) { // -- Entity no longer exists, despawn them.
                NetworkFunctions::NetworkOutEntityDelete(nc->Id, vEntity.ClientId);
                toRemove.push_back(vEntity.Id);
                continue;
            }
            bool shouldDelete = false;
            if (fullEntity->MapID != nc->player->MapId)
                shouldDelete = true;
            if (nc->player->tEntity && nc->player->tEntity->Id == vEntity.Id)
                shouldDelete = true;
            if (fullEntity->resend) {
                shouldDelete = true;
                fullEntity->resend = false;
            }

            if (shouldDelete) {
                NetworkFunctions::NetworkOutEntityDelete(nc->Id, vEntity.ClientId);
                toRemove.push_back(vEntity.Id);
            }
        }

        while(true) { // -- stupid.. but a safe way to delete elements =/
            int removed = 0;
            int iterator = 0;
            if (toRemove.empty())
                break;
            
            for(auto const &vEntity : nc->player->Entities) {
                if (vEntity.Id == toRemove.at(0)) {
                    nc->player->Entities.erase(nc->player->Entities.begin() + iterator);
                    removed++;
                    break;
                }
                iterator++;
            }

            if (removed == 0) break;
        }

        // -- now loop the global entities list, for creation.
        for (auto const &bEntity : _entities) {
            if (bEntity.second->MapID != nc->player->MapId)
                continue;
            bool create = true;
            for(auto const &vEntity : nc->player->Entities) {
                if (vEntity.Id == bEntity.first) {
                    create = false;
                    break;
                }
            }
            if (bEntity.first == nc->player->tEntity->Id)
                create = false;

            if (create) {
                EntityShort s{};
                s.Id = bEntity.first;
                s.ClientId = bEntity.second->ClientId;
                nc->player->Entities.push_back(s); // -- track the new client
                // -- spawn them :)
                NetworkFunctions::NetworkOutEntityAdd(nc->Id, s.ClientId, Entity::GetDisplayname(s.Id), bEntity.second->Location);
                CPE::PostEntityActions(nc, bEntity.second);
            }
        }
    }
    // Loop through the entire global entities list *again*
    for (auto const &bEntity : _entities) {
        if (bEntity.second->SendPos) {
            bEntity.second->SendPos = 0;
            for(auto const &nc : n->roClients) {
                if (!nc->LoggedIn)
                    continue;

                for (auto const &vEntity : nc->player->Entities) {
                    if (vEntity.Id == bEntity.first)
                        NetworkFunctions::NetworkOutEntityPosition(nc->Id, vEntity.ClientId, bEntity.second->Location);
                }
            }
        }

        if (bEntity.second->SendPosOwn) {
            bEntity.second->SendPosOwn = false;
            for(auto const &nc : n->roClients) {
                if (!nc->LoggedIn)
                    continue;

                if (nc->player->tEntity == bEntity.second) {
                    NetworkFunctions::NetworkOutEntityPosition(nc->Id, 255, bEntity.second->Location);
                }
            }
        }

        if (bEntity.second->SpawnSelf) {
            bEntity.second->SpawnSelf = false;
            for(auto const &nc : n->roClients) {
                if (!nc->LoggedIn)
                    continue;

                if (nc->player->tEntity == bEntity.second) {
                    NetworkFunctions::NetworkOutEntityAdd(nc->Id, 255, Entity::GetDisplayname(bEntity.first), bEntity.second->Location);
                }
            }
        }
    }

}

void Entity::Delete() {

}

void Entity::Add(std::shared_ptr<Entity> e) {
    _entities.insert(std::make_pair(e->Id, e));
    EventEntityAdd ea;
    ea.entityId = e->Id;
    Dispatcher::post(ea);
}

void Entity::SetModel(std::string modelName) {
    Network* nm = Network::GetInstance();

    model = modelName;
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
    resend = true;
}

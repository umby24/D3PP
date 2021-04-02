//
// Created by Wande on 3/17/2021.
//

#include "Entity.h"
const std::string MODULE_NAME = "Entity";
std::map<int, std::shared_ptr<Entity>> Entity::_entities;

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
    shared_ptr<Entity> e = GetPointer(id);
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
                break;
            }
        }
        if (!found) {
            return id;
        }
    }
    Logger::LogAdd(MODULE_NAME, "No free map clientID", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__ );
    return -1;
}

Entity::Entity(std::string name, int mapId, float X, float Y, float Z, float rotation, float look) {
    Prefix = "";
    Name = name;
    Suffix = "";
    Id = GetFreeId();
    ClientId = GetFreeIdClient(mapId);
    MapID = mapId;
    this->X = X;
    this->Y = Y;
    this->Z = Z;
    Rotation = rotation;
    Look = look;
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
}

shared_ptr<Entity> Entity::GetPointer(int id) {
    if (_entities.find(id) == _entities.end())
        return nullptr;

    return _entities.at(id);
}

void Entity::MessageToClients(int id, const std::string& message) {
    Network* n = Network::GetInstance();

    for(auto const &nc : n->_clients) {
        if (nc.second->player->tEntity->Id == id) {
            NetworkFunctions::SystemMessageNetworkSend(nc.second->Id, message);
        }
    }
}

std::string Entity::GetDisplayname(int id) {
    shared_ptr<Entity> e = GetPointer(id);

    if (e == nullptr)
        return "";

    return e->Prefix + e->Name + e->Suffix;
}

shared_ptr<Entity> Entity::GetPointer(std::string name) {
    for(auto const &e : _entities) {
        if (Utils::InsensitiveCompare(e.second->Name, name)) {
            return e.second;
        }
    }

    return nullptr;
}

void Entity::Delete(int id) {
    shared_ptr<Entity> e = GetPointer(id);
    if (e == nullptr)
        return;
    Network* n = Network::GetInstance();

    for(auto const &nc : n->_clients) {
        if (nc.second->player->tEntity == e) {
            nc.second->player->tEntity = nullptr;
        }
    }

    _entities.erase(id);
}

void Entity::Kill() {
    // -- mapSelectId(MapId)
    if (timeMessageDeath < time(nullptr)) {
        timeMessageDeath = time(nullptr) + 2000;
        NetworkFunctions::SystemMessageNetworkSend2All(MapID, "&c" + Name + " died.");
        // -- setPos to map spawn..
        //PositionSet();
    }
}

void Entity::PositionSet(int mapId, float x, float y, float z, float rot, float lk, char priority, bool sendOwn) {
    if (SendPos <= priority) {
        if (mapId != MapID) { // -- Changing map
            // -- mapSelect
            // -- TODO:
        } else {
            if (sendOwn || !SendPosOwn) {
                X = x;
                Y = y;
                Z = z;
                Rotation = rot;
                Look = lk;
                SendPos = priority;
                if (sendOwn)
                    SendPosOwn = true;
            }
        }
    }
}

void Entity::PositionCheck() {
    int mapId = MapID;
    float x = round(X);
    float y = round(Y);
    float z = round(Z);
    // -- if mapselect
    // -- do a teleporter check..
    //else.. set them to a functional map
    // -- check if the block we're touching is a killing block, if so call kill.
    // -- TODO:
}

void Entity::MainFunc() {
    for(auto const &e : _entities) {
        e.second->PositionCheck();
    }

    Send();
}

void Entity::Send() {
    Network *n = Network::GetInstance();
    for(auto const &nc : n->_clients) {
        if (!nc.second->LoggedIn)
            continue;

        std::vector<int> toRemove;
        for(auto const &vEntity : nc.second->player->Entities) {
            shared_ptr<Entity> fullEntity = GetPointer(vEntity.Id);
            if (fullEntity == nullptr) { // -- Entity no longer exists, despawn them.
                NetworkFunctions::NetworkOutEntityDelete(nc.first, vEntity.ClientId);
                toRemove.push_back(vEntity.Id);
                continue;
            }
            bool shouldDelete = false;
            if (fullEntity->MapID != nc.second->player->MapId)
                shouldDelete = true;
            if (nc.second->player->tEntity && nc.second->player->tEntity->Id == vEntity.Id)
                shouldDelete = true;
            if (fullEntity->resend) {
                shouldDelete = true;
                fullEntity->resend = false;
            }

            if (shouldDelete) {
                NetworkFunctions::NetworkOutEntityDelete(nc.first, vEntity.ClientId);
                toRemove.push_back(vEntity.Id);
            }
        }

        while(true) { // -- stupid.. but a safe way to delete elements =/
            int removed = 0;
            int iterator = 0;
            for(auto const &vEntity : nc.second->player->Entities) {
                if (vEntity.Id == toRemove.at(0)) {
                    nc.second->player->Entities.erase(nc.second->player->Entities.begin() + iterator);
                    removed++;
                    break;
                }
                iterator++;
            }
            if (removed == 0 || toRemove.empty())
                break;
        }

        // -- now loop the global entities list, for creation.
        for (auto const &bEntity : _entities) {
            if (bEntity.second->MapID != nc.second->player->MapId)
                continue;
            bool create = true;
            for(auto const &vEntity : nc.second->player->Entities) {
                if (vEntity.Id == bEntity.first) {
                    create = false;
                    break;
                }
            }
            if (create) {
                EntityShort s;
                s.Id = bEntity.first;
                s.ClientId = bEntity.second->ClientId;
                nc.second->player->Entities.push_back(s); // -- track the new client
                // -- spawn them :)
                NetworkFunctions::NetworkOutEntityAdd(nc.first, s.ClientId, Entity::GetDisplayname(s.Id), bEntity.second->X, bEntity.second->Y, bEntity.second->Z, bEntity.second->Rotation, bEntity.second->Look);
            }
        }
    }
    // Loop through the entire global entities list *again*
    for (auto const &bEntity : _entities) {
        if (bEntity.second->SendPos) {
            bEntity.second->SendPos = 0;
            for(auto const &nc : n->_clients) {
                if (!nc.second->LoggedIn)
                    continue;

                for (auto const &vEntity : nc.second->player->Entities) {
                    if (vEntity.Id == bEntity.first)
                        NetworkFunctions::NetworkOutEntityPosition(nc.first, vEntity.ClientId, bEntity.second->X, bEntity.second->Y, bEntity.second->Z, bEntity.second->Rotation, bEntity.second->Look);
                }
            }
        }

        if (bEntity.second->SendPosOwn) {
            bEntity.second->SendPosOwn = false;
            for(auto const &nc : n->_clients) {
                if (!nc.second->LoggedIn)
                    continue;

                if (nc.second->player->tEntity == bEntity.second) {
                    NetworkFunctions::NetworkOutEntityPosition(nc.first, 255, bEntity.second->X, bEntity.second->Y, bEntity.second->Z, bEntity.second->Rotation, bEntity.second->Look);
                }
            }
        }

        if (bEntity.second->SpawnSelf) {
            bEntity.second->SpawnSelf = false;
            for(auto const &nc : n->_clients) {
                if (!nc.second->LoggedIn)
                    continue;

                if (nc.second->player->tEntity == bEntity.second) {
                    NetworkFunctions::NetworkOutEntityAdd(nc.first, 255, Entity::GetDisplayname(bEntity.first), bEntity.second->X, bEntity.second->Y, bEntity.second->Z, bEntity.second->Rotation, bEntity.second->Look);
                }
            }
        }
    }

}

void Entity::Delete() {

}

void Entity::Add(shared_ptr<Entity> e) {
    _entities.insert(std::make_pair(e->Id, e));
}

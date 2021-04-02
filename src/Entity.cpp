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
    
}

//
// Created by Wande on 3/31/2021.
//

#include "Client.h"
const std::string MODULE_NAME = "Client";

void Client::Login(int clientId, std::string name, std::string mppass, char version) {
    Network *n = Network::GetInstance();
    PlayerMain *pm = PlayerMain::GetInstance();
    Player_List *pl = Player_List::GetInstance();

    shared_ptr<NetworkClient> c = n->GetClient(clientId);

    c->player = make_unique<Player>();
    c->player->LoginName = name;
    c->player->MPPass = mppass;
    c->player->ClientVersion = version;

    bool preLoginCorrect = true;
    if (version != 7) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Unknown Client version: " + stringulate(version), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        c->Kick("Unknown client version", true);
    } else if (Chat::StringIV(name)) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Invalid Name: " + name, LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        c->Kick("Invalid name", true);
    } else if (name.empty()) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Empty Name provided: " + stringulate(version), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        c->Kick("Invalid name", true);
    } else if (n->_clients.size() > pm->MaxPlayers) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Login Failed: Server is full", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        c->Kick("Server is full", true);
    }

    if (!preLoginCorrect) {
        return;
    }
    PlayerListEntry *entry = pl->GetPointer(c->player->LoginName);

    if (entry == nullptr) {
        pl->Add(c->player->LoginName);
    } else {
        if (entry->Banned) {
            c->Kick("You are banned", true);
            return;
        }
        entry->Online = 1;
        entry->LoginCounter++;
        entry->IP = c->IP;
        c->GlobalChat = entry->GlobalChat;
        //std::string name, int mapId, float X, float Y, float Z, float rotation, float look
        shared_ptr<Entity> newEntity = std::make_shared<Entity>(name, pm->spawnMapId, 0, 0, 0, 0, 0);
        Entity::Add(newEntity);
        Entity::SetDisplayName(newEntity->Id, "", name, "");
    }
}

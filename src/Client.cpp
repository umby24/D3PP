//
// Created by Wande on 3/31/2021.
//

#include "Client.h"
const std::string MODULE_NAME = "Client";

void Client::Login(int clientId, std::string name, std::string mppass, char version) {
    Network *n = Network::GetInstance();
    PlayerMain *pm = PlayerMain::GetInstance();
    Player_List *pl = Player_List::GetInstance();
    MapMain *mm = MapMain::GetInstance();
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
        shared_ptr<Map> spawnMap = mm->GetPointer(pm->spawnMapId);
        shared_ptr<Entity> newEntity = std::make_shared<Entity>(name, pm->spawnMapId, spawnMap->data.SpawnX, spawnMap->data.SpawnY, spawnMap->data.SpawnZ, spawnMap->data.SpawnRot, spawnMap->data.SpawnLook);
        newEntity->buildMaterial = -1;
        newEntity->playerList = entry;
        c->player->tEntity = newEntity;
        c->LoggedIn = true;
        Entity::Add(newEntity);
        Entity::SetDisplayName(newEntity->Id, "", name, "");
        Logger::LogAdd(MODULE_NAME, "Player Logged in (IP:" + c->IP + " Name:" + name + ")", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
        NetworkFunctions::SystemMessageNetworkSend2All(-1, "Player '" + Entity::GetDisplayname(newEntity->Id) + "' logged in");
        newEntity->model = "default";
        spawnMap->data.Clients += 1;
        pl->SaveFile = true;
    }
}

void Client::Logout(int clientId, std::string message, bool showtoall) {
    Network *n = Network::GetInstance();
    MapMain *mm = MapMain::GetInstance();
    shared_ptr<NetworkClient> c = n->GetClient(clientId);

    Logger::LogAdd(MODULE_NAME, "Player logged out (IP: " + c->IP + " Name: " + c->player->LoginName + " Message: " + message + ")", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    if (c->player && c->player->tEntity) {
        shared_ptr<Map> currentMap = mm->GetPointer(c->player->MapId);
        currentMap->data.Clients -= 1;

        if (showtoall && !c->player->LogoutHide) {
            NetworkFunctions::SystemMessageNetworkSend2All(-1, "Player '" + Entity::GetDisplayname(c->player->tEntity->Id) + "' logged out (" + message + ")");
        }
        Entity::Delete(c->player->tEntity->Id);
        c->player->tEntity = nullptr;
    }
}

void Client::LoginThread() {
    Network *n = Network::GetInstance();
    while (true) {
        watchdog::Watch("Client_Login", "Begin Thread-Slope", 0);
        for(auto const &nc : n->_clients) {
            if (!nc.second->LoggedIn || !nc.second->player->tEntity)
                continue;

            if (nc.second->player->MapId == nc.second->player->tEntity->MapID)
                continue;
            int rank = 0;
            std::string motd = "meh";
            if (nc.second->player->tEntity->playerList)
                rank = nc.second->player->tEntity->playerList->PRank;

            // -- TODO: Get Map MOTD Override..
            // -- Map sending stuff..
        }
        watchdog::Watch("Client_Login", "End Thread-Slope", 2);
        break;
    }
}

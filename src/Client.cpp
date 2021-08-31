//
// Created by Wande on 3/31/2021.
//

#include "Client.h"

#include <memory>

#include "Rank.h"
#include "Logger.h"
#include "Chat.h"
#include "Utils.h"

#include "Network.h"
#include "NetworkClient.h"
#include "Player.h"
#include "Player_List.h"
#include "Entity.h"

#include "Network_Functions.h"
#include "Packets.h"

#include "Map.h"
#include "watchdog.h"
#include "System.h"
#include "Heartbeat.h"

#include "CPE.h"

#include "EventSystem.h"
#include "events/EventClientLogin.h"
#include "events/EventClientLogout.h"

const std::string MODULE_NAME = "Client";

void Client::Login(int clientId, std::string name, std::string mppass, char version) {
    Network *n = Network::GetInstance();
    PlayerMain *pm = PlayerMain::GetInstance();
    Player_List *pl = Player_List::GetInstance();
    MapMain *mm = MapMain::GetInstance();
    Rank *rm = Rank::GetInstance();
    Heartbeat* hbm = Heartbeat::GetInstance();

    std::shared_ptr<NetworkClient> c = n->GetClient(clientId);

    c->player = std::make_unique<Player>();
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
    } else if (n->roClients.size() > pm->MaxPlayers) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Login Failed: Server is full", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        c->Kick("Server is full", true);
    } else if (mm->GetPointer(pm->spawnMapId) == nullptr) {
         preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Login Failed: Spawnmap invalid", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        c->Kick("&eSpawnmap Invalid", true);
    } else if (pm->NameVerification && c->IP != "127.0.0.1" && (!hbm->VerifyName(name, mppass))) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Login Failed: failed name verification", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        c->Kick("&eName verification failed", true);
    }
    
    if (!preLoginCorrect) {
        return;
    }

    PlayerListEntry *entry = pl->GetPointer(c->player->LoginName);

    if (entry == nullptr) {
        pl->Add(c->player->LoginName);
        entry = pl->GetPointer(c->player->LoginName);
    }  
    if (entry->Banned) {
        c->Kick("You are banned", true);
        return;
    }
    entry->Online = 1;
    entry->LoginCounter++;
    entry->IP = c->IP;
    entry->Save = true;
    
    if (entry->OntimeCounter < 0) {
        entry->OntimeCounter = 0;
    }

    c->GlobalChat = entry->GlobalChat;
    std::shared_ptr<Map> spawnMap = mm->GetPointer(pm->spawnMapId);
    std::shared_ptr<Entity> newEntity = std::make_shared<Entity>(name, pm->spawnMapId, spawnMap->data.SpawnX, spawnMap->data.SpawnY, spawnMap->data.SpawnZ, spawnMap->data.SpawnRot, spawnMap->data.SpawnLook);
    RankItem currentRank = rm->GetRank(entry->PRank, false);

    newEntity->buildMaterial = -1;
    newEntity->playerList = entry;
    newEntity->model = "default";
    
    c->player->tEntity = newEntity;
    c->LoggedIn = true;
    Entity::Add(newEntity);
    Entity::SetDisplayName(newEntity->Id, currentRank.Prefix, name, currentRank.Suffix);

    Logger::LogAdd(MODULE_NAME, "Player Logged in (IP:" + c->IP + " Name:" + name + ")", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    NetworkFunctions::SystemMessageNetworkSend2All(-1, "&ePlayer '" + Entity::GetDisplayname(newEntity->Id) + "&e' logged in");
    NetworkFunctions::SystemMessageNetworkSend(c->Id, pm->WelcomeMessage);
    
    EventClientLogin ecl;
    ecl.clientId = c->Id;
    Dispatcher::post(ecl);

    spawnMap->data.Clients += 1;
    pl->SaveFile = true;
}

void Client::LoginCpe(int clientId, std::string name, std::string mppass, char version) {
    Network *n = Network::GetInstance();

    std::shared_ptr<NetworkClient> c = n->GetClient(clientId);

    c->player = std::make_unique<Player>();
    c->player->LoginName = name;
    c->player->MPPass = mppass;
    c->CPE = true;
    c->player->ClientVersion = version;
    Packets::SendExtInfo(c, "D3PP Server Alpha", 14);
    Packets::SendExtEntry(c, CUSTOM_BLOCKS_EXT_NAME, 1);
    Packets::SendExtEntry(c, HELDBLOCK_EXT_NAME, 1);
    Packets::SendExtEntry(c, CLICK_DISTANCE_EXT_NAME, 1);
    Packets::SendExtEntry(c, CHANGE_MODEL_EXT_NAME, 1);
    Packets::SendExtEntry(c, EXT_PLAYER_LIST_EXT_NAME, 2);
    Packets::SendExtEntry(c, EXT_WEATHER_CONTROL_EXT_NAME, 1);
    Packets::SendExtEntry(c, ENV_APPEARANCE_EXT_NAME, 1);
    Packets::SendExtEntry(c, MESSAGE_TYPES_EXT_NAME, 1);
    Packets::SendExtEntry(c, BLOCK_PERMISSIONS_EXT_NAME, 1);
    Packets::SendExtEntry(c, ENV_COLORS_EXT_NAME, 1);
    Packets::SendExtEntry(c, HOTKEY_EXT_NAME, 1);
    Packets::SendExtEntry(c, HACKCONTROL_EXT_NAME, 1);
    Packets::SendExtEntry(c, SELECTION_CUBOID_EXT_NAME, 1);
    Packets::SendExtEntry(c, LONG_MESSAGES_EXT_NAME, 1);
    Logger::LogAdd(MODULE_NAME, "LoginCPE complete", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Client::Logout(int clientId, std::string message, bool showtoall) {
    Network *n = Network::GetInstance();
    MapMain *mm = MapMain::GetInstance();
    std::shared_ptr<NetworkClient> c = n->GetClient(clientId);
    if (!c || c == nullptr || c == NULL) {
        return;
    }
    if (!c->LoggedIn) {
        return;
    }

    Logger::LogAdd(MODULE_NAME, "Player logged out (IP: " + c->IP + " Name: " + c->player->LoginName + " Message: " + message + ")", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);

    if (c->player && c->player->tEntity) {
        std::shared_ptr<Map> currentMap = mm->GetPointer(c->player->MapId);
        currentMap->data.Clients -= 1;

        if (showtoall && !c->player->LogoutHide) {
            NetworkFunctions::SystemMessageNetworkSend2All(-1, "&ePlayer '" + Entity::GetDisplayname(c->player->tEntity->Id) + "&e' logged out (" + message + ")");
        }
        for(auto const &nc : n->roClients) {
            if (CPE::GetClientExtVersion(nc, EXT_PLAYER_LIST_EXT_NAME) > 0) {
                Packets::SendExtRemovePlayerName(nc, c->player->NameId);
            }
        }

        Entity::Delete(c->player->tEntity->Id);
        c->player->tEntity = nullptr;
    }
    
    EventClientLogout ecl;
    ecl.clientId = clientId;
    Dispatcher::post(ecl);
}

void Client::LoginThread() {
    Network *n = Network::GetInstance();
    System* sMain = System::GetInstance();
    MapMain* mMain = MapMain::GetInstance();
    Rank* rMain = Rank::GetInstance();

    while (System::IsRunning) {
        watchdog::Watch("Client_login", "Begin Thread-Slope", 0);
        for(auto const &nc : n->roClients) {
            if (!nc->LoggedIn || !nc->player->tEntity)
                continue;

            if (nc->player->MapId == nc->player->tEntity->MapID)
                continue;

            int rank = 0;
            std::string motd = "";

            if (nc->player->tEntity->playerList)
                rank = nc->player->tEntity->playerList->PRank;

            motd = mMain->GetMapMOTDOverride(nc->player->tEntity->MapID);
            
            if (motd.empty())
                motd = sMain->Motd;

            int clientId = nc->Id;
            int eMapId = nc->player->tEntity->MapID;
            float entityX = nc->player->tEntity->X;
            float entityY = nc->player->tEntity->Y;
            float entityZ = nc->player->tEntity->Z;
            float entityRot = nc->player->tEntity->Rotation;
            float entityLook = nc->player->tEntity->Look;

            RankItem ri = rMain->GetRank(rank, false);
            std::shared_ptr<Map> sendMap = mMain->GetPointer(eMapId);
            
            NetworkFunctions::SystemLoginScreen(clientId, sMain->ServerName, motd, ri.OnClient);
            sendMap->Send(clientId);
            
            nc->player->tEntity->SpawnSelf = true;
            
            for(auto const &pe : nc->player->Entities) {
                NetworkFunctions::NetworkOutEntityDelete(clientId, pe.ClientId);
            }
            nc->player->Entities.clear();
            // -- Map is now sent, change client map.
            nc->player->MapId = eMapId;
        }
        watchdog::Watch("Client_login", "End Thread-Slope", 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

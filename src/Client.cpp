//
// Created by Wande on 3/31/2021.
//

#include "Client.h"

#include <memory>

#include "Rank.h"
#include "common/Logger.h"
#include "common/Configuration.h"
#include "common/MinecraftLocation.h"
#include "Utils.h"

#include "network/Chat.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/Server.h"
#include "network/Network_Functions.h"
#include "network/Packets.h"
#include "network/packets/ExtRemovePlayerName.h"
#include "world/Player.h"
#include "common/Player_List.h"
#include "world/Entity.h"
#include "world/MapMain.h"


#include "world/Map.h"
#include "System.h"
#include "plugins/Heartbeat.h"

#include "CPE.h"

#include "EventSystem.h"
#include "events/EventClientLogin.h"
#include "events/EventClientLogout.h"

const std::string MODULE_NAME = "Client";

using namespace D3PP::world;
using namespace D3PP::Common;

void Client::Login(int clientId, std::string name, std::string mppass, char version) {
    Player_List *pl = Player_List::GetInstance();
    MapMain *mm = MapMain::GetInstance();
    Rank *rm = Rank::GetInstance();
    Heartbeat* hbm = Heartbeat::GetInstance();

    std::shared_ptr<NetworkClient> c = std::static_pointer_cast<NetworkClient>(Network::GetClient(clientId));

    auto myPlayer = std::make_shared<D3PP::world::Player>(name, mppass, version);

    myPlayer->myClientId = c->GetId();
    c->player = myPlayer;


    bool preLoginCorrect = true;
    if (version != 7) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Unknown Client version: " + stringulate(version), LogType::L_ERROR, GLF);
        c->Kick("Unknown client version", true);
    } else if (Chat::StringIV(name)) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Invalid Name: " + name, LogType::L_ERROR, GLF);
        c->Kick("Invalid name", true);
    } else if (name.empty()) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Empty Name provided: " + stringulate(version), LogType::L_ERROR, GLF);
        c->Kick("Invalid name", true);
    } else if (D3PP::network::Server::roClients.size() > Configuration::NetSettings.MaxPlayers) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Login Failed: Server is full", LogType::L_ERROR, GLF);
        c->Kick("Server is full", true);
    } else if (mm->GetPointer(Configuration::GenSettings.SpawnMapId) == nullptr) {
         preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Login Failed: Spawnmap invalid", LogType::L_ERROR, GLF);
        c->Kick("&eSpawnmap Invalid", true);
    } else if (Configuration::NetSettings.VerifyNames && c->IP != "127.0.0.1" && (!hbm->VerifyName(name, mppass))) {
        preLoginCorrect = false;
        Logger::LogAdd(MODULE_NAME, "Login Failed: failed name verification", LogType::L_ERROR, GLF);
        c->Kick("&eName verification failed", true);
    }
    
    if (!preLoginCorrect) {
        return;
    }

    std::shared_ptr<PlayerListEntry> entry = pl->GetPointer(c->GetLoginName());

    if (entry == nullptr) {
        pl->Add(c->GetLoginName());
        entry = pl->GetPointer(c->GetLoginName());
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
    std::shared_ptr<Map> spawnMap = mm->GetPointer(Configuration::GenSettings.SpawnMapId);
    MinecraftLocation spawnLocation = spawnMap->GetSpawn();

    std::shared_ptr<Entity> newEntity = std::make_shared<Entity>(name, Configuration::GenSettings.SpawnMapId, spawnLocation, c);
    RankItem currentRank = rm->GetRank(entry->PRank, false);

    newEntity->buildMaterial = -1;
    newEntity->playerList = entry;
    newEntity->model = "default";
    
    myPlayer->tEntity = newEntity;
    myPlayer->MapId = spawnMap->ID;
    c->LoggedIn = true;


    std::string motd = MapMain::GetMapMOTDOverride(spawnMap->ID);

    if (motd.empty())
        motd = Configuration::GenSettings.motd;

    NetworkFunctions::SystemLoginScreen(c->GetId(), Configuration::GenSettings.name, motd, currentRank.OnClient);


    Entity::Add(newEntity);
    Entity::SetDisplayName(newEntity->Id, currentRank.Prefix, name, currentRank.Suffix);

    myPlayer->SendMap();

//    newEntity->SpawnSelf = true;
    newEntity->Spawn();

    Logger::LogAdd(MODULE_NAME, "Player Logged in (IP:" + c->IP + " Name:" + name + ")", LogType::NORMAL, GLF);
    NetworkFunctions::SystemMessageNetworkSend2All(-1, "&ePlayer '" + Entity::GetDisplayname(newEntity->Id) + "&e' logged in");
    NetworkFunctions::SystemMessageNetworkSend(c->GetId(), Configuration::GenSettings.WelcomeMessage);
    
    EventClientLogin ecl;
    ecl.clientId = c->GetId();
    Dispatcher::post(ecl);

    newEntity->SendPosOwn = true;
    newEntity->HandleMove();
    spawnMap->AddEntity(newEntity);

    pl->SaveFile = true;
}

void Client::LoginCpe(int clientId, std::string name, std::string mppass, char version) {
    std::shared_ptr<NetworkClient> c = std::static_pointer_cast<NetworkClient>(Network::GetClient(clientId));

    auto myPlayer = std::make_unique<D3PP::world::Player>(name, mppass, version);
    c->CPE = true;
    myPlayer->myClientId = c->GetId();

    Packets::SendExtInfo(c, "D3PP Server Alpha", 22);
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
    Packets::SendExtEntry(c, PLAYER_CLICK_EXT_NAME, 1);
    Packets::SendExtEntry(c, TWOWAY_PING_EXT_NAME, 1);
    Packets::SendExtEntry(c, BLOCK_DEFS_EXT_NAME, 1);
    Packets::SendExtEntry(c, BLOCK_DEFS_EXTENDED_EXT_NAME, 2);
    Packets::SendExtEntry(c, EXTENDED_TEXTURES_EXT_NAME, 1);
    Packets::SendExtEntry(c, FULL_CODEPAGE_EXT_NAME, 1);
    Packets::SendExtEntry(c, CUSTOM_PARTICLES_EXT_NAME, 1);
    Packets::SendExtEntry(c, CUSTOM_MODELS_EXT_NAME, 2);
    c->player = std::move(myPlayer);
    Logger::LogAdd(MODULE_NAME, "LoginCPE complete", LogType::DEBUG, GLF);
}

void Client::Logout(int clientId, std::string message, bool showtoall) {
    Network *n = Network::GetInstance();
    MapMain *mm = MapMain::GetInstance();
    std::shared_ptr<NetworkClient> c = std::static_pointer_cast<NetworkClient>(n->GetClient(clientId));
    if (!c || c == nullptr || c == NULL) {
        return;
    }
    if (c->LoggedIn) {
        c->LoggedIn = false;
        Logger::LogAdd(MODULE_NAME, "Player logged out (IP: " + c->IP + " Name: " + c->GetLoginName() + " Message: " + message + ")", LogType::NORMAL, GLF);
    }

    if (c->player && c->GetPlayerInstance()->GetEntity()) {
        std::shared_ptr<Map> currentMap = mm->GetPointer(c->player->GetEntity()->MapID);
        if (currentMap != nullptr) {
            currentMap->RemoveEntity(c->player->GetEntity());
        }

        if (showtoall) {
            NetworkFunctions::SystemMessageNetworkSend2All(-1, "&ePlayer '" + Entity::GetDisplayname(c->GetPlayerInstance()->GetEntity()->Id) + "&e' logged out (" + message + ")");
        }

        D3PP::network::ExtRemovePlayerName rpnPacket(c->player->GetNameId());
        D3PP::network::Server::SendToAll(rpnPacket, EXT_PLAYER_LIST_EXT_NAME, 1);

        c->player->GetEntity()->Despawn();
        Entity::Delete(c->player->GetEntity()->Id);
        //c->player->tEntity = nullptr; TODO:
    }
    
    EventClientLogout ecl;
    ecl.clientId = clientId;
    Dispatcher::post(ecl);
}

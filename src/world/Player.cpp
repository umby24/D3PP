//
// Created by Wande on 3/21/2021.
//

#include "world/Player.h"

#include <utility>

#include "world/Entity.h"
#include "world/Map.h"

#include "events/PlayerEventArgs.h"
#include "events/EventEntityMapChange.h"

#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/Network_Functions.h"
#include "network/Packets.h"
#include "network/Server.h"

#include "common/Player_List.h"

using namespace D3PP::world;

const std::string MODULE_NAME = "Player";
PlayerMain* PlayerMain::Instance = nullptr;

PlayerMain::PlayerMain() {
    this->Interval = std::chrono::seconds(1);
    this->Main= [this] { MainFunc(); };

    OntimeCounter = 0;
    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

void PlayerMain::MainFunc() {
    if (OntimeCounter < time(nullptr)) {
        int difference = time(nullptr) - OntimeCounter;
        OntimeCounter = time(nullptr) + (1*10);
        Player_List* pll = Player_List::GetInstance();

        for(auto &pli : pll->_pList) {
            if (pli.Online) {
                pli.OntimeCounter += difference;
            }
        }
    }
}

PlayerMain *PlayerMain::GetInstance() {
    if (Instance == nullptr)
        Instance = new PlayerMain();

    return Instance;
}

int PlayerMain::GetFreeNameId() {
    int id = 0;
    bool found;

    while (true) {
        found = false;
        for(auto const &nc: D3PP::network::Server::roClients) {
            if (!nc->GetLoggedIn())
                continue;

            if (nc->GetPlayerInstance()->GetNameId() == id)
                found = true;
        }

        if (found)
            id++;
        else {
            return id;
        }
    }
}

void D3PP::world::Player::SendMap() {
    D3PP::world::MapMain* mm = D3PP::world::MapMain::GetInstance();
    std::shared_ptr<D3PP::world::Map> myMap = mm->GetPointer(MapId);
    myMap->Send(myClientId);
    tEntity->SendPosOwn = true;
    tEntity->resend = true;
    auto myClient = Network::GetClient(myClientId);

    if (myClientId != -1 && myClient != nullptr) {
        auto entities = myMap->GetEntities();
        for (auto const& eId : entities) {
            myClient->SpawnEntity(Entity::GetPointer(eId));
        }
    }
}

void D3PP::world::Player::PlayerClicked(ClickButton button, ClickAction action, short yaw, short pitch, char targetEntity,
                           D3PP::Common::Vector3S targetBlock, ClickTargetBlockFace blockFace) {
    PlayerClickEventArgs event;
    event.playerId = this->tEntity->playerList->Number;
    event.button = button;
    event.action = action;
    event.yaw = yaw;
    event.pitch = pitch;
    event.targetEntity = targetEntity;
    event.targetBlock = targetBlock;
    event.blockFace = blockFace;
    Dispatcher::post(event);
}

void D3PP::world::Player::ChangeMap(std::shared_ptr<D3PP::world::Map> map) {
    auto myClient =  Network::GetClient(myClientId);

    if (myClientId != -1 && myClient != nullptr) {
        D3PP::world::MapMain* mm = D3PP::world::MapMain::GetInstance();
        std::shared_ptr<D3PP::world::Map> currentMap = mm->GetPointer(MapId);
        std::string entityName = Entity::GetDisplayname(tEntity->Id);
        std::string mapChangeMessage = "&ePlayer '" + entityName + "&e' changed to map '" + map->Name() + "'";

        NetworkFunctions::SystemMessageNetworkSend2All(map->ID, mapChangeMessage);
        NetworkFunctions::SystemMessageNetworkSend2All(MapId, mapChangeMessage);

        DespawnEntities(); // -- Despawn others on us
        tEntity->Despawn(); // -- Despawn us for others

        MapId = map->ID; // -- Set our new map ID
        currentMap->RemoveEntity(tEntity);
        map->AddEntity(tEntity);
        tEntity->MapID = MapId;
        tEntity->ClientId = Entity::GetFreeIdClient(MapId);

        EventEntityMapChange emc;
        emc.entityId = tEntity->Id;
        emc.oldMapId = currentMap->ID;
        emc.newMapId = MapId;
        Dispatcher::post(emc);

        SendMap();

        auto entities = map->GetEntities();
        for(auto const &eId : entities) {
            myClient->SpawnEntity(Entity::GetPointer(eId)); // -- Spawn the new entities on us
        }

        tEntity->Location = map->GetSpawn();
        tEntity->SendPosOwn = true;
        tEntity->SpawnSelf = true;

        tEntity->HandleMove();
        tEntity->Spawn(); // -- Spawn us on other entities.
    }
}

void D3PP::world::Player::DespawnEntities() {
    auto* mm = D3PP::world::MapMain::GetInstance();
    auto myClient = Network::GetClient(myClientId);

    if (myClientId != -1 && myClient != nullptr) {
        auto entities = mm->GetPointer(MapId)->GetEntities();
        for(auto const &eId : entities) {
            myClient->DespawnEntity(Entity::GetPointer(eId));
        }
    }
}

std::string D3PP::world::Player::GetLoginName() {
    return LoginName;
}

D3PP::world::Player::Player(std::string name, std::string mppass, char clientVersion) {
    MapId = -1;
    timeDeathMessage = 0;
    timeBuildMessage = 0;
    LogoutHide = false;
    NameId = PlayerMain::GetFreeNameId();
    LoginName = std::move(name);
    MPPass = std::move(mppass);
    ClientVersion = clientVersion;
}

int D3PP::world::Player::GetId() {
    return tEntity->playerList->Number;
}

int D3PP::world::Player::GetRank() {
    return tEntity->playerList->PRank;
}

int D3PP::world::Player::GetCustomBlockLevel() {
    return Network::GetClient(myClientId)->GetCustomBlocksLevel();
}

std::shared_ptr<Entity> D3PP::world::Player::GetEntity() {
    return tEntity;
}

void D3PP::world::Player::Login() {

}

void D3PP::world::Player::Logout() {

}

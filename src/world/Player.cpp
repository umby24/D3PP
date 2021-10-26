//
// Created by Wande on 3/21/2021.
//

#include "world/Player.h"
#include <iomanip>
#include <network/Network.h>
#include <world/Map.h>
#include "network/NetworkClient.h"
#include "world/Entity.h"
#include "common/Player_List.h"
#include "network/Network_Functions.h"
#include "network/Packets.h"

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
    Network* nm = Network::GetInstance();
    int id = 0;
    bool found = false;
    while (true) {
        found = false;
        for(auto const &nc: nm->roClients) {
            if (!nc->LoggedIn)
                continue;

            if (nc->player->NameId == id)
                found = true;
        }

        if (found)
            id++;
        else {
            return id;
        }
    }
}

Player::Player() {
    ClientVersion = 0;
    MapId = -1;
    timeDeathMessage = 0;
    timeBuildMessage = 0;
    LogoutHide = false;
    NameId = PlayerMain::GetFreeNameId();
}

void Player::SendMap() {
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> myMap = mm->GetPointer(MapId);
    myMap->Send(myClientId);
    Entities.clear(); // -- Bounce our entities
    tEntity->SendPosOwn = true;
    tEntity->resend = true;
}

void Player::PlayerClicked(ClickButton button, ClickAction action, short yaw, short pitch, char targetEntity,
                           Vector3S targetBlock, ClickTargetBlockFace blockFace) {

    // -- Trigger a lua event.
}

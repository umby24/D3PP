//
// Created by Wande on 3/21/2021.
//

#include "world/Player.h"
#include <iomanip>
#include <network/Network.h>
#include <world/Map.h>
#include "network/NetworkClient.h"
#include "world/Entity.h"
#include "common/Files.h"
#include "common/Player_List.h"
#include "common/Logger.h"
#include "Utils.h"
#include "network/Network_Functions.h"
#include "network/Packets.h"

const std::string MODULE_NAME = "Player";
PlayerMain* PlayerMain::Instance = nullptr;

void PlayerMain::Save() {
    json j;
    j["WelcomeMessage"] = WelcomeMessage;
    j["MaxPlayers"] = MaxPlayers;
    j["NameVerification"] = NameVerification;
    j["KillMode"] = killMode;
    j["KillSpawnMapId"] = killMapId;
    j["KillSpawnX"] = killSpawnX;
    j["KillSpawnY"] = killSpawnY;
    j["KillSpawnZ"] = killSpawnZ;
    j["KillSpawnRot"] = killSpawnRot;
    j["KillSpawnLook"] = killSpawnLook;
    j["SpawnMapId"] = spawnMapId;

    std::ofstream oStream(fileName, std::ios::trunc);
    oStream << std::setw(4) << j;
    oStream.flush();
    oStream.close();

    time_t modTime = Utils::FileModTime(fileName);
    LastFileDate = modTime;

    Logger::LogAdd(MODULE_NAME, "File saved [" + fileName + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

PlayerMain::PlayerMain() {
    Files* f = Files::GetInstance();
    fileName = f->GetFile(MODULE_NAME);
    this->Interval = std::chrono::seconds(1);
    this->Setup = [this] { Load(); };
    this->Main= [this] { MainFunc(); };
    this->Teardown = [this] { Save(); };
    MaxPlayers = 32;
    NameVerification = true;
    killMode = KillMode::MAP_SPAWN;
    killMapId = 0;
    WelcomeMessage = "";
    killSpawnX = 0;
    killSpawnY = 0;
    killSpawnZ = 0;
    killSpawnRot = 0;
    killSpawnLook = 0;
    spawnMapId = 0;
    SaveFile = false;
    LastFileDate = 0;
    OntimeCounter = 0;
    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

void PlayerMain::Load() {
    json j;
    std::ifstream iStream(fileName);

    if (!iStream.is_open()) {
        Logger::LogAdd(MODULE_NAME, "Failed to load Player!!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        WelcomeMessage = "Welcome to D3";
        MaxPlayers = 32;
        NameVerification = true;
        killMode = KillMode::MAP_SPAWN;
        SaveFile = true;
        spawnMapId = 0;
        return;
    }

    try {
        iStream >> j;
    } catch (int ex) {
        Logger::LogAdd(MODULE_NAME, "Failed to load Player!!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        WelcomeMessage = "Welcome to D3";
        MaxPlayers = 32;
        NameVerification = true;
        killMode = KillMode::MAP_SPAWN;
        SaveFile = true;
        spawnMapId = 0;
        return;
    }
    iStream.close();

    WelcomeMessage = j["WelcomeMessage"];
    MaxPlayers = j["MaxPlayers"];
    NameVerification = j["NameVerification"];
    killMode = j["KillMode"];
    killMapId = j["KillSpawnMapId"];
    killSpawnX = j["KillSpawnX"];
    killSpawnY = j["KillSpawnY"];
    killSpawnZ = j["KillSpawnZ"];
    killSpawnRot = j["KillSpawnRot"];
    killSpawnLook = j["KillSpawnLook"];
    spawnMapId = j["SpawnMapId"];
}

void PlayerMain::MainFunc() {
    if (SaveFile) {
        SaveFile = false;
        Save();
    }
    time_t modTime = Utils::FileModTime(fileName);

    if (modTime != LastFileDate) {
        Load();
        LastFileDate = modTime;
    }
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

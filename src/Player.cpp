//
// Created by Wande on 3/21/2021.
//

#include "Player.h"
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

    Logger::LogAdd(MODULE_NAME, "File saved.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

PlayerMain::PlayerMain() {
    Files* f = Files::GetInstance();
    fileName = f->GetFile(MODULE_NAME);
    this->Interval = std::chrono::seconds(1);
    this->Setup = [this] { Load(); };
    this->Main= [this] { MainFunc(); };
    this->Teardown = [this] { Save(); };

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

        // -- TODO: Player_List: Player_Ontime_Counter_Add(seconds);
    }
}

PlayerMain *PlayerMain::GetInstance() {
    if (Instance == nullptr)
        Instance = new PlayerMain();

    return Instance;
}

Player::Player() {
    ClientVersion = 0;
    MapId = -1;
    timeDeathMessage = 0;
    timeBuildMessage = 0;
    LogoutHide = false;
}

//
// Created by Wande on 3/21/2021.
//

#ifndef D3PP_PLAYER_H
#define D3PP_PLAYER_H
#include <string>
#include "TaskScheduler.h"
#include "Files.h"
#include "json.hpp"

using json = nlohmann::json;

enum KillMode {
    MAP_SPAWN = 0,
    GLOBAL_SPAWN,
    KICK,
    BAN
};

class PlayerMain : TaskItem {
public:
    PlayerMain();
    std::string WelcomeMessage;
    int MaxPlayers;
    bool NameVerification;
    KillMode killMode;
    int killMapId;
    int killSpawnX;
    int killSpawnY;
    int killSpawnZ;
    int killSpawnRot;
    int killSpawnLook;
    int spawnMapId;
private:
    bool SaveFile;
    time_t LastFileDate;
    time_t OntimeCounter;
    std::string fileName;

    void MainFunc();
    void Load();
    void Save();
};

class Player : TaskItem {
public:
    Player();

    int GetAttribute(std::string attrName); // -- REQ: Player_List
    std::string GetAttributeStr(std::string attrName); // -- REQ: Player_List

    void SetAttribute(std::string attrName, int value); // -- REQ: Player_List
    void SetAttribute(std::string attrName, std::string value); // -- REQ: Player_List

    void SetRank(int rank, std::string reason); // -- REQ: Player_List
    void Kick(std::string reason, int count, bool log, bool show);
    void Ban(std::string reason);
    void Mute(int minutes, std::string reason);
    void Unmute();
    void Stop(std::string reason);
    void Unstop();
    void SetGlobal(bool globalChat);
    void GetGlobal();
};

#endif //D3PP_PLAYER_H

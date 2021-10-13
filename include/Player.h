//
// Created by Wande on 3/21/2021.
//

#ifndef D3PP_PLAYER_H
#define D3PP_PLAYER_H
#include <string>
#include <memory>
#include <chrono>
#include <vector>

#include "json.hpp"
#include "TaskScheduler.h"
#include "MinecraftLocation.h"

struct EntityShort;
class Entity;

using json = nlohmann::json;

enum KillMode {
    MAP_SPAWN = 0,
    GLOBAL_SPAWN,
    KICK,
    BAN
};

enum ClickButton {
    LEFT = 0,
    RIGHT,
    MIDDLE
};

enum ClickAction {
    Pressed = 0,
    Released
};

enum ClickTargetBlockFace {
    AwayX = 0,
    TowardsX,
    AwayY,
    TowardY,
    AwayZ,
    TowardZ,
    NONE
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
    static PlayerMain* GetInstance();
    static int GetFreeNameId();
private:
    static PlayerMain* Instance;
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
    // -- Properties
    std::string LoginName;
    std::string MPPass;
    char ClientVersion{};
    int MapId{};
    std::vector<EntityShort> Entities;
    std::shared_ptr<Entity> tEntity;
    short NameId{};
    std::string lastPrivateMessage;
    int timeDeathMessage{};
    int timeBuildMessage{};
    bool LogoutHide{};
    int myClientId;

    // -- Methods
    Player();
    void SendMap();
    void PlayerClicked(ClickButton button, ClickAction action, short yaw, short pitch, char targetEntity, Vector3S targetBlock, ClickTargetBlockFace blockFace);
};

#endif //D3PP_PLAYER_H

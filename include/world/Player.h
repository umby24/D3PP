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
#include "common/TaskScheduler.h"
#include "common/MinecraftLocation.h"

struct EntityShort;
class Entity;
class Map;

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

    static PlayerMain* GetInstance();
    static int GetFreeNameId();
private:
    static PlayerMain* Instance;
    time_t OntimeCounter;

    void MainFunc();
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
    void ChangeMap(std::shared_ptr<Map> map);
    void SendMap();
    void PlayerClicked(ClickButton button, ClickAction action, short yaw, short pitch, char targetEntity, Vector3S targetBlock, ClickTargetBlockFace blockFace);
private:
    void DespawnEntities();
};

#endif //D3PP_PLAYER_H

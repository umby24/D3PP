//
// Created by Wande on 3/21/2021.
//

#ifndef D3PP_PLAYER_H
#define D3PP_PLAYER_H
#include <string>
#include <memory>
#include <chrono>
#include <vector>

#include "common/TaskScheduler.h"
#include "common/Vectors.h"
#include "world/IMinecraftPlayer.h"

struct EntityShort;
class Entity;
namespace D3PP::world {
    class Map;
}


enum KillMode {
    MAP_SPAWN = 0,
    GLOBAL_SPAWN,
    KICK,
    BAN
};
// -- Some CPE 'PlayerClicked' related enums
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

/*
    The PlayerMain class is responsible for managing the main player instances
    It's the central location to retrieve 'player' instances.
*/
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

/*
    The Player class represents a player in the Minecraft world.
    It manages player-specific data such as login credentials, current map,
    entity representation, and interaction with the game world.

    This abstracts and separates player-specific logic from the network client,
    allowing for cleaner code and easier management of player states and actions.
*/
namespace D3PP::world {
    class Player : public TaskItem, public IMinecraftPlayer {
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
        Player(const std::string& name, const std::string& mppass, const char &version);
        void ChangeMap(std::shared_ptr<Map> map);

        void SendMap();

        void PlayerClicked(ClickButton button, ClickAction action, short yaw, short pitch, char targetEntity,
                           Common::Vector3S targetBlock, ClickTargetBlockFace blockFace);

         int GetId() override;

         int GetRank() override;
         int GetNameId() override;
         int GetCustomBlockLevel() override;
         std::string GetLoginName() override;
         std::shared_ptr<Entity> GetEntity() override;
        void Login() override;

        void Logout() override;

    private:
        void DespawnEntities();
    };
}

#endif //D3PP_PLAYER_H

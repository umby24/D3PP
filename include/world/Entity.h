//
// Created by Wande on 3/17/2021.
//

#ifndef D3PP_ENTITY_H
#define D3PP_ENTITY_H
#define Client_Player_Buildmode_Variables 5

#include <string>
#include <map>
#include <memory>

#include "common/TaskScheduler.h"
#include "common/MinecraftLocation.h"

class PlayerListEntry;
class Player;
class NetworkClient;

struct BuildVariable {
    float X;
    float Y;
    float Z;
    float Float;
    std::string String;
    int Long;
};

struct EntityShort {
    int Id;
    char ClientId;
};

class Entity {
public:
    // -- Properties:
    int Id;
    std::string Prefix;
    std::string Name;
    std::string Suffix;
    char ClientId;
    PlayerListEntry* playerList;
    bool resend;
    // --
    int MapID;
    MinecraftLocation Location;
    bool SendPosOwn;
    char SendPos;
    // --
    time_t timeMessageDeath;
    time_t timeMessageOther;
    std::string lastPrivateMessage;
    // --
    unsigned heldBlock;
    std::string model;
    unsigned lastMaterial;
    short buildMaterial;
    std::string BuildMode;
    char BuildState;
    BuildVariable variables[Client_Player_Buildmode_Variables];
    std::string ChatBuffer;
    bool SpawnSelf;

    // -- Methods:
    Entity(std::string name, int mapId, float X, float Y, float Z, float rotation, float look);
    Entity(std::string name, int mapId, MinecraftLocation loc, std::shared_ptr<NetworkClient> c);

    static std::shared_ptr<Entity> GetPointer(int id, bool isClientId = false);
    static std::shared_ptr<Entity> GetPointer(const std::string& name);
    static std::string GetDisplayname(int id);
    static void SetDisplayName(int id, std::string prefix, std::string name, std::string suffix);
    static void MessageToClients(int id, const std::string& message);
    static void Add(const std::shared_ptr<Entity>& e);
    static void Delete(int id);

    void Spawn();
    void Despawn();

    void Kill();
    void PositionCheck();
    void PositionSet(int mapId, MinecraftLocation location, unsigned char priority, bool sendOwn);
    void SetModel(std::string modelName);
    void HandleMove();
    static int GetFreeId();
    static int GetFreeIdClient(int mapId);
    void Resend(int id);

    static std::map<int, std::shared_ptr<Entity>> AllEntities;
    static std::mutex entityMutex;
private:
    std::shared_ptr<NetworkClient> associatedClient;
};

class EntityMain : TaskItem {
    public:
        EntityMain();
        void MainFunc();
    private:
};
#endif //D3PP_ENTITY_H

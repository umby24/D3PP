//
// Created by Wande on 3/17/2021.
//

#ifndef D3PP_ENTITY_H
#define D3PP_ENTITY_H

#include <string>
#include <map>
#include <memory>

#include "TaskScheduler.h"

class PlayerListEntry;
class Player;

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
    float X;
    float Y;
    float Z;
    float Rotation;
    float Look;
    bool SendPosOwn;
    char SendPos;
    // --
    int timeMessageDeath;
    int timeMessageOther;
    std::string lastPrivateMessage;
    // --
    unsigned heldBlock;
    std::string model;
    unsigned lastMaterial;
    short buildMaterial;
    std::string BuildMode;
    char BuildState;
    // -- array of build variables
    std::string ChatBuffer;
    bool SpawnSelf;

    // -- Methods:
    Entity(std::string name, int mapId, float X, float Y, float Z, float rotation, float look);

    static std::shared_ptr<Entity> GetPointer(int id);
    static std::shared_ptr<Entity> GetPointer(std::string name);
    static std::string GetDisplayname(int id);
    static void SetDisplayName(int id, std::string prefix, std::string name, std::string suffix);
    static void MessageToClients(int id, const std::string& message);
    static void Add(std::shared_ptr<Entity> e);
    static void Delete(int id);

    void Kill();
    void PositionCheck();
    void PositionSet(int mapId, float x, float y, float z, float rot, float lk, unsigned char priority, bool sendOwn);
    static void Send();
    static int GetFreeId();
    static int GetFreeIdClient(int mapId);
    void Delete();
    void Resend(int id);
    static std::map<int, std::shared_ptr<Entity>> _entities;
private:
    
};

class EntityMain : TaskItem {
    public:
        EntityMain();
        void MainFunc();
    private:
};
#endif //D3PP_ENTITY_H

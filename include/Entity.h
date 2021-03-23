//
// Created by Wande on 3/17/2021.
//

#ifndef D3PP_ENTITY_H
#define D3PP_ENTITY_H

#include <string>
#include <map>
#include <memory>
#include "Player_List.h"

// -- Dependencies:
//-> Player
//-> Map
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
    Player_List* playerList;
    bool resend;
    // --
    int MapID;
    float X;
    float Y;
    float Z;
    float Rotation;
    float Look;
    bool SendPosOwn;
    bool SendPos;
    // --
    int timeMessageDeath;
    int timeMessageOther;
    std::string lastPrivateMessage;
    // --
    char heldBlock;
    std::string model;
    char lastMaterial;
    short buildMaterial;
    std::string BuildMode;
    char BuildState;
    // -- array of build variables
    std::string ChatBuffer;
    bool SpawnSelf;

    // -- Methods:
    Entity(std::string name, int mapId, float X, float Y, float Z, float rotation, float look);

    static void GetPointer(int id);
    static std::string GetDisplayname(int id);
    static void SetDisplayName(int id, std::string prefix, std::string name, std::string suffix);
    void Kill();
    void PositionCheck();
    void PositionSet();
    void Send();
    void MainFunc();
    int GetFreeId();
    int GetFreeIdClient(int mapId);
    void Delete();
    void Resend(int id);
private:
    std::map<int, std::shared_ptr<Entity>> _entities;
};

#endif //D3PP_ENTITY_H

//
// Created by Wande on 3/17/2021.
//

#ifndef D3PP_ENTITY_H
#define D3PP_ENTITY_H
#include <string>
// -- Dependencies:
//-> Player
//-> NC
//->Network_Functions
//-> Map

class Entity {
public:
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
};

#endif //D3PP_ENTITY_H

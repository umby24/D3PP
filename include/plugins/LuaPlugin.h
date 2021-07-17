#ifndef D3PP_LUA_PLUGIN_H
#define D3PP_LUA_PLUGIN_H

#include <string>
#include <chrono>
#include <memory>
#include <map>
#include <lua.hpp>

#include "TaskScheduler.h"

struct LuaFile {
    std::string FilePath;
    time_t LastLoaded;
};

class LuaPlugin : TaskItem {
public:
    LuaPlugin();
    ~LuaPlugin();

    static LuaPlugin* Instance;
    static LuaPlugin* GetInstance();
    void TriggerCommand(std::string function, int clientId, std::string parsedCmd, std::string text0, std::string text1, std::string op1, std::string op2, std::string op3, std::string op4, std::string op5);
    void TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, std::string function, std::string args);
    void TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, std::string function);
private:
    lua_State* state;
    std::map<std::string, LuaFile> _files;

    void Init();
    void MainFunc();
    void BindFunctions();
    void LoadFile(std::string path);
    // -- Lua interface functions :)
    // -- Client Functions
    int LuaClientGetTable(lua_State *L);
    int LuaClientGetMapId(lua_State *L);
    int LuaClientGetIp(lua_State *L);
    int LuaClientGetLoginName(lua_State *L);
    int LuaClientGetLoggedIn(lua_State *L);
    int LuaClientGetEntity(lua_State *L);
    // -- Build Mode
    // -- Build Functions
    // -- Entity Functions
    int LuaEntityGetTable(lua_State *L);
    int LuaEntityAdd(lua_State *L);
    int LuaEntityDelete(lua_State *L);
    int LuaEntityGetPlayer(lua_State *L);
    int LuaEntityGetMapId(lua_State *L);
    int LuaEntityGetX(lua_State *L);
    int LuaEntityGetY(lua_State *L);
    int LuaEntityGetZ(lua_State *L);
    int LuaEntityGetRotation(lua_State *L);
    int LuaEntityGetLook(lua_State *L);
    int LuaEntityResend(lua_State *L);
    int LuaEntityMessage2Clients(lua_State *L);
    int LuaEntityDisplaynameGet(lua_State *L);
    int LuaEntityDisplaynameSet(lua_State *L);
    int LuaEntityPositionSet(lua_State *L);
    int LuaEntityKill(lua_State *L);
    // -- Player Functions
    // -- Map functions
    int LuaMapBlockChange(lua_State *L);
    int LuaMapBlockGetType(lua_State *L);
    int LuaMapBlockGetPlayer(lua_State *L);
    int LuaMessageToAll(lua_State *L);
    int LuaMessage(lua_State *L);

};
#endif
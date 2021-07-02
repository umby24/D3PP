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
    void TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, std::string function, std::string args);
private:
    lua_State* state;
    std::map<std::string, LuaFile> _files;

    void Init();
    void MainFunc();
    void BindFunctions();
    void LoadFile(std::string path);
    // -- Lua interface functions :)
    int LuaMapBlockChange(lua_State *L);
    int LuaMapBlockGetType(lua_State *L);
    int LuaMessageToAll(lua_State *L);
};
#endif
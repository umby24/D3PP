#ifndef D3PP_LUA_PLUGIN_H
#define D3PP_LUA_PLUGIN_H

#include <string>
#include <chrono>
#include <memory>
#include <map>
#include <lua.hpp>

struct LuaFile {
    std::string FilePath;
    std::chrono::time_point<std::chrono::system_clock> LastLoaded;
};

class LuaPlugin {
public:
    LuaPlugin();
    ~LuaPlugin();

    static LuaPlugin* Instance;
    static LuaPlugin* GetInstance();
private:
    lua_State* state;
    std::map<std::string, LuaFile> _files;
};
#endif
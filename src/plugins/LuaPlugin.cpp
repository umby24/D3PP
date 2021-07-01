#include "plugins/LuaPlugin.h"

#include "Logger.h"

LuaPlugin* LuaPlugin::Instance = nullptr;

LuaPlugin::LuaPlugin() {
    state = luaL_newstate();
    luaL_openlibs(state);
    luaL_dostring(state, "print(\"hello world from lua\")");

}

LuaPlugin::~LuaPlugin() {
    lua_close(state);
}

LuaPlugin* LuaPlugin::GetInstance() {
    if (Instance == nullptr) {
        Instance = new LuaPlugin();
    }

    return Instance;
}
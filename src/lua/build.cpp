#include "lua/build.h"

#include <lua.hpp>

#include "common/Logger.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "Build.h"
#include "Utils.h"

const struct luaL_Reg LuaBuildLib::lib[] = {
       {"line",  &LuaBuildLinePlayer},
       {"box", &LuaBuildBoxPlayer},
       {"sphere", &LuaBuildSpherePlayer},
       {"rankbox",  &LuaBuildRankBox},
       {NULL, NULL}
};

int LuaBuildLib::openLib(lua_State* L)
{
    lua_getglobal(L, "Build");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaBuildLib::lib, 0);
    lua_setglobal(L, "Build");
    return 1;
}


int LuaBuildLib::LuaBuildLinePlayer(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 12) {
        Logger::LogAdd("Lua", "LuaError: Build_Line_Player called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int playerNumber = luaL_checkinteger(L, 1);
    int mapId = luaL_checkinteger(L, 2);
    int x0 = luaL_checkinteger(L, 3);
    int y0 = luaL_checkinteger(L, 4);
    int z0 = luaL_checkinteger(L, 5);
    int x1 = luaL_checkinteger(L, 6);
    int y1 = luaL_checkinteger(L, 7);
    int z1 = luaL_checkinteger(L, 8);

    short material = luaL_checkinteger(L, 9);
    unsigned char priority = luaL_checkinteger(L, 10);
    bool undo = (luaL_checkinteger(L, 11) > 0);
    bool physics = luaL_checkinteger(L, 12);

    Build::BuildLinePlayer(static_cast<short>(playerNumber), mapId, x0, y0, z0, x1, y1, z1, material, priority, undo, physics);
    return 0;
}

int LuaBuildLib::LuaBuildBoxPlayer(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 14) {
        Logger::LogAdd("Lua", "LuaError: Build_Box_Player called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    int mapId = luaL_checkinteger(L, 2);
    int x0 = luaL_checkinteger(L, 3);
    int y0 = luaL_checkinteger(L, 4);
    int z0 = luaL_checkinteger(L, 5);
    int x1 = luaL_checkinteger(L, 6);
    int y1 = luaL_checkinteger(L, 7);
    int z1 = luaL_checkinteger(L, 8);

    short material = luaL_checkinteger(L, 9);
    short replaceMaterial = luaL_checkinteger(L, 10);
    bool hollow = (luaL_checkinteger(L, 11) > 0);
    unsigned char priority = luaL_checkinteger(L, 12);
    bool undo = (luaL_checkinteger(L, 13) > 0);
    bool physics = lua_toboolean(L, 14);

    Build::BuildBoxPlayer(static_cast<short>(playerNumber), mapId, x0, y0, z0, x1, y1, z1, material, static_cast<char>(replaceMaterial), hollow, priority, undo, physics);
    return 0;
}

int LuaBuildLib::LuaBuildSpherePlayer(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 12) {
        Logger::LogAdd("Lua", "LuaError: Build_Sphere_Player called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int playerNumber = luaL_checkinteger(L, 1);
    int mapId = luaL_checkinteger(L, 2);
    int x = luaL_checkinteger(L, 3);
    int y = luaL_checkinteger(L, 4);
    int z = luaL_checkinteger(L, 5);
    float radius = luaL_checkinteger(L, 6);
    short material = luaL_checkinteger(L, 7);
    short replaceMaterial = luaL_checkinteger(L, 8);
    bool hollow = (luaL_checkinteger(L, 9) > 0);
    unsigned char priority = luaL_checkinteger(L, 10);
    bool undo = (luaL_checkinteger(L, 11) > 0);
    bool physics = lua_toboolean(L, 12);

    Build::BuildSpherePlayer(static_cast<short>(playerNumber), mapId, x, y, z, radius, material, static_cast<char>(replaceMaterial), hollow, priority, undo, physics);

    return 0;
}

int LuaBuildLib::LuaBuildRankBox(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 9) {
        Logger::LogAdd("Lua", "LuaError: Build_Rank_Box called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    int X0 = lua_tointeger(L, 2);
    int Y0 = lua_tointeger(L, 3);
    int Z0 = lua_tointeger(L, 4);
    int X1 = lua_tointeger(L, 5);
    int Y1 = lua_tointeger(L, 6);
    int Z1 = lua_tointeger(L, 7);
    int Rank = lua_tointeger(L, 8);
    int MaxRank = lua_tointeger(L, 9);

    Build::BuildRankBox(mapId, X0, Y0, Z0, X1, Y1, Z1, static_cast<short>(Rank), static_cast<short>(MaxRank));

    return 0;
}
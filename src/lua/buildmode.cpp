#include "lua/buildmode.h"

#include <lua.hpp>

#include "common/Logger.h"
#include "common/Vectors.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "world/Player.h"
#include "world/Entity.h"
#include "plugins/LuaPlugin.h"
#include "BuildMode.h"
#include "Utils.h"

const struct luaL_Reg LuaBuildModeLib::lib[] = {
       {"set",  &LuaBuildModeSet},
       {"get", &LuaBuildModeGet},
       {"setstate", &LuaBuildModeStateSet},
       {"getstate",  &LuaBuildModeStateGet},
       {"setcoordinate", &LuaBuildModeCoordinateSet},
       {"getcoordinate", &LuaBuildModeCoordinateGet},
       {"getlong", &LuaBuildModeLongGet},
       {"setlong", &LuaBuildModeLongSet},
       {"setfloat", &LuaBuildModeFloatSet},
       {"getfloat", &LuaBuildModeFloatGet},
       {"getstring", &LuaBuildModeStringGet},
       {"setstring", &LuaBuildModeStringSet},
       {NULL, NULL}
};

int LuaBuildModeLib::openLib(lua_State* L)
{
    lua_getglobal(L, "BuildMode");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaBuildModeLib::lib, 0);
    lua_setglobal(L, "BuildMode");
    return 1;
}


int LuaBuildModeLib::LuaBuildModeSet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Set called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    std::string buildMode(lua_tostring(L, 2));

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetMode(clientId, buildMode);

    return 0;
}

int LuaBuildModeLib::LuaBuildModeGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Get called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    Network* nm = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> networkClient = nm->GetClient(clientId);

    if (networkClient != nullptr) {
        std::shared_ptr<Entity> clientEntity = Entity::GetPointer(clientId, true);

        if (clientEntity != nullptr) {
            lua_pushstring(L, clientEntity->BuildMode.c_str());
        } else {
            lua_pushstring(L, "");
        }
    }
    else {
        lua_pushstring(L, "");
    }

    return 1;
}

int LuaBuildModeLib::LuaBuildModeStateSet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_State_Set called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = luaL_checkinteger(L, 1);
    int buildState = luaL_checkinteger(L, 2);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetState(clientId, static_cast<char>(buildState));

    return 0;
}

int LuaBuildModeLib::LuaBuildModeStateGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_State_Get called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = luaL_checkinteger(L, 1);
    Network* nm = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> networkClient = nm->GetClient(clientId);

    if (networkClient != nullptr) {
        std::shared_ptr<Entity> e = Entity::GetPointer(clientId, true);
        if (e != nullptr) {
            lua_pushinteger(L, e->BuildState);
        }
    }
    else {
        lua_pushinteger(L, -1);
    }

    return 1;
}

int LuaBuildModeLib::LuaBuildModeCoordinateSet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 5) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Coordinate_Set called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    float X = luaL_checknumber(L, 3);
    float Y = luaL_checknumber(L, 4);
    float Z = luaL_checknumber(L, 5);
    D3PP::Common::Vector3F position{X,Y,Z};
    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetCoordinate(clientId, index, position);

    return 0;
}

int LuaBuildModeLib::LuaBuildModeCoordinateGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Coordinate_Get called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int X = -1;
    int Y = -1;
    int Z = -1;

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    X = buildModeMain->GetCoordinateX(clientId, index);
    Y = buildModeMain->GetCoordinateY(clientId, index);
    Z = buildModeMain->GetCoordinateZ(clientId, index);

    lua_pushinteger(L, X);
    lua_pushinteger(L, Y);
    lua_pushinteger(L, Z);

    return 3;
}

int LuaBuildModeLib::LuaBuildModeLongSet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Long_Set called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int X = luaL_checknumber(L, 3);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetInt(clientId, index, X);

    return 0;
}

int LuaBuildModeLib::LuaBuildModeLongGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Long_Get called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    int val = -1;

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    val = buildModeMain->GetInt(clientId, index);

    lua_pushinteger(L, val);
    return 1;
}

int LuaBuildModeLib::LuaBuildModeFloatSet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_float_Set called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    float X = luaL_checknumber(L, 3);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetFloat(clientId, index, X);
    return 0;
}

int LuaBuildModeLib::LuaBuildModeFloatGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Float_Get called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    float val = -1;

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    val = buildModeMain->GetFloat(clientId, index);

    lua_pushnumber(L, val);
    return 1;
}

int LuaBuildModeLib::LuaBuildModeStringSet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_String_Set called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    std::string val(luaL_checkstring(L, 3));

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetString(clientId, index, val);

    return 0;
}

int LuaBuildModeLib::LuaBuildModeStringGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_String_Get called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int clientId = luaL_checkinteger(L, 1);
    int index = luaL_checkinteger(L, 2);
    std::string val;

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    val = buildModeMain->GetString(clientId, index);

    lua_pushstring(L, val.c_str());
    return 1;
}
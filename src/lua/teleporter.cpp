#include "lua/teleporter.h"

#include <lua.hpp>
#include "common/Logger.h"
#include "common/MinecraftLocation.h"
#include "world/Map.h"
#include "world/Teleporter.h"
#include "network/NetworkClient.h"

using namespace D3PP::world;
using namespace D3PP::Common;

const struct luaL_Reg LuaTeleporterLib::lib[] = {
        {"getall", &LuaTeleporterGetTable},
        {"add", &LuaTeleporterAdd},
        {"delete", &LuaTeleporterDelete},
        {"getlocation", &LuaTeleporterGetBox},
        {"getdestination", &LuaTeleporterGetDestination},
        {NULL, NULL}
};

int LuaTeleporterLib::openLib(lua_State* L)
{
    lua_getglobal(L, "Teleporter");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaTeleporterLib::lib, 0);
    lua_setglobal(L, "Teleporter");
    return 1;
}

int LuaTeleporterLib::LuaTeleporterGetTable(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Teleporter_Get_Table called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> chosenMap = mm->GetPointer(mapId);

    if (chosenMap == nullptr)
        return 0;

    int numPortals = static_cast<int>(chosenMap->Portals.size());
    int index = 1;

    lua_newtable(L);

    if (numPortals > 0) {
        for (auto const& nc : chosenMap->Portals) {
            lua_pushinteger(L, index++);
            lua_pushstring(L, nc.Name.c_str());
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numPortals);

    return 2;
}

int LuaTeleporterLib::LuaTeleporterGetBox(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Teleporter_Get_Box called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);
    std::string tpId(luaL_checkstring(L, 2));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> chosenMap = mm->GetPointer(mapId);

    if (chosenMap == nullptr)
        return 0;

    Teleporter item = chosenMap->GetTeleporter(tpId);

    if (item.Name.empty())
        return 0;

    Vector3S startPos = item.OriginStart.GetAsBlockCoords();
    Vector3S endPos = item.OriginEnd.GetAsBlockCoords();

    lua_pushinteger(L, startPos.X);
    lua_pushinteger(L, startPos.Y);
    lua_pushinteger(L, startPos.Z);
    lua_pushinteger(L, endPos.X);
    lua_pushinteger(L, endPos.Y);
    lua_pushinteger(L, endPos.Z);
    return 6;
}

int LuaTeleporterLib::LuaTeleporterGetDestination(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Teleporter_Get_Destination called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);
    std::string tpId(luaL_checkstring(L, 2));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> chosenMap = mm->GetPointer(mapId);

    if (chosenMap == nullptr)
        return 0;

    Teleporter item = chosenMap->GetTeleporter(tpId);

    if (item.Name.empty())
        return 0;

    Vector3S DestPos = item.Destination.GetAsBlockCoords();

    lua_pushstring(L, item.DestinationMap.c_str());
    lua_pushinteger(L, -1);
    lua_pushnumber(L, DestPos.X);
    lua_pushnumber(L, DestPos.Y);
    lua_pushnumber(L, DestPos.Z);
    lua_pushnumber(L, item.Destination.Rotation);
    lua_pushnumber(L, item.Destination.Look);
    return 7;
}

int LuaTeleporterLib::LuaTeleporterAdd(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 15) {
        Logger::LogAdd("Lua", "LuaError: Teleporter_Add called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);
    std::string tpId(luaL_checkstring(L, 2));
    Vector3S start{};
    Vector3S end{};
    start.X = luaL_checkinteger(L, 3);
    start.Y = luaL_checkinteger(L, 4);
    start.Z = luaL_checkinteger(L, 5);

    end.X = luaL_checkinteger(L, 6);
    end.Y = luaL_checkinteger(L, 7);
    end.Z = luaL_checkinteger(L, 8);

    std::string destMapUniqueId(luaL_checkstring(L, 9));
    int destMapId = luaL_checkinteger(L, 10);
    float DestX = luaL_checknumber(L, 11);
    float DestY = luaL_checknumber(L, 12);
    float DestZ = luaL_checknumber(L, 13);
    float DestRot = luaL_checknumber(L, 14);
    float DestLook = luaL_checknumber(L, 15);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> chosenMap = mm->GetPointer(mapId);

    if (chosenMap == nullptr)
        return 0;

    MinecraftLocation oStart{};
    MinecraftLocation oEnd{};
    oStart.SetAsBlockCoords(start);
    oEnd.SetAsBlockCoords(end);

    MinecraftLocation destLocation {DestRot, DestLook};
    Vector3F destFloats {DestX, DestY, DestZ};
    destLocation.SetAsPlayerCoords(destFloats);

    chosenMap->AddTeleporter(tpId, oStart, oEnd, destLocation, destMapUniqueId, destMapId);
    return 0;
}

int LuaTeleporterLib::LuaTeleporterDelete(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Teleporter_Delete called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    std::string tpId(luaL_checkstring(L, 2));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> chosenMap = mm->GetPointer(mapId);

    if (chosenMap == nullptr)
        return 0;


    chosenMap->DeleteTeleporter(tpId);
    return 0;
}

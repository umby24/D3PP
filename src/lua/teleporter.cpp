#include "lua/teleporter.h"

#include <lua.hpp>
#include "common/Logger.h"
#include "Utils.h"
#include "world/Map.h"
#include "world/Teleporter.h"
#include "network/NetworkClient.h"

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

    int numRanks = static_cast<int>(chosenMap->data.Teleporter.size());
    int index = 1;

    lua_newtable(L);

    if (numRanks > 0) {
        for (auto const& nc : chosenMap->data.Teleporter) {
            lua_pushinteger(L, index++);
            lua_pushstring(L, nc.second.Id.c_str());
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numRanks);

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

    if (chosenMap->data.Teleporter.find(tpId) == chosenMap->data.Teleporter.end())
        return 0;
    
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].X0);
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].Y0);
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].Z0);
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].X1);
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].Y1);
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].Z1);
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

    if (chosenMap->data.Teleporter.find(tpId) == chosenMap->data.Teleporter.end())
        return 0;

    lua_pushstring(L, chosenMap->data.Teleporter[tpId].DestMapUniqueId.c_str());
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].DestMapId);
    lua_pushnumber(L, chosenMap->data.Teleporter[tpId].DestX);
    lua_pushnumber(L, chosenMap->data.Teleporter[tpId].DestY);
    lua_pushnumber(L, chosenMap->data.Teleporter[tpId].DestZ);
    lua_pushnumber(L, chosenMap->data.Teleporter[tpId].DestRot);
    lua_pushnumber(L, chosenMap->data.Teleporter[tpId].DestLook);
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

    if (chosenMap->data.Teleporter.find(tpId) != chosenMap->data.Teleporter.end())
        return 0;
    Teleporter::AddTeleporter(chosenMap, tpId, start.X, end.X, start.Y, end.Y, start.Z, end.Z, destMapUniqueId, destMapId, DestX, DestY, DestZ, DestRot, DestLook);
    //MapTeleporterElement newTp{
    //    tpId,
    //    start.X,
    //    start.Y,
    //    start.Z,
    //    end.X,
    //    end.Y,
    //    end.Z,
    //    destMapUniqueId,
    //    destMapId,
    //    DestX,
    //    DestY,
    //    DestZ,
    //    DestRot,
    //    DestLook
    //};

    //chosenMap->data.Teleporter.insert(std::make_pair(tpId, newTp));

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

    if (chosenMap->data.Teleporter.find(tpId) == chosenMap->data.Teleporter.end())
        return 0;

    chosenMap->data.Teleporter.erase(tpId);
    return 0;
}

#include "lua/map.h"

#include <lua.hpp>
#include "world/Map.h"
#include "Utils.h"
#include "common/Logger.h"
#include "common/Player_List.h"
#include "network/NetworkClient.h"
#include "network/Network.h"

const struct luaL_Reg LuaMapLib::lib[] = {
        {"getall", &LuaMapGetTable},
        {"setblock", &LuaMapBlockChange},
        {"setblockclient", &LuaMapBlockChangeClient},
        {"setblockplayer", &LuaMapBlockChangePlayer},
        {"moveblock", &LuaMapBlockMove},
        {"getblock", &LuaMapBlockGetType},
        {"getrank", &LuaMapBlockGetRank},
        {"getplayer", &LuaMapBlockGetPlayer},
        {"name", &LuaMapGetName},
        {"uuid", &LuaMapGetUniqueId},
        {"directory", &LuaMapGetDirectory},
        {"buildrank", &LuaMapGetRankBuild},
        {"showrank", &LuaMapGetRankShow},
        {"joinrank", &LuaMapGetRankJoin},
        {"size", &LuaMapGetDimensions},
        {"spawn", &LuaMapGetSpawn},
        {"saveinterval", &LuaMapGetSaveInterval},
        {"setname", &LuaMapSetName},
        {"setdirectory", &LuaMapSetDirectory},
        {"setbuildrank", &LuaMapSetRankBuild},
        {"setjoinrank", &LuaMapSetRankJoin},
        {"setshowrank", &LuaMapSetRankShow},
        {"setspawn", &LuaMapSetSpawn},
        {"setsaveinterval", &LuaMapSetSaveInterval},
        {"add", &LuaMapAdd},
        {"resize", &LuaMapActionAddResize},
        {"save", &LuaMapActionAddSave},
        {"fill", &LuaMapActionAddFill},
        {"delete", &LuaMapActionAddDelete},
        {"resend", &LuaMapResend},
        {"export", &LuaMapExport},
        {"import", &LuaMapImportPlayer},
        {"exportsize", &LuaMapExportGetSize},
        {NULL, NULL}
};

int LuaMapLib::openLib(lua_State* L)
{
    lua_getglobal(L, "Map");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaMapLib::lib, 0);
    lua_setglobal(L, "Map");
    return 1;
}

int LuaMapLib::LuaMapBlockMove(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 10) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Move called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);
    int x0 = static_cast<int>(luaL_checknumber(L, 2));
    int y0 = static_cast<int>(luaL_checknumber(L, 3));
    int z0 = static_cast<int>(luaL_checknumber(L, 4));
    int x1 = static_cast<int>(luaL_checknumber(L, 5));
    int y1 = static_cast<int>(luaL_checknumber(L, 6));
    int z1 = static_cast<int>(luaL_checknumber(L, 7));
    bool undo = lua_toboolean(L, 8);
    bool physic = lua_toboolean(L, 9);
    unsigned char priority = luaL_checkinteger(L, 10);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        map->BlockMove(x0, y0, z0, x1, y1, z1, undo, physic, priority);
    }

    return 0;
}

int LuaMapLib::LuaMapGetName(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Name called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        lua_pushstring(L, map->data.Name.c_str());
        return 1;
    }

    return 0;
}

int LuaMapLib::LuaMapGetDimensions(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Dimensions called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        lua_pushinteger(L, map->data.SizeX);
        lua_pushinteger(L, map->data.SizeY);
        lua_pushinteger(L, map->data.SizeZ);
        return 3;
    }

    return 0;
}

int LuaMapLib::LuaMapGetTable(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 0) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Table() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    MapMain* mm = MapMain::GetInstance();
    int numEntities = static_cast<int>(mm->_maps.size());
    int index = 1;

    lua_newtable(L);

    if (numEntities > 0) {
        for (auto const& e : mm->_maps) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, e.second->data.ID);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numEntities);

    return 2;
}

int LuaMapLib::LuaMapBlockChangePlayer(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 10) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Change_Client called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerId = luaL_checkinteger(L, 1);
    int mapId = luaL_checkinteger(L, 2);
    int X = static_cast<int>(luaL_checknumber(L, 3));
    int Y = static_cast<int>(luaL_checknumber(L, 4));
    int Z = static_cast<int>(luaL_checknumber(L, 5));
    unsigned char type = luaL_checkinteger(L, 6);
    bool undo = lua_toboolean(L, 7);
    bool physic = lua_toboolean(L, 8);
    bool send = lua_toboolean(L, 9);
    unsigned char priority = luaL_checkinteger(L, 10);

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* pEntry = pll->GetPointer(playerId);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (pEntry == nullptr || map == nullptr) {
        return 0;
    }

    map->BlockChange(static_cast<short>(playerId), X, Y, Z, type, undo, physic, send, priority);
    return 0;
}

int LuaMapLib::LuaMapBlockGetRank(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Get_Rank called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    int X = luaL_checkinteger(L, 2);
    int Y = luaL_checkinteger(L, 3);
    int Z = luaL_checkinteger(L, 4);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    int result = map->BlockGetRank(X, Y, Z);
    lua_pushinteger(L, result);

    return 1;
}

int LuaMapLib::LuaMapGetUniqueId(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Unique_Id called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushstring(L, map->data.UniqueID.c_str());
    return 1;
}

int LuaMapLib::LuaMapGetDirectory(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Directory called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushstring(L, map->data.Directory.c_str());
    return 1;
}

int LuaMapLib::LuaMapGetRankBuild(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Rank_Build called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushinteger(L, map->data.RankBuild);
    return 1;
}

int LuaMapLib::LuaMapGetRankShow(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Rank_Show called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushinteger(L, map->data.RankShow);
    return 1;
}

int LuaMapLib::LuaMapGetRankJoin(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Rank_Join called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushinteger(L, map->data.RankJoin);
    return 1;
}

int LuaMapLib::LuaMapGetSpawn(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Spawn called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushnumber(L, map->data.SpawnX);
    lua_pushnumber(L, map->data.SpawnY);
    lua_pushnumber(L, map->data.SpawnZ);
    lua_pushnumber(L, map->data.SpawnRot);
    lua_pushnumber(L, map->data.SpawnLook);
    return 5;
}

int LuaMapLib::LuaMapGetSaveInterval(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Save_Intervall called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushinteger(L, map->data.SaveInterval);
    return 1;
}

int LuaMapLib::LuaMapSetName(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Name called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    std::string newName(luaL_checkstring(L, 2));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.Name = newName;
    return 0;
}

int LuaMapLib::LuaMapSetDirectory(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Directory called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    std::string newName(luaL_checkstring(L, 2));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.Directory = newName;
    return 0;
}

int LuaMapLib::LuaMapSetRankBuild(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Rank_Build called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    short newRank = luaL_checkinteger(L, 2);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.RankBuild = newRank;
    return 0;
}

int LuaMapLib::LuaMapSetRankJoin(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Rank_Join called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    short newRank = luaL_checkinteger(L, 2);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.RankJoin = newRank;
    return 0;
}

int LuaMapLib::LuaMapSetRankShow(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Rank_Show called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    short newRank = luaL_checkinteger(L, 2);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.RankShow = newRank;
    return 0;
}

int LuaMapLib::LuaMapSetSpawn(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 6) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Spawn called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    float newX = luaL_checknumber(L, 2);
    float newY = luaL_checknumber(L, 3);
    float newZ = luaL_checknumber(L, 4);
    float newRot = luaL_checknumber(L, 5);
    float newLook = luaL_checknumber(L, 6);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.SpawnX = newX;
    map->data.SpawnY = newY;
    map->data.SpawnZ = newZ;
    map->data.SpawnRot = newRot;
    map->data.SpawnLook = newLook;

    return 0;
}

int LuaMapLib::LuaMapSetSaveInterval(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Save_Interval called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    int saveInterval = luaL_checkinteger(L, 2);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.SaveInterval = saveInterval;
    return 0;
}

int LuaMapLib::LuaMapAdd(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 5) {
        Logger::LogAdd("Lua", "LuaError: Map_Add called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    short sizeX = luaL_checkinteger(L, 2);
    short sizeY = luaL_checkinteger(L, 3);
    short sizeZ = luaL_checkinteger(L, 4);
    std::string name(luaL_checkstring(L, 5));
    MapMain* mm = MapMain::GetInstance();

    int resultId = mm->Add(mapId, sizeX, sizeY, sizeZ, name);
    lua_pushinteger(L, resultId);
    return 1;
}

int LuaMapLib::LuaMapActionAddResize(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map_Action_Add_Resize called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    int sizeX = luaL_checkinteger(L, 2);
    int sizeY = luaL_checkinteger(L, 3);
    int sizeZ = luaL_checkinteger(L, 4);
    MapMain* mm = MapMain::GetInstance();

    mm->AddResizeAction(0, mapId, sizeX, sizeY, sizeZ);

    return 0;
}

int LuaMapLib::LuaMapActionAddFill(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Map_Action_Add_Fill called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);
    std::string functionName(luaL_checkstring(L, 2));
    std::string argumentString(luaL_checkstring(L, 3));
    MapMain* mm = MapMain::GetInstance();

    mm->AddFillAction(0, mapId, functionName, argumentString);
    return 0;
}

int LuaMapLib::LuaMapActionAddSave(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Action_Add_Save called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);
    std::string directory(luaL_checkstring(L, 2));
    MapMain* mm = MapMain::GetInstance();

    mm->AddSaveAction(0, mapId, directory);
    return 0;
}

int LuaMapLib::LuaMapActionAddDelete(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Action_Add_Delete called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    mm->AddDeleteAction(0, mapId);

    return 0;
}

int LuaMapLib::LuaMapResend(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Resend called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> givenMap = mm->GetPointer(mapId);

    if (givenMap != nullptr) {
        givenMap->Resend();
    }

    return 0;
}

int LuaMapLib::LuaMapExport(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 8) {
        Logger::LogAdd("Lua", "LuaError: Map_Export called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    Vector3S startLoc{};
    Vector3S endLoc{};
    int mapId = luaL_checkinteger(L, 1);
    startLoc.X = luaL_checkinteger(L, 2);
    startLoc.Y = luaL_checkinteger(L, 3);
    startLoc.Z = luaL_checkinteger(L, 4);
    endLoc.X = luaL_checkinteger(L, 5);
    endLoc.Y = luaL_checkinteger(L, 6);
    endLoc.Z = luaL_checkinteger(L, 7);
    std::string fileName(luaL_checkstring(L, 8));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> givenMap = mm->GetPointer(mapId);

    MinecraftLocation start{};
    start.SetAsBlockCoords(startLoc);

    MinecraftLocation end{};
    end.SetAsBlockCoords(endLoc);

    if (givenMap != nullptr) {
        givenMap->MapExport(start, end, fileName);
    }
    return 0;
}

int LuaMapLib::LuaMapExportGetSize(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Export_Get_Size called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    std::string filename(luaL_checkstring(L, 1));
    Vector3S result = MapMain::GetMapExportSize(filename);

    lua_pushinteger(L, result.X);
    lua_pushinteger(L, result.Y);
    lua_pushinteger(L, result.Z);
    return 3;
}



int LuaMapLib::LuaMapImportPlayer(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 9) {
        Logger::LogAdd("Lua", "LuaError: Map_Import called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int playerNumber = luaL_checkinteger(L, 1);
    std::string fileName(luaL_checkstring(L, 2));
    int mapId = luaL_checkinteger(L, 3);
    Vector3S placeLoc{};
    placeLoc.X = luaL_checkinteger(L, 4);
    placeLoc.Y = luaL_checkinteger(L, 5);
    placeLoc.Z = luaL_checkinteger(L, 6);
    short scaleX = luaL_checkinteger(L, 7);
    short scaleY = luaL_checkinteger(L, 8);
    short scaleZ = luaL_checkinteger(L, 9);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> givenMap = mm->GetPointer(mapId);

    MinecraftLocation start{};
    start.SetAsBlockCoords(placeLoc);

    if (givenMap != nullptr) {
        givenMap->MapImport(fileName, start, scaleX, scaleY, scaleZ);
    }

    return 0;
}

int LuaMapLib::LuaMapBlockGetPlayer(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Get_Player_Last called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    int X = static_cast<int>(luaL_checknumber(L, 2));
    int Y = static_cast<int>(luaL_checknumber(L, 3));
    int Z = static_cast<int>(luaL_checknumber(L, 4));

    int result = -1;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        result = map->GetBlockPlayer(X, Y, Z);
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaMapLib::LuaMapBlockChange(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 10) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Change called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    int mapId = luaL_checkinteger(L, 2);
    int X = static_cast<int>(luaL_checknumber(L, 3));
    int Y = static_cast<int>(luaL_checknumber(L, 4));
    int Z = static_cast<int>(luaL_checknumber(L, 5));
    unsigned char type = luaL_checkinteger(L, 6);
    bool Undo = (luaL_checkinteger(L, 7) > 0);
    bool physics = (luaL_checkinteger(L, 8) > 0);
    bool send = (luaL_checkinteger(L, 9) > 0);
    unsigned char priority = luaL_checkinteger(L, 10);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        map->BlockChange(static_cast<short>(playerNumber), X, Y, Z, type, Undo, physics, send, priority);
    }

    return 0;
}

int LuaMapLib::LuaMapBlockChangeClient(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Change_Client called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    int mapId = luaL_checkinteger(L, 2);
    int X = static_cast<int>(luaL_checknumber(L, 3));
    int Y = static_cast<int>(luaL_checknumber(L, 4));
    int Z = static_cast<int>(luaL_checknumber(L, 5));
    unsigned char mode = luaL_checkinteger(L, 6);
    unsigned char type = luaL_checkinteger(L, 7);
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (client == nullptr || map == nullptr) {
        return 0;
    }

    map->BlockChange(client, X, Y, Z, mode, type);
    return 0;
}


int LuaMapLib::LuaMapBlockGetType(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Get_Type called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    int X = static_cast<int>(luaL_checknumber(L, 2));
    int Y = static_cast<int>(luaL_checknumber(L, 3));
    int Z = static_cast<int>(luaL_checknumber(L, 4));

    int result = -1;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        result = map->GetBlockType(X, Y, Z);
    }

    lua_pushinteger(L, result);
    return 1;
}
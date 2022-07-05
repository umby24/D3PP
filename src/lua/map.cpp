#include "lua/map.h"

#include <lua.hpp>
#include <network/Network_Functions.h>
#include "world/Map.h"
#include "world/MapMain.h"
#include "Utils.h"
#include "common/Logger.h"
#include "common/Player_List.h"
#include "network/NetworkClient.h"
#include "network/Network.h"
#include "generation/flatgrass.cpp"
#include "world/CustomParticle.h"
#include "network/packets/SpawnEffectPacket.h"
#include "CPE.h"

using namespace D3PP::world;
using namespace D3PP::Common;

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
        {"load", &LuaMapLoad},
        {"resize", &LuaMapActionAddResize},
        {"save", &LuaMapActionAddSave},
        {"fill", &LuaMapActionAddFill},
        {"delete", &LuaMapActionAddDelete},
        {"resend", &LuaMapResend},
        {"export", &LuaMapExport},
        {"import", &LuaMapImportPlayer},
        {"exportsize", &LuaMapExportGetSize},
        {"beginfill", &LuaBeginFill},
        {"getfillblock", &LuaGetFillBlock},
        {"setfillblock", &LuaSetFillBlock},
        {"endfill", &LuaEndFill},
        {"fillflat", &LuaFillFlat},
        {"createParticle", &LuaCreateParticle},
        {"deleteParticle", &LuaDeleteParticle},
        {"spawnParticle", &LuaSpawnParticle},
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

int LuaMapLib::LuaFillFlat(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: FillFlat called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int mapId = luaL_checkinteger(L, 1);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        GenTools::FlatgrassGen(mapId);
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
        lua_pushstring(L, map->Name().c_str());
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

    if (map != nullptr && map->loading == false && map->loaded) {
        Vector3S mapSize = map->GetSize();
        lua_pushinteger(L, mapSize.X);
        lua_pushinteger(L, mapSize.Y);
        lua_pushinteger(L, mapSize.Z);
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
            lua_pushinteger(L, e.second->ID);
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
    auto pEntry = pll->GetPointer(playerId);

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

    lua_pushstring(L, "Deprecated");
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

    lua_pushstring(L, map->filePath.c_str());
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
    MapPermissions perms = map->GetMapPermissions();
    lua_pushinteger(L, perms.RankBuild);
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
    MapPermissions perms = map->GetMapPermissions();
    lua_pushinteger(L, perms.RankShow);
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
    MapPermissions perms = map->GetMapPermissions();
    lua_pushinteger(L, perms.RankJoin);
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
    MinecraftLocation mapSpawn = map->GetSpawn();
    lua_pushnumber(L, mapSpawn.X());
    lua_pushnumber(L, mapSpawn.Y());
    lua_pushnumber(L, mapSpawn.Z());
    lua_pushnumber(L, mapSpawn.Rotation);
    lua_pushnumber(L, mapSpawn.Look);
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

    lua_pushinteger(L, 10);
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

    //map->data.Name = newName;
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

    map->filePath = newName;
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
    MapPermissions perms = map->GetMapPermissions();
    perms.RankBuild = newRank;
    map->SetMapPermissions(perms);
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
    MapPermissions perms = map->GetMapPermissions();
    perms.RankJoin = newRank;
    map->SetMapPermissions(perms);
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
    MapPermissions perms = map->GetMapPermissions();
    perms.RankShow = newRank;
    map->SetMapPermissions(perms);

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

    MinecraftLocation newSpawn{newRot, newLook};
    Vector3F spawnPos{newX, newY, newZ};
    newSpawn.SetAsPlayerCoords(spawnPos);
    map->SetSpawn(newSpawn);

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

int LuaMapLib::LuaMapLoad(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Action_Add_Resize called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    std::string mapDir(luaL_checkstring(L, 2));

    MapMain* mm = MapMain::GetInstance();

    mm->AddLoadAction(0, mapId, mapDir);

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

int LuaMapLib::LuaBeginFill(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map.BeginFill called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = static_cast<int>(luaL_checkinteger(L, 1));

    int result = -1;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        if (map->CurrentFillState != nullptr) {
            result = 0;
        } else {
            map->CurrentFillState = std::make_unique<FillState>(map->GetSize());
            result = 1;
        }
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaMapLib::LuaSetFillBlock(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 5) {
        Logger::LogAdd("Lua", "LuaError: Map.SetFillBlock called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = static_cast<int>(luaL_checkinteger(L, 1));
    int X = static_cast<int>(luaL_checkinteger(L, 2));
    int Y = static_cast<int>(luaL_checkinteger(L, 3));
    int Z = static_cast<int>(luaL_checkinteger(L, 4));
    int Type = static_cast<int>(luaL_checkinteger(L, 5));

    int result = -1;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        if (map->CurrentFillState == nullptr) {
            result = 0;
        } else {
            map->CurrentFillState->SetBlock(Vector3S((short)X, Y, Z), (unsigned char)Type);
            result = 1;
        }
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaMapLib::LuaEndFill(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map.EndFill called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = static_cast<int>(luaL_checkinteger(L, 1));

    int result = -1;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        if (map->CurrentFillState == nullptr) {
            result = 0;
        } else {
            map->SetBlocks(map->CurrentFillState->fillData);
            map->CurrentFillState.reset();
            result = 1;
        }
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaMapLib::LuaGetFillBlock(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map.GetFillBlock called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = static_cast<int>(luaL_checkinteger(L, 1));
    int X = static_cast<int>(luaL_checkinteger(L, 2));
    int Y = static_cast<int>(luaL_checkinteger(L, 3));
    int Z = static_cast<int>(luaL_checkinteger(L, 4));

    int result = -1;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        if (map->CurrentFillState == nullptr) {
            result = 0;
        } else {
            result = map->CurrentFillState->GetBlock(Vector3S((short)X, Y, Z));
        }
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaMapLib::LuaCreateParticle(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 20) {
        Logger::LogAdd("Lua", "LuaError: Map.createParticle called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    CustomParticle cp{};

    int mapId = static_cast<int>(luaL_checkinteger(L, 1));
    cp.effectId = luaL_checkinteger(L, 2);
    cp.U1 = luaL_checkinteger(L, 3);
    cp.V1 = luaL_checkinteger(L, 4);
    cp.U2 = luaL_checkinteger(L, 5);
    cp.V2 = luaL_checkinteger(L, 6);
    cp.redTint = luaL_checkinteger(L, 7);
    cp.blueTint = luaL_checkinteger(L, 8);
    cp.greenTint = luaL_checkinteger(L, 9);
    cp.frameCount = luaL_checkinteger(L, 10);
    cp.particleCount = luaL_checkinteger(L, 11);
    cp.size = luaL_checkinteger(L, 12);
    cp.sizeVariation = luaL_checkinteger(L, 13);
    cp.spread = luaL_checkinteger(L, 14);
    cp.speed = luaL_checkinteger(L, 15);
    cp.gravity = luaL_checkinteger(L, 16);
    cp.baseLifetime = luaL_checkinteger(L, 17);
    cp.lifetimeVariation = luaL_checkinteger(L, 18);
    cp.collideFlags = luaL_checkinteger(L, 19);
    cp.fullBright = luaL_checkinteger(L, 20);

    bool result = false;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        map->AddParticle(cp);
        result = true;
    }

    lua_pushboolean(L, result);
    return 1;
}

int LuaMapLib::LuaDeleteParticle(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map.deleteParticle called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = static_cast<int>(luaL_checkinteger(L, 1));
    int particleId = luaL_checkinteger(L, 2);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        map->DeleteParticle(particleId);
    }

    return 0;
}

int LuaMapLib::LuaSpawnParticle(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 8) {
        Logger::LogAdd("Lua", "LuaError: Map.spawnParticle called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = static_cast<int>(luaL_checkinteger(L, 1));
    int particleId = luaL_checkinteger(L, 2);
    int originX = luaL_checkinteger(L, 3);
    int originY = luaL_checkinteger(L, 4);
    int originZ = luaL_checkinteger(L, 5);

    int positionX = luaL_checkinteger(L, 6);
    int positionY = luaL_checkinteger(L, 7);
    int positionZ = luaL_checkinteger(L, 8);

    D3PP::network::SpawnEffectPacket spp;
    spp.effectId = particleId;
    spp.originX = originX;
    spp.originY = originY;
    spp.originZ = originZ;
    spp.positionX = positionX;
    spp.positionY = positionY;
    spp.positionZ = positionZ;
    NetworkFunctions::PacketToMap(mapId, spp, CUSTOM_PARTICLES_EXT_NAME, 1);

    return 0;
}
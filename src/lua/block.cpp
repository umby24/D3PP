#include "lua/block.h"

#include <lua.hpp>
#include "common/Logger.h"
#include "Utils.h"
#include "Block.h"

const struct luaL_Reg LuaBlockLib::lib[] = {
        {"getall", &LuaBlockGetTable},
        {"name", &LuaBlockGetName},
        {"placerank", &LuaBlockGetRankPlace},
        {"deleterank", &LuaBlockGetRankDelete},
        {"setplacerank", &LuaBlockSetRankPlace},
        {"setdeleterank", &LuaBlockSetRankDelete},
        {"setphysics", &LuaBlockSetPhysics},
        {"setkills", &LuaBlockSetKills},
        {"clienttype", &LuaBlockGetClientType},
        {"create", &LuaBlockCreate},
        {NULL, NULL}
};

int LuaBlockLib::openLib(lua_State* L)
{
    lua_getglobal(L, "Block");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaBlockLib::lib, 0);
    lua_setglobal(L, "Block");
    return 1;
}

int LuaBlockLib::LuaBlockGetTable(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 0) {
        Logger::LogAdd("Lua", "LuaError: BlocK_Get_Table() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    Block* bm = Block::GetInstance();
    int numEntities = static_cast<int>(bm->Blocks.size());
    int index = 1;

    lua_newtable(L);

    if (numEntities > 0) {
        for (auto const& e : bm->Blocks) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, e.Id);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numEntities);

    return 2;
}

int LuaBlockLib::LuaBlockGetName(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Block_Get_Name() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);
    lua_pushstring(L, be.Name.c_str());

    return 1;
}

int LuaBlockLib::LuaBlockGetRankPlace(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Block_Get_Rank_Place() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);
    lua_pushinteger(L, be.RankPlace);

    return 1;
}

int LuaBlockLib::LuaBlockGetRankDelete(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Block_Get_Rank_Delete() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);
    lua_pushinteger(L, be.RankDelete);

    return 1;
}

int LuaBlockLib::LuaBlockGetClientType(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Block_Get_Client_Type() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);
    lua_pushinteger(L, be.OnClient);

    return 1;
}

int LuaBlockLib::LuaBlockCreate(lua_State* L)
{
    int nArgs = lua_gettop(L);

    if (nArgs < 2)
    {
        Logger::LogAdd("Lua", "LuaError: Block.Create invalid number of arguments", LogType::L_ERROR, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    if (blockId < 66) {
        Logger::LogAdd("Lua", "LuaError: Please don't redefine the blocks under #66!", LogType::L_ERROR, GLF);
        return 0;
    }

    std::string blockName(luaL_checkstring(L, 2));
    int clientId = luaL_checkinteger(L, 3);
    // -- opt: physics, physics plugin, phystime, physRandom.
    int physics = luaL_optinteger(L, 4, 0);
    std::string physicsPlugin(luaL_optstring(L, 5, ""));
    int physicsTime = luaL_optinteger(L, 6, 0);
    int physicsRandom = luaL_optinteger(L, 7, 0);

    MapBlock newBlock{ blockId, blockName, clientId, physics, physicsPlugin, physicsTime, physicsRandom, false, false, "", "", 0, 0, 0, 0, false, false, 0, 0, 0};
    Block* bm = Block::GetInstance();
    bm->Blocks[blockId] = newBlock;
    bm->SaveFile = true;

    return 0;
}

int LuaBlockLib::LuaBlockSetRankPlace(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Block_Set_Rank_Place() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    int setRank = luaL_checkinteger(L, 2);

    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);

    be.RankPlace = setRank;
    bm->Blocks[be.Id] = be;
    bm->SaveFile = true;
    return 0;
}

int LuaBlockLib::LuaBlockSetRankDelete(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Block_Set_Rank_Delete() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    int setRank = luaL_checkinteger(L, 2);

    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);

    be.RankDelete = setRank;
    bm->Blocks[be.Id] = be;
    bm->SaveFile = true;
    return 0;
}

int LuaBlockLib::LuaBlockSetKills(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Block_Set_Kills() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    bool kills = luaL_checkinteger(L, 2) > 0;
    
    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);

    be.Kills = kills;
    bm->Blocks[be.Id] = be;
    bm->SaveFile = true;
    return 0;
}

int LuaBlockLib::LuaBlockSetPhysics(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: Block_Set_Physics() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    int physics = luaL_checkinteger(L, 2);
    bool onLoad = luaL_checkinteger(L, 3) > 0;
    std::string physPlugin(luaL_checkstring(L, 4));
    int physRandom = luaL_checkinteger(L, 5);
    bool repeat = luaL_checkinteger(L, 6);
    int physTime = luaL_checkinteger(L, 7);
    // -- physics, onload, plugin, random, repeat, time,
    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);

    be.Physics = physics;
    be.PhysicsOnLoad = onLoad;
    be.PhysicsPlugin = physPlugin;
    be.PhysicsRandom = physRandom;
    be.PhysicsRepeat = repeat;
    be.PhysicsTime = physTime;

    bm->Blocks[be.Id] = be;
    bm->SaveFile = true;
    return 0;
}
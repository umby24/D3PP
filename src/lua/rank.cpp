#include "lua/rank.h"

#include <lua.hpp>
#include "common/Logger.h"
#include "Utils.h"
#include "Rank.h"

const struct luaL_Reg LuaRankLib::lib[] = {
        {"getall", &LuaRankGetTable},
        {"add", &LuaRankAdd},
        {"delete", &LuaRankDelete},
        {"getname", &LuaRankGetName},
        {"getprefix", &LuaRankGetPrefix},
        {"getsuffix", &LuaRankGetSuffix},
        {"getroot", &LuaRankGetRoot},
        {NULL, NULL}
};

int LuaRankLib::openLib(lua_State* L)
{
    lua_getglobal(L, "Rank");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaRankLib::lib, 0);
    lua_setglobal(L, "Rank");
    return 1;
}

int LuaRankLib::LuaRankGetTable(lua_State* L) {
    Rank* rm = Rank::GetInstance();
    int numRanks = static_cast<int>(rm->m_ranks.size());
    int index = 1;

    lua_newtable(L);

    if (numRanks > 0) {
        for (auto const& nc : rm->m_ranks) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, nc.first);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numRanks);

    return 2;
}




int LuaRankLib::LuaRankAdd(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Rank_Add called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int rankNumber = luaL_checkinteger(L, 1);
    std::string rankName(luaL_checkstring(L, 2));
    std::string rankPrefix(luaL_checkstring(L, 3));
    std::string rankSuffix(luaL_checkstring(L, 4));
    RankItem newRankItem;
    newRankItem.Name = rankName;
    newRankItem.Rank = rankNumber;
    newRankItem.Prefix = rankPrefix;
    newRankItem.Suffix = rankSuffix;

    Rank* r = Rank::GetInstance();
    r->Add(newRankItem);
    return 0;
}

int LuaRankLib::LuaRankDelete(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Rank_Delete called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int rankNumber = luaL_checkinteger(L, 1);
    int isExact = luaL_checkinteger(L, 2);

    Rank* r = Rank::GetInstance();
    r->Delete(rankNumber, isExact > 0);

    return 0;
}

int LuaRankLib::LuaRankGetName(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Rank_Get_Name called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int rankNumber = luaL_checkinteger(L, 1);
    int isExact = luaL_checkinteger(L, 2);

    Rank* r = Rank::GetInstance();
    RankItem ri = r->GetRank(rankNumber, isExact > 0);
    lua_pushstring(L, ri.Name.c_str());

    return 1;
}

int LuaRankLib::LuaRankGetPrefix(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Rank_Get_Prefix called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int rankNumber = luaL_checkinteger(L, 1);
    int isExact = luaL_checkinteger(L, 2);

    Rank* r = Rank::GetInstance();
    RankItem ri = r->GetRank(rankNumber, isExact > 0);
    lua_pushstring(L, ri.Prefix.c_str());

    return 1;
}

int LuaRankLib::LuaRankGetSuffix(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Rank_Get_Suffix called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int rankNumber = luaL_checkinteger(L, 1);
    int isExact = luaL_checkinteger(L, 2);

    Rank* r = Rank::GetInstance();
    RankItem ri = r->GetRank(rankNumber, isExact > 0);
    lua_pushstring(L, ri.Suffix.c_str());

    return 1;
}

int LuaRankLib::LuaRankGetRoot(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Rank_Get_Name called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int rankNumber = luaL_checkinteger(L, 1);
    int isExact = luaL_checkinteger(L, 2);

    Rank* r = Rank::GetInstance();
    RankItem ri = r->GetRank(rankNumber, isExact > 0);
    lua_pushinteger(L, ri.Rank);

    return 1;
}

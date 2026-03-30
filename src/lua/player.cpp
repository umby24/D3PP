#include "lua/player.h"

#include <lua.hpp>
#include "common/Player_List.h"
#include "Utils.h"
#include "common/Logger.h"
#include "Rank.h"

const struct luaL_Reg LuaPlayerLib::lib[] = {
        {"getall", &LuaPlayerGetTable},
        {"getprefix", &LuaPlayerGetPrefix},
        {"getname", &LuaPlayerGetName},
        {"getsuffix", &LuaPlayerGetSuffix},
        {"ip", &LuaPlayerGetIp},
        {"rank", &LuaPlayerGetRank},
        {"online", &LuaPlayerGetOnline},
        {"ontime", &LuaPlayerGetOntime},
        {"mutetime", &LuaPlayerGetMuteTime},
        {"setrank", &LuaPlayerSetRank},
        {"kick", &LuaPlayerKick},
        {"ban", &LuaPlayerBan},
        {"unban", &LuaPlayerUnban},
        {"stop", &LuaPlayerStop},
        {"unstop", &LuaPlayerUnstop},
        {"mute", &LuaPlayerMute},
        {"unmute", &LuaPlayerUnmute},
       {NULL, NULL}
};

int LuaPlayerLib::openLib(lua_State* L)
{
    lua_getglobal(L, "Player");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, lib, 0);
    lua_setglobal(L, "Player");
    return 1;
}

int LuaPlayerLib::LuaPlayerGetTable(lua_State* L) {
    Player_List* pll = Player_List::GetInstance();
    int numEntities = static_cast<int>(pll->_pList.size());
    int index = 1;

    lua_newtable(L);

    if (numEntities > 0) {
        for (auto const& e : pll->_pList) {
            lua_pushinteger(L, e->Number);
            lua_rawseti(L, -2, index++);
        }
    }

    lua_pushinteger(L, numEntities);

    return 2;
}

int LuaPlayerLib::LuaPlayerGetPrefix(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.getprefix() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();
    auto ple = pll->GetPointer(playerNumber);
    std::string result;

    if (ple != nullptr) {
        RankItem ri = rm->GetRank(ple->PRank, false);
        result = ri.Prefix;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlayerLib::LuaPlayerGetName(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.getname() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    auto ple = pll->GetPointer(playerNumber);
    std::string result;

    if (ple != nullptr) {

        result = ple->Name;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlayerLib::LuaPlayerGetSuffix(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.getsuffix() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();
    auto ple = pll->GetPointer(playerNumber);
    std::string result;

    if (ple != nullptr) {
        RankItem ri = rm->GetRank(ple->PRank, false);
        result = ri.Suffix;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}
int LuaPlayerLib::LuaPlayerGetIp(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.ip() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    auto ple = pll->GetPointer(playerNumber);
    std::string result;

    if (ple != nullptr) {
        result = ple->IP;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlayerLib::LuaPlayerGetOntime(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.ontime() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    auto ple = pll->GetPointer(playerNumber);
    double result = -1;

    if (ple != nullptr) {
        result = ple->OntimeCounter;
    }

    lua_pushnumber(L, result);
    return 1;
}

int LuaPlayerLib::LuaPlayerGetRank(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.rank() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    auto ple = pll->GetPointer(playerNumber);
    int result = -1;

    if (ple != nullptr) {
        result = ple->PRank;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlayerLib::LuaPlayerGetOnline(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.online() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    auto ple = pll->GetPointer(playerNumber);
    int result = 0;

    if (ple != nullptr) {
        result = ple->Online;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlayerLib::LuaPlayerGetMuteTime(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.mutetime() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    auto ple = pll->GetPointer(playerNumber);
    int result = -1;

    if (ple != nullptr) {
        result = ple->MuteTime;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlayerLib::LuaPlayerSetRank(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: player.setrank() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    int rankNumber = luaL_checkinteger(L, 2);
    std::string reason(luaL_checkstring(L, 3));

    Player_List* pll = Player_List::GetInstance();
    auto ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->SetRank(rankNumber, reason);
    }

    return 0;
}

int LuaPlayerLib::LuaPlayerKick(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs > 2) {
        Logger::LogAdd("Lua", "LuaError: player.kick() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    std::string reason(luaL_checkstring(L, 2));
    int count = 0;
    bool log = true;
    bool show = true;

    if (nArgs >= 3)
        count = luaL_checknumber(L, 3);

    if (nArgs >= 4)
        log = (luaL_checknumber(L, 4) > 0);

    if (nArgs >= 5)
        show = (luaL_checknumber(L, 5) > 0);

    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Kick(reason, show);
    }

    return 0;
}

int LuaPlayerLib::LuaPlayerBan(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: player.ban() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    std::string reason(luaL_checkstring(L, 2));

    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Ban(reason);
    }

    return 0;
}

int LuaPlayerLib::LuaPlayerUnban(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.unban() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);

    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Unban();
    }

    return 0;
}

int LuaPlayerLib::LuaPlayerStop(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: player.stop() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    std::string reason(luaL_checkstring(L, 2));

    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Stop(reason);
    }

    return 0;
}

int LuaPlayerLib::LuaPlayerUnstop(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.unstop() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);

    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Unstop();
    }

    return 0;
}

int LuaPlayerLib::LuaPlayerMute(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: player.mute() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);
    int minutes = luaL_checkinteger(L, 2);
    std::string reason(luaL_checkstring(L, 3));

    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Mute(minutes, reason);
    }

    return 0;
}

int LuaPlayerLib::LuaPlayerUnmute(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: player.unmute() called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int playerNumber = luaL_checkinteger(L, 1);

    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Unmute();
    }

    return 0;
}
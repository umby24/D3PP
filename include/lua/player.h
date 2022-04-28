#ifndef D3PP_LUA_PLAYER_H
#define D3PP_LUA_PLAYER_H

struct lua_State;
struct luaL_Reg;

class LuaPlayerLib
{
public:
    const static struct luaL_Reg lib[];
    int openLib(lua_State* L);
protected:
    static int LuaPlayerGetTable(lua_State* L);
    static int LuaPlayerGetPrefix(lua_State* L);
    static int LuaPlayerGetName(lua_State* L);
    static int LuaPlayerGetSuffix(lua_State* L);
    static int LuaPlayerGetIp(lua_State* L);
    static int LuaPlayerGetRank(lua_State* L);
    static int LuaPlayerGetOnline(lua_State* L);
    static int LuaPlayerGetOntime(lua_State* L);
    static int LuaPlayerGetMuteTime(lua_State* L);
    static int LuaPlayerSetRank(lua_State* L);
    static int LuaPlayerKick(lua_State* L);
    static int LuaPlayerBan(lua_State* L);
    static int LuaPlayerUnban(lua_State* L);
    static int LuaPlayerStop(lua_State* L);
    static int LuaPlayerUnstop(lua_State* L);
    static int LuaPlayerMute(lua_State* L);
    static int LuaPlayerUnmute(lua_State* L);
private:
};



#endif
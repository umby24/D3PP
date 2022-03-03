#ifndef D3PP_LUA_RANK_H
#define D3PP_LUA_RANK_H

struct lua_State;
struct luaL_Reg;

class LuaRankLib
{
public:
    const static struct luaL_Reg lib[];
    int openLib(lua_State* L);
protected:
    static int LuaRankGetTable(lua_State* L);
    static int LuaRankAdd(lua_State* L);
    static int LuaRankDelete(lua_State* L);
    static int LuaRankGetName(lua_State* L);
    static int LuaRankGetPrefix(lua_State* L);
    static int LuaRankGetSuffix(lua_State* L);
    static int LuaRankGetRoot(lua_State* L);
private:
};

#endif
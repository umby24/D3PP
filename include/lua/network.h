#ifndef D3PP_LUA_NETWORK_H
#define D3PP_LUA_NETWORK_H

struct lua_State;
struct luaL_Reg;

class LuaNetworkLib
{
public:
    const static struct luaL_Reg lib[];
    int openLib(lua_State* L);
private:
    static int LuaNetworkOutBlockSet(lua_State* L);
};

#endif
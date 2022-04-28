#ifndef D3PP_LUA_BLOCK_H
#define D3PP_LUA_BLOCK_H

struct lua_State;
struct luaL_Reg;

class LuaBlockLib
{
public:
    const static struct luaL_Reg lib[];
    int openLib(lua_State* L);
protected:
    static int LuaBlockGetTable(lua_State* L);
    static int LuaBlockGetName(lua_State* L);
    static int LuaBlockGetRankPlace(lua_State* L);
    static int LuaBlockGetRankDelete(lua_State* L);
    static int LuaBlockGetClientType(lua_State* L);
    static int LuaBlockCreate(lua_State* L);
    static int LuaBlockSetRankPlace(lua_State* L);
    static int LuaBlockSetRankDelete(lua_State* L);
    static int LuaBlockSetKills(lua_State* L);
    static int LuaBlockSetPhysics(lua_State* L);
private:
};

#endif
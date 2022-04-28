#ifndef D3PP_LUA_TELEPORTER_H
#define D3PP_LUA_TELEPORTER_H

struct lua_State;
struct luaL_Reg;

class LuaTeleporterLib
{
public:
    const static struct luaL_Reg lib[];
    static int openLib(lua_State* L);
protected:
    static int LuaTeleporterGetTable(lua_State* L);
    static int LuaTeleporterGetBox(lua_State* L);
    static int LuaTeleporterGetDestination(lua_State* L);
    static int LuaTeleporterAdd(lua_State* L);
    static int LuaTeleporterDelete(lua_State* L);
private:
};

#endif
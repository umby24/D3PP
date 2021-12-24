#ifndef D3PP_LUA_SYSTEM_H
#define D3PP_LUA_SYSTEM_H

struct lua_State;
struct luaL_Reg;

class LuaSystemLib
{
public:
    const static struct luaL_Reg lib[];
    int openLib(lua_State* L);
protected:
    static int LuaMessageToAll(lua_State* L);
    static int LuaMessage(lua_State* L);
    static int LuaFileGet(lua_State* L);
    static int LuaFolderGet(lua_State* L);
    static int LuaEventAdd(lua_State* L);
    static int LuaEventDelete(lua_State* L);
private:
};

#endif
#ifndef D3PP_LUA_CPE_H
#define D3PP_LUA_CPE_H

struct lua_State;
struct luaL_Reg;

class LuaCPELib
{
public:
    const static struct luaL_Reg lib[];
    int openLib(lua_State* L);
protected:
    static int LuaServerGetExtensions(lua_State* L);
    static int LuaServerGetExtension(lua_State* L);
    static int LuaClientGetExtensions(lua_State* L);
    static int LuaClientGetExtension(lua_State* L);
    static int LuaSelectionCuboidAdd(lua_State* L);
    static int LuaSelectionCuboidDelete(lua_State* L);
    static int LuaGetHeldBlock(lua_State* L);
    static int LuaSetHeldBlock(lua_State* L);
    static int LuaChangeModel(lua_State* L);
    static int LuaSetWeather(lua_State* L);
    static int LuaMapSetEnvColors(lua_State* L);
    static int LuaClientSetBlockPermissions(lua_State* L);
    static int LuaMapEnvSet(lua_State* L);
    static int LuaClientHackcontrolSend(lua_State* L);
    static int LuaHotkeyAdd(lua_State* L);
    static int LuaHotkeyRemove(lua_State* L);
    static int LuaMapHackcontrolSet(lua_State* L);
    static int LuaCreateBlock(lua_State* L);
    static int LuaDeleteBlock(lua_State* L);
    static int LuaSetBlockExt(lua_State* L);
    static int LuaSetBlockExtClient(lua_State* L);

    static int LuaCreateBlockClient(lua_State* L);
    static int LuaDeleteBlockClient(lua_State* L);
private:
};



#endif
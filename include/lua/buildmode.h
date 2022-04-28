#ifndef D3PP_LUA_BUILDMODE_H
#define D3PP_LUA_BUILDMODE_H

struct lua_State;
struct luaL_Reg;

class LuaBuildModeLib
{
public:
	const static struct luaL_Reg lib[];
	int openLib(lua_State* L);
protected:
    static int LuaBuildModeSet(lua_State* L);
    static int LuaBuildModeGet(lua_State* L);
    static int LuaBuildModeStateSet(lua_State* L);
    static int LuaBuildModeStateGet(lua_State* L);
    static int LuaBuildModeCoordinateSet(lua_State* L);
    static int LuaBuildModeCoordinateGet(lua_State* L);
    static int LuaBuildModeLongSet(lua_State* L);
    static int LuaBuildModeLongGet(lua_State* L);
    static int LuaBuildModeFloatSet(lua_State* L);
    static int LuaBuildModeFloatGet(lua_State* L);
    static int LuaBuildModeStringSet(lua_State* L);
    static int LuaBuildModeStringGet(lua_State* L);
private:
};



#endif
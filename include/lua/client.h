#ifndef D3PP_LUA_CLIENT_H
#define D3PP_LUA_CLIENT_H

struct lua_State;
struct luaL_Reg;

class LuaClientLib
{
public:
	const static struct luaL_Reg d3ClientLib[];
	int openLib(lua_State* L);
protected:
    static int LuaClientGetTable(lua_State* L);
	static int LuaClientGetMapId(lua_State* L);
	static int LuaClientGetIp(lua_State* L);
	static int LuaClientGetLoginName(lua_State* L);
	static int LuaClientGetLoggedIn(lua_State* L);
	static int LuaClientGetEntity(lua_State* L);
private:
};



#endif
#ifndef D3PP_LUA_BUILD_H
#define D3PP_LUA_BUILD_H

struct lua_State;
struct luaL_Reg;

class LuaBuildLib
{
public:
	const static struct luaL_Reg lib[];
	int openLib(lua_State* L);
protected:
    static int LuaBuildLinePlayer(lua_State* L);
    static int LuaBuildBoxPlayer(lua_State* L);
    static int LuaBuildSpherePlayer(lua_State* L);
    static int LuaBuildRankBox(lua_State* L);
private:
};

#endif
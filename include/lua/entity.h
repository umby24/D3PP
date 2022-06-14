#ifndef D3PP_LUA_ENTITY_H
#define D3PP_LUA_ENTITY_H

struct lua_State;
struct luaL_Reg;

class LuaEntityLib
{
public:
	const static struct luaL_Reg lib[];
	int openLib(lua_State* L);
protected:
   static int LuaEntityGetTable(lua_State* L);
   static int LuaEntityAdd(lua_State* L);
   static int LuaEntityDelete(lua_State* L);
   static int LuaEntityGetPlayer(lua_State* L);
   static int LuaEntityGetMapId(lua_State* L);
   static int LuaEntityGetPosition(lua_State* L);
   static int LuaEntityGetRotation(lua_State* L);
   static int LuaEntityGetLook(lua_State* L);
   static int LuaEntityResend(lua_State* L);
   static int LuaEntityMessage2Clients(lua_State* L);
   static int LuaEntityDisplaynameGet(lua_State* L);
   static int LuaEntityDisplaynameSet(lua_State* L);
   static int LuaEntityPositionSet(lua_State* L);
   static int LuaEntityKill(lua_State* L);
   static int LuaEntitySetModel(lua_State* L);
private:
};

#endif
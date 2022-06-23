#ifndef D3PP_LUA_SYSTEM_H
#define D3PP_LUA_SYSTEM_H
#include <memory>

struct lua_State;
struct luaL_Reg;

namespace D3PP::plugins {
    class LuaState;
}

class LuaSystemLib
{
public:
    const static struct luaL_Reg lib[];
    int openLib(lua_State* L, std::shared_ptr<D3PP::plugins::LuaState> thisPlugin);
    int LuaEventAdd(lua_State* L);
    int LuaEventDelete(lua_State* L);
protected:
    static int LuaMessageToAll(lua_State* L);
    static int LuaMessage(lua_State* L);
    static int LuaFileGet(lua_State* L);
    static int LuaFolderGet(lua_State* L);
    static int LuaSystemLog(lua_State* L);
    static int LuaGetPlatform(lua_State* L);
    static int LuaAddCommand(lua_State* L);
    static int LuaSetSoftwareName(lua_State* L);
private:
    std::shared_ptr<D3PP::plugins::LuaState> m_thisPlugin;
};

#endif
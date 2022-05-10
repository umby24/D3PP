#ifndef D3PP_LUA_MAP_H
#define D3PP_LUA_MAP_H

struct lua_State;
struct luaL_Reg;

class LuaMapLib
{
public:
    const static struct luaL_Reg lib[];
    int openLib(lua_State* L);
protected:
    static int LuaMapGetTable(lua_State* L);
    static int LuaMapBlockChange(lua_State* L);
    static int LuaMapBlockChangeClient(lua_State* L);
    static int LuaMapBlockChangePlayer(lua_State* L);
    static int LuaMapBlockMove(lua_State* L);
    static int LuaMapBlockSend(lua_State* L);
    static int LuaMapBlockGetType(lua_State* L);
    static int LuaMapBlockGetRank(lua_State* L);
    static int LuaMapBlockGetPlayer(lua_State* L);
    static int LuaMapGetName(lua_State* L);
    static int LuaMapGetUniqueId(lua_State* L);
    static int LuaMapGetDirectory(lua_State* L);
    static int LuaMapGetRankBuild(lua_State* L);
    static int LuaMapGetRankShow(lua_State* L);
    static int LuaMapGetRankJoin(lua_State* L);
    static int LuaMapGetDimensions(lua_State* L);
    static int LuaMapGetSpawn(lua_State* L);
    static int LuaMapGetSaveInterval(lua_State* L);
    static int LuaMapSetName(lua_State* L);
    static int LuaMapSetDirectory(lua_State* L);
    static int LuaMapSetRankBuild(lua_State* L);
    static int LuaMapSetRankJoin(lua_State* L);
    static int LuaMapSetRankShow(lua_State* L);
    static int LuaMapSetSpawn(lua_State* L);
    static int LuaMapSetSaveInterval(lua_State* L);
    static int LuaMapAdd(lua_State* L);
    static int LuaMapActionAddResize(lua_State* L);
    static int LuaMapActionAddFill(lua_State* L);
    static int LuaMapActionAddSave(lua_State* L);
    static int LuaMapActionAddDelete(lua_State* L);
    static int LuaMapLoad(lua_State* L);
    static int LuaMapResend(lua_State* L);
    static int LuaMapExport(lua_State* L);
    static int LuaMapExportGetSize(lua_State* L);
    static int LuaMapImportPlayer(lua_State* L);
    static int LuaFillFlat(lua_State* L);

    static int LuaBeginFill(lua_State* L);
    static int LuaSetFillBlock(lua_State* L);
    static int LuaGetFillBlock(lua_State *L);
    static int LuaEndFill(lua_State *L);
private:
};

#endif
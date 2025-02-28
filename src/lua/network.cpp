#include "lua/network.h"

#include <lua.hpp>
#include "common/Logger.h"
#include "Utils.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/Network_Functions.h"

const struct luaL_Reg LuaNetworkLib::lib[] = {
        {"setblock", &LuaNetworkOutBlockSet},
        {NULL, NULL}
};

int LuaNetworkLib::openLib(lua_State* L)
{
    lua_getglobal(L, "Network");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, lib, 0);
    lua_setglobal(L, "Network");
    return 1;
}

int LuaNetworkLib::LuaNetworkOutBlockSet(lua_State* L)
{
    int nArgs = lua_gettop(L);

    if (nArgs != 5) {
        Logger::LogAdd("Lua", "LuaError: NetworkOutBlockSet called with invalid number of arguments.", WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    int blockX = luaL_checkinteger(L, 2);
    int blockY = luaL_checkinteger(L, 3);
    int blockZ = luaL_checkinteger(L, 4);
    int blockType = luaL_checkinteger(L, 5);

    Network* n = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> client = n->GetClient(clientId);

    if (client != nullptr) {
        NetworkFunctions::NetworkOutBlockSet(clientId, blockX, blockY, blockZ, static_cast<unsigned char>(blockType));
    }

    return 0;
}
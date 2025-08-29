#include "lua/client.h"

#include <lua.hpp>
#include <shared_mutex>

#include "common/Logger.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/Server.h"
#include "world/Player.h"
#include "world/Entity.h"
#include "plugins/LuaPlugin.h"

const struct luaL_Reg LuaClientLib::d3ClientLib[] = {
       {"getall",  &LuaClientGetTable},
       {"getmap", &LuaClientGetMapId},
       {"getip", &LuaClientGetIp},
       {"getloginname",  &LuaClientGetLoginName},
       {"isloggedin", &LuaClientGetLoggedIn},
       {"getentity", &LuaClientGetEntity},
       {"kick", &LuaClientKick},
       {NULL, NULL}
};

int LuaClientLib::openLib(lua_State* L)
{
    lua_getglobal(L, "Client");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaClientLib::d3ClientLib, 0);
    lua_setglobal(L, "Client");
    return 1;
}

int LuaClientLib::LuaClientGetTable(lua_State* L) {
    std::shared_lock lock(D3PP::network::Server::roMutex, std::defer_lock);
    int numClients = static_cast<int>(D3PP::network::Server::roClients.size());
    int index = 1;
    lua_newtable(L);

    if (numClients > 0) {
        for (auto const& nc : D3PP::network::Server::roClients) {
            lua_pushinteger(L, nc->GetId());
            lua_rawseti(L, -2, index++);
        }
    }

    lua_pushinteger(L, numClients);

    return 2;
}

int LuaClientLib::LuaClientGetMapId(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: client.getmapid called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = static_cast<int>(luaL_checkinteger(L, 1));
    int result = -1;
    std::shared_ptr<NetworkClient> client = std::static_pointer_cast<NetworkClient>(Network::GetClient(clientId));
    if (client != nullptr && client->GetLoggedIn()) {
        if (client->GetPlayerInstance())
            if (client->GetPlayerInstance()->GetEntity())
                result = client->GetPlayerInstance()->GetEntity()->MapID;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaClientLib::LuaClientGetIp(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: client.getip() called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = static_cast<int>(luaL_checkinteger(L, 1));
    std::string result;
    std::shared_ptr<NetworkClient> client = std::static_pointer_cast<NetworkClient>(Network::GetClient(clientId));
    if (client != nullptr) {
        result = client->IP;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaClientLib::LuaClientGetLoginName(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Client.getloginname called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = static_cast<int>(luaL_checkinteger(L, 1));
    std::string result;
    std::shared_ptr<IMinecraftClient> client = Network::GetClient(clientId);

    if (client != nullptr) {
        result = client->GetLoginName();
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaClientLib::LuaClientGetLoggedIn(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: client.getloggedin called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = static_cast<int>(luaL_checkinteger(L, 1));
    int result = -1;

    std::shared_ptr<NetworkClient> client = std::static_pointer_cast<NetworkClient>(Network::GetClient(clientId));

    if (client != nullptr) {
        result = client->LoggedIn;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaClientLib::LuaClientGetEntity(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Client.getEntity called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = static_cast<int>(luaL_checkinteger(L, 1));
    int result = -1;
    std::shared_ptr<IMinecraftClient> client = Network::GetClient(clientId);

    if (client != nullptr && client->GetLoggedIn()) {
        std::shared_ptr<Entity> clientEntity = Entity::GetPointer(clientId, true);

        if (clientEntity != nullptr)
            result = clientEntity->Id;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaClientLib::LuaClientKick(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 1) {
        Logger::LogAdd("Lua", "LuaError: Client.kick called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = static_cast<int>(luaL_checkinteger(L, 1));
    std::string reason(luaL_optstring(L, 2, "Kicked by script"));
    int result = -1;
    std::shared_ptr<IMinecraftClient> client = Network::GetClient(clientId);

    if (client != nullptr) {
        client->Kick(reason, true);
    }

    lua_pushinteger(L, result);
    return 1;
}

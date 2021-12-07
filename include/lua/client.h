#ifndef D3PP_LUA_CLIENT_H
#define D3PP_LUA_CLIENT_H
//#include <lua.hpp>
//
//const struct luaL_Reg d3ClientLib[] = {
//    {"getall", &dispatch<&LuaClientLib::LuaClientGetTable>},
//    {"getmap", &dispatch<&LuaClientLib::LuaClientGetMapId>},
//    {"getip", &dispatch<&LuaClientLib::LuaClientGetIp>},
//    {"getloginname",  &dispatch<&LuaClientLib::LuaClientGetLoginName>},
//    {"isloggedin", &dispatch<&LuaClientLib::LuaClientGetLoggedIn>},
//    {"getentity", &dispatch<&LuaClientLib::LuaClientGetEntity>},
//    {NULL, NULL}
//};
//
//class LuaClientLib
//{
//public:
//	int openLib(lua_State *L)
//	{
//	    lua_getglobal(L, "d3client");
//	    if (lua_isnil(L, -1))
//	    {
//	        lua_pop(L, 1);
//	        lua_newtable(L);
//	    }
//	    luaL_setfuncs(L, LuaPlugin::d3ClientLib, 0);
//	    lua_setglobal(L, "d3client");
//	    return 1;
//	}
//protected:
//    int LuaClientGetTable();
//    int LuaClientGetMapId();
//    int LuaClientGetIp();
//    int LuaClientGetLoginName();
//    int LuaClientGetLoggedIn();
//    int LuaClientGetEntity();
//private:
//};
//
//
//int LuaPlugin::LuaClientGetTable(lua_State* L) {
//    Network* nm = Network::GetInstance();
//    int numClients = static_cast<int>(nm->roClients.size());
//    int index = 1;
//
//    lua_newtable(L);
//
//    if (numClients > 0) {
//        for (auto const& nc : nm->roClients) {
//            lua_pushinteger(L, index++);
//            lua_pushinteger(L, nc->GetId());
//            lua_settable(L, -3);
//        }
//    }
//
//    lua_pushinteger(L, numClients);
//
//    return 2;
//}
//
//int LuaPlugin::LuaClientGetMapId(lua_State* L) {
//    int nArgs = lua_gettop(L);
//
//    if (nArgs != 1) {
//        Logger::LogAdd("Lua", "LuaError: Client_Get_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
//        return 0;
//    }
//
//    int clientId = lua_tointeger(L, 1);
//    int result = -1;
//    Network* nm = Network::GetInstance();
//    std::shared_ptr<NetworkClient> client = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));
//    if (client != nullptr) {
//        result = client->player->MapId;
//    }
//
//    lua_pushinteger(L, result);
//    return 1;
//}
//
//int LuaPlugin::LuaClientGetIp(lua_State* L) {
//    int nArgs = lua_gettop(L);
//
//    if (nArgs != 1) {
//        Logger::LogAdd("Lua", "LuaError: CLient_Get_IP called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
//        return 0;
//    }
//
//    int clientId = lua_tointeger(L, 1);
//    std::string result;
//    Network* nm = Network::GetInstance();
//    std::shared_ptr<NetworkClient> client = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));
//    if (client != nullptr) {
//        result = client->IP;
//    }
//
//    lua_pushstring(L, result.c_str());
//    return 1;
//}
//
//int LuaPlugin::LuaClientGetLoginName(lua_State* L) {
//    int nArgs = lua_gettop(L);
//
//    if (nArgs != 1) {
//        Logger::LogAdd("Lua", "LuaError: CLient_Get_Login_Name called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
//        return 0;
//    }
//
//    int clientId = lua_tointeger(L, 1);
//    std::string result;
//    Network* nm = Network::GetInstance();
//    std::shared_ptr<IMinecraftClient> client = nm->GetClient(clientId);
//
//    if (client != nullptr) {
//        result = client->GetLoginName();
//    }
//
//    lua_pushstring(L, result.c_str());
//    return 1;
//}
//
//int LuaPlugin::LuaClientGetLoggedIn(lua_State* L) {
//    int nArgs = lua_gettop(L);
//
//    if (nArgs != 1) {
//        Logger::LogAdd("Lua", "LuaError: CLient_Get_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
//        return 0;
//    }
//
//    int clientId = lua_tointeger(L, 1);
//    int result = -1;
//
//    Network* nm = Network::GetInstance();
//    std::shared_ptr<NetworkClient> client = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));
//
//    if (client != nullptr) {
//        result = client->LoggedIn;
//    }
//
//    lua_pushinteger(L, result);
//    return 1;
//}
//
//int LuaPlugin::LuaClientGetEntity(lua_State* L) {
//    int nArgs = lua_gettop(L);
//
//    if (nArgs != 1) {
//        Logger::LogAdd("Lua", "LuaError: CLient_Get_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
//        return 0;
//    }
//
//    int clientId = lua_tointeger(L, 1);
//    int result = -1;
//    Network* nm = Network::GetInstance();
//    std::shared_ptr<IMinecraftClient> client = nm->GetClient(clientId);
//
//    if (client != nullptr) {
//        std::shared_ptr<Entity> clientEntity = Entity::GetPointer(clientId, true);
//
//        if (clientEntity != nullptr)
//            result = clientEntity->Id;
//    }
//
//    lua_pushinteger(L, result);
//    return 1;
//}

#endif
#include "plugins/LuaPlugin.h"

#include <filesystem>

#include "Map.h"
#include "Network_Functions.h"
#include "Logger.h"
#include "Utils.h"
#include "Network.h"
#include "NetworkClient.h"
#include "Player.h"
#include "Entity.h"
#include "BuildMode.h"
#include "Player_List.h"
#include "Build.h"
#include "Rank.h"
#include "EventSystem.h"
#include "Block.h"
#include "MinecraftLocation.h"
#include "CPE.h"
#include "Files.h"

// -- Events..
#include "events/EventChatAll.h"
#include "events/EventChatMap.h"
#include "events/EventChatPrivate.h"
#include "events/EventClientAdd.h"
#include "events/EventClientDelete.h"
#include "events/EventClientLogin.h"
#include "events/EventClientLogout.h"
#include "events/EventEntityAdd.h"
#include "events/EventEntityDelete.h"
#include "events/EventEntityDie.h"
#include "events/EventEntityMapChange.h"
#include "events/EventEntityPositionSet.h"
#include "events/EventMapActionDelete.h"
#include "events/EventMapActionFill.h"
#include "events/EventMapActionLoad.h"
#include "events/EventMapActionResize.h"
#include "events/EventMapActionSave.h"
#include "events/EventMapAdd.h"
#include "events/EventMapBlockChange.h"
#include "events/EventMapBlockChangeClient.h"
#include "events/EventMapBlockChangePlayer.h"
#include "events/EventTimer.h"

LuaPlugin* LuaPlugin::Instance = nullptr;

typedef int (LuaPlugin::*mem_func)(lua_State * L);

// This template wraps a member function into a C-style "free" function compatible with lua.
template <mem_func func>
int dispatch(lua_State * L) {
    LuaPlugin * ptr = *static_cast<LuaPlugin**>(lua_getextraspace(L));
    return ((*ptr).*func)(L);
}

void bail(lua_State *L, std::string msg) {
    Logger::LogAdd("Lua", "Fatal Error: " + msg + " " + lua_tostring(L, -1), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
}


LuaPlugin::LuaPlugin() {
    this->Setup = [this] { Init(); };
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);

    TaskScheduler::RegisterTask("Lua", *this);

    state = luaL_newstate();
    luaL_openlibs(state);
    *static_cast<LuaPlugin**>(lua_getextraspace(this->state)) = this;
    executionMutex;

    BindFunctions();
    RegisterEventListener();
}

LuaPlugin::~LuaPlugin() {
    lua_close(state);
}

LuaPlugin* LuaPlugin::GetInstance() {
    if (Instance == nullptr) {
        Instance = new LuaPlugin();
    }

    return Instance;
}

void LuaPlugin::RegisterEventListener() {
    Dispatcher::subscribe(EventChatAll{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventChatMap{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
   // Dispatcher::subscribe(EventChatPrivate{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventClientAdd{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventClientDelete{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventClientLogin{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventClientLogout{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityAdd{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityDelete{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityDie{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityMapChange{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityPositionSet{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapActionDelete{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapActionFill{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapActionLoad{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapActionResize{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapActionSave{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapAdd{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapBlockChange{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapBlockChangeClient{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapBlockChangePlayer{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventTimer{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
}

void LuaPlugin::HandleEvent(const Event& event) {
    if (!event.PushLua)
        return;

    auto type = event.type(); // -- get the type..

    if( _luaEvents.find( type ) == _luaEvents. end() ) // -- find all functions that want to be called on this event
        return;

    auto&& observers = _luaEvents.at( type );

    for( auto&& observer : observers ) {
        executionMutex.lock();
        lua_getglobal(state, observer.functionName.c_str()); // -- Get the function to be called
        if (!lua_isfunction(state, -1)) {
            lua_pop(state, 1);
            executionMutex.unlock();
            continue;
        }
        int argCount = event.PushLua(state); // -- Have the event push its args and return how many were pushed..
        try {
            if (lua_pcall(state, argCount, 0, 0) != 0) { // -- Call the function.
                bail(state, "[Event Handler]"); // -- catch errors
                executionMutex.unlock();
                return;
            }
        } catch (const int exception) {
            bail(state, "[Error Handler]"); // -- catch errors
            executionMutex.unlock();
            return;
        }
        executionMutex.unlock();
        // -- done.
    }
}

void LuaPlugin::BindFunctions() {
    // -- client functions:
    lua_register(state, "Client_Get_Table", &dispatch<&LuaPlugin::LuaClientGetTable>);
    lua_register(state, "Client_Get_Map_ID", &dispatch<&LuaPlugin::LuaClientGetMapId>);
    lua_register(state, "Client_Get_IP", &dispatch<&LuaPlugin::LuaClientGetIp>);
    lua_register(state, "Client_Get_Login_Name", &dispatch<&LuaPlugin::LuaClientGetLoginName>);
    lua_register(state, "Client_Get_Logged_In", &dispatch<&LuaPlugin::LuaClientGetLoggedIn>);
    lua_register(state, "Client_Get_Entity", &dispatch<&LuaPlugin::LuaClientGetEntity>);
    // -- Build Mode Functions
    lua_register(state, "Build_Mode_Set", &dispatch<&LuaPlugin::LuaBuildModeSet>);
    lua_register(state, "Build_Mode_Get", &dispatch<&LuaPlugin::LuaBuildModeGet>);
    lua_register(state, "Build_Mode_State_Set", &dispatch<&LuaPlugin::LuaBuildModeStateSet>);
    lua_register(state, "Build_Mode_State_Get", &dispatch<&LuaPlugin::LuaBuildModeStateGet>);
    lua_register(state, "Build_Mode_Coordinate_Set", &dispatch<&LuaPlugin::LuaBuildModeCoordinateSet>);
    lua_register(state, "Build_Mode_Coordinate_Get", &dispatch<&LuaPlugin::LuaBuildModeCoordinateGet>);
    lua_register(state, "Build_Mode_Long_Set", &dispatch<&LuaPlugin::LuaBuildModeLongSet>);
    lua_register(state, "Build_Mode_Long_Get", &dispatch<&LuaPlugin::LuaBuildModeLongGet>);
    lua_register(state, "Build_Mode_Float_Set", &dispatch<&LuaPlugin::LuaBuildModeFloatSet>);
    lua_register(state, "Build_Mode_Float_Get", &dispatch<&LuaPlugin::LuaBuildModeFloatGet>);
    lua_register(state, "Build_Mode_String_Set", &dispatch<&LuaPlugin::LuaBuildModeStringSet>);
    lua_register(state, "Build_Mode_String_Get", &dispatch<&LuaPlugin::LuaBuildModeStringGet>);
    // -- build functions
    lua_register(state, "Build_Line_Player", &dispatch<&LuaPlugin::LuaBuildLinePlayer>);
    lua_register(state, "Build_Box_Player", &dispatch<&LuaPlugin::LuaBuildBoxPlayer>);
    lua_register(state, "Build_Sphere_Player", &dispatch<&LuaPlugin::LuaBuildSpherePlayer>);
    lua_register(state, "Build_Rank_Box", &dispatch<&LuaPlugin::LuaBuildRankBox>);
    // -- Entity Functions:
    lua_register(state, "Entity_Get_Table", &dispatch<&LuaPlugin::LuaEntityGetTable>);
    lua_register(state, "Entity_Add", &dispatch<&LuaPlugin::LuaEntityAdd>);
    lua_register(state, "Entity_Delete", &dispatch<&LuaPlugin::LuaEntityDelete>);
    lua_register(state, "Entity_Get_Player", &dispatch<&LuaPlugin::LuaEntityGetPlayer>);
    lua_register(state, "Entity_Get_Map_ID", &dispatch<&LuaPlugin::LuaEntityGetMapId>);
    lua_register(state, "Entity_Get_X", &dispatch<&LuaPlugin::LuaEntityGetX>);
    lua_register(state, "Entity_Get_Y", &dispatch<&LuaPlugin::LuaEntityGetY>);
    lua_register(state, "Entity_Get_Z", &dispatch<&LuaPlugin::LuaEntityGetZ>);
    lua_register(state, "Entity_Get_Rotation", &dispatch<&LuaPlugin::LuaEntityGetRotation>);
    lua_register(state, "Entity_Get_Look", &dispatch<&LuaPlugin::LuaEntityGetLook>);
    lua_register(state, "Entity_Resend", &dispatch<&LuaPlugin::LuaEntityResend>);
    lua_register(state, "Entity_Message_2_Clients", &dispatch<&LuaPlugin::LuaEntityMessage2Clients>);
    lua_register(state, "Entity_Displayname_Get", &dispatch<&LuaPlugin::LuaEntityDisplaynameGet>);
    lua_register(state, "Entity_Displayname_Set", &dispatch<&LuaPlugin::LuaEntityDisplaynameSet>);
    lua_register(state, "Entity_Position_Set", &dispatch<&LuaPlugin::LuaEntityPositionSet>);
    lua_register(state, "Entity_Kill", &dispatch<&LuaPlugin::LuaEntityKill>);
    // -- Player functions:
    lua_register(state, "Player_Get_Table", &dispatch<&LuaPlugin::LuaPlayerGetTable>);
    lua_register(state, "Player_Get_Prefix", &dispatch<&LuaPlugin::LuaPlayerGetPrefix>);
    lua_register(state, "Player_Get_Name", &dispatch<&LuaPlugin::LuaPlayerGetName>);
    lua_register(state, "Player_Get_Suffix", &dispatch<&LuaPlugin::LuaPlayerGetSuffix>);
    lua_register(state, "Player_Get_IP", &dispatch<&LuaPlugin::LuaPlayerGetIp>);
    lua_register(state, "Player_Get_Rank", &dispatch<&LuaPlugin::LuaPlayerGetRank>);
    lua_register(state, "Player_Get_Online", &dispatch<&LuaPlugin::LuaPlayerGetOnline>);
    lua_register(state, "Player_Get_Ontime", &dispatch<&LuaPlugin::LuaPlayerGetOntime>);
    lua_register(state, "Player_Get_Mute_Time", &dispatch<&LuaPlugin::LuaPlayerGetMuteTime>);
    lua_register(state, "Player_Set_Rank", &dispatch<&LuaPlugin::LuaPlayerSetRank>);
    lua_register(state, "Player_Kick", &dispatch<&LuaPlugin::LuaPlayerKick>);
    lua_register(state, "Player_Ban", &dispatch<&LuaPlugin::LuaPlayerBan>);
    lua_register(state, "Player_Unban", &dispatch<&LuaPlugin::LuaPlayerUnban>);
    lua_register(state, "Player_Stop", &dispatch<&LuaPlugin::LuaPlayerStop>);
    lua_register(state, "Player_Unstop", &dispatch<&LuaPlugin::LuaPlayerUnstop>);
    lua_register(state, "Player_Mute", &dispatch<&LuaPlugin::LuaPlayerMute>);
    lua_register(state, "Player_Unmute", &dispatch<&LuaPlugin::LuaPlayerUnmute>);
    // -- Map Functions:
    lua_register(state, "Map_Get_Table", &dispatch<&LuaPlugin::LuaMapGetTable>);
    lua_register(state, "Map_Block_Change", &dispatch<&LuaPlugin::LuaMapBlockChange>);
    lua_register(state, "Map_Block_Change_Client", &dispatch<&LuaPlugin::LuaMapBlockChangeClient>);
    lua_register(state, "Map_Block_Change_Player", &dispatch<&LuaPlugin::LuaMapBlockChangePlayer>);
    lua_register(state, "Map_Block_Move", &dispatch<&LuaPlugin::LuaMapBlockMove>);
    lua_register(state, "Map_Block_Get_Type", &dispatch<&LuaPlugin::LuaMapBlockGetType>);
    lua_register(state, "Map_Block_Get_Rank", &dispatch<&LuaPlugin::LuaMapBlockGetRank>);
    lua_register(state, "Map_Block_Get_Player_Last", &dispatch<&LuaPlugin::LuaMapBlockGetPlayer>);
    lua_register(state, "Map_Get_Name", &dispatch<&LuaPlugin::LuaMapGetName>);
    lua_register(state, "Map_Get_Unique_ID", &dispatch<&LuaPlugin::LuaMapGetUniqueId>);
    lua_register(state, "Map_Get_Directory", &dispatch<&LuaPlugin::LuaMapGetDirectory>);
    lua_register(state, "Map_Get_Rank_Build", &dispatch<&LuaPlugin::LuaMapGetRankBuild>);
    lua_register(state, "Map_Get_Rank_Show", &dispatch<&LuaPlugin::LuaMapGetRankShow>);
    lua_register(state, "Map_Get_Rank_Join", &dispatch<&LuaPlugin::LuaMapGetRankJoin>);
    lua_register(state, "Map_Get_Dimensions", &dispatch<&LuaPlugin::LuaMapGetDimensions>);
    lua_register(state, "Map_Get_Spawn", &dispatch<&LuaPlugin::LuaMapGetSpawn>);
    lua_register(state, "Map_Get_Save_Intervall", &dispatch<&LuaPlugin::LuaMapGetSaveInterval>);
    lua_register(state, "Map_Set_Name", &dispatch<&LuaPlugin::LuaMapSetName>);
    lua_register(state, "Map_Set_Directory", &dispatch<&LuaPlugin::LuaMapSetDirectory>);
    lua_register(state, "Map_Set_Rank_Build", &dispatch<&LuaPlugin::LuaMapSetRankBuild>);
    lua_register(state, "Map_Set_Rank_Join", &dispatch<&LuaPlugin::LuaMapSetRankJoin>);
    lua_register(state, "Map_Set_Rank_Show", &dispatch<&LuaPlugin::LuaMapSetRankShow>);
    lua_register(state, "Map_Set_Spawn", &dispatch<&LuaPlugin::LuaMapSetSpawn>);
    lua_register(state, "Map_Set_Save_Intervall", &dispatch<&LuaPlugin::LuaMapSetSaveInterval>);
    lua_register(state, "Map_Add", &dispatch<&LuaPlugin::LuaMapAdd>);
    lua_register(state, "Map_Action_Add_Resize", &dispatch<&LuaPlugin::LuaMapActionAddResize>);
    lua_register(state, "Map_Action_Add_Save", &dispatch<&LuaPlugin::LuaMapActionAddSave>);
    lua_register(state, "Map_Action_Add_Fill", &dispatch<&LuaPlugin::LuaMapActionAddFill>);
    lua_register(state, "Map_Action_Add_Delete", &dispatch<&LuaPlugin::LuaMapActionAddDelete>);
    lua_register(state, "Map_Resend", &dispatch<&LuaPlugin::LuaMapResend>);
    lua_register(state, "Map_Export", &dispatch<&LuaPlugin::LuaMapExport>);
    lua_register(state, "Map_Import", &dispatch<&LuaPlugin::LuaMapImportPlayer>);
    // -- Block Functions
    lua_register(state, "Block_Get_Table", &dispatch<&LuaPlugin::LuaBlockGetTable>);
    lua_register(state, "Block_Get_Name", &dispatch<&LuaPlugin::LuaBlockGetName>);
    lua_register(state, "Block_Get_Rank_Place", &dispatch<&LuaPlugin::LuaBlockGetRankPlace>);
    lua_register(state, "Block_Get_Rank_Delete", &dispatch<&LuaPlugin::LuaBlockGetRankDelete>);
    lua_register(state, "Block_Get_Client_Type", &dispatch<&LuaPlugin::LuaBlockGetClientType>);
    // -- Rank Functions
    lua_register(state, "Rank_Get_Table", &dispatch<&LuaPlugin::LuaRankGetTable>);
    lua_register(state, "Rank_Add", &dispatch<&LuaPlugin::LuaRankAdd>);
    lua_register(state, "Rank_Delete", &dispatch<&LuaPlugin::LuaRankDelete>);
    lua_register(state, "Rank_Get_Name", &dispatch<&LuaPlugin::LuaRankGetName>);
    lua_register(state, "Rank_Get_Prefix", &dispatch<&LuaPlugin::LuaRankGetPrefix>);
    lua_register(state, "Rank_Get_Suffix", &dispatch<&LuaPlugin::LuaRankGetSuffix>);
    lua_register(state, "Rank_Get_Root", &dispatch<&LuaPlugin::LuaRankGetRoot>);
    // -- Teleporter functions
    lua_register(state, "Teleporter_Get_Table", &dispatch<&LuaPlugin::LuaTeleporterGetTable>);
    lua_register(state, "Teleporter_Add", &dispatch<&LuaPlugin::LuaTeleporterAdd>);
    lua_register(state, "Teleporter_Delete", &dispatch<&LuaPlugin::LuaTeleporterDelete>);
    lua_register(state, "Teleporter_Get_Box", &dispatch<&LuaPlugin::LuaTeleporterGetBox>);
    lua_register(state, "Teleporter_Get_Destination", &dispatch<&LuaPlugin::LuaTeleporterGetDestination>);
    // -- System Functions
    lua_register(state, "System_Message_Network_Send_2_All", &dispatch<&LuaPlugin::LuaMessageToAll>);
    lua_register(state, "System_Message_Network_Send", &dispatch<&LuaPlugin::LuaMessage>);
    lua_register(state, "Lang_Get", &dispatch<&LuaPlugin::LuaLanguageGet>);
    lua_register(state, "Files_File_Get", &dispatch<&LuaPlugin::LuaFileGet>);
    lua_register(state, "Files_Folder_Get", &dispatch<&LuaPlugin::LuaFolderGet>);
    // -- Events
    lua_register(state, "Event_Add", &dispatch<&LuaPlugin::LuaEventAdd>);
    lua_register(state, "Event_Delete", &dispatch<&LuaPlugin::LuaEventDelete>);
    // -- CPE
    lua_register(state, "Client_Get_Extension", &dispatch<&LuaPlugin::LuaClientGetExtension>);
    lua_register(state, "CPE_Selection_Cuboid_Add", &dispatch<&LuaPlugin::LuaSelectionCuboidAdd>);
    lua_register(state, "CPE_Selection_Cuboid_Delete", &dispatch<&LuaPlugin::LuaSelectionCuboidDelete>);
    lua_register(state, "CPE_Get_Held_Block", &dispatch<&LuaPlugin::LuaGetHeldBlock>);
    lua_register(state, "CPE_Set_Held_Block", &dispatch<&LuaPlugin::LuaSetHeldBlock>);

}

void LuaPlugin::Init() {
    if (!std::filesystem::exists("Lua"))
        std::filesystem::create_directory("Lua");
}

std::vector<std::string> EnumerateDirectory(const std::string &dir) {
    std::vector<std::string> result{};

    if (std::filesystem::is_directory(dir)) {
        for (const auto &entry : std::filesystem::directory_iterator(dir)) {
            std::string fileName = entry.path().filename().string();

            if (fileName.length() < 3)
                continue;

            if (entry.is_directory()) {
                std::vector<std::string> recurseResult = EnumerateDirectory(entry.path().string());
                result.insert(result.end(), recurseResult.begin(), recurseResult.end());
            }

            if (fileName.substr(fileName.length() - 3) == "lua") {
                result.push_back(entry.path().string());
            }
        }
    }

    return result;
}

void LuaPlugin::MainFunc() {
    std::vector<std::string> files = EnumerateDirectory("Lua/");

    for (auto const &f : files) {
        auto i = _files.find(f);
        if (i != _files.end()) {
            time_t lastMod = Utils::FileModTime(f);
            // -- Note only accurate within +/- 1 second. shouldn't matter.
            if (i->second.LastLoaded != lastMod) {
                LoadFile(f);
                i->second.LastLoaded = lastMod;
            }

            continue;
        }
        LuaFile newFile;
        newFile.FilePath = f;
        newFile.LastLoaded = 0;
        _files.insert(std::make_pair(f, newFile));
    }
    auto timerDescriptor = Dispatcher::getDescriptor("Timer");
    if (_luaEvents.find(timerDescriptor) == _luaEvents.end())
        return;

    auto &eventsAt = _luaEvents[timerDescriptor];
    for(auto &e : eventsAt) {
        if (clock() >= (e.lastRun + e.duration)) {
            e.lastRun = clock();
            executionMutex.lock();
            lua_getglobal(state, e.functionName.c_str()); // -- Get the function to be called
            if (!lua_isfunction(state, -1)) {
                lua_pop(state, 1);
                executionMutex.unlock();
                continue;
            }
            if (lua_pcall(state, 0, 0, 0) != 0) { // -- Call the function.
                bail(state, "[Timer Event Handler]"); // -- catch errors
                executionMutex.unlock();
                return;
            }
            executionMutex.unlock();
        }
    }
}

int LuaPlugin::LuaMapBlockChange(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 10) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Change called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int X = lua_tointeger(L, 3);
    int Y = lua_tointeger(L, 4);
    int Z = lua_tointeger(L, 5);
    unsigned char type = lua_tointeger(L, 6);
    bool Undo = (lua_tointeger(L, 7) > 0);
    bool physics = (lua_tointeger(L, 8) > 0);
    bool send = (lua_tointeger(L, 9) > 0);
    unsigned char priority = lua_tointeger(L, 10);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        map->BlockChange(playerNumber, X, Y, Z, type, Undo, physics, send, priority);
    }

    return 0;
}

int LuaPlugin::LuaMapBlockChangeClient(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Change_Client called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int X = lua_tointeger(L, 3);
    int Y = lua_tointeger(L, 4);
    int Z = lua_tointeger(L, 5);
    unsigned char mode = lua_tointeger(L, 6);
    unsigned char type = lua_tointeger(L, 7);
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (client == nullptr || map== nullptr) {
        return 0;
    }

    map->BlockChange(client, X, Y, Z, mode, type);
    return 0;
}


int LuaPlugin::LuaMapBlockGetType(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Get_Type called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int X = lua_tointeger(L, 2);
    int Y = lua_tointeger(L, 3);
    int Z = lua_tointeger(L, 4);

    int result = -1;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        result = map->GetBlockType(X, Y, Z);
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaMessageToAll(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: System_Message_Network_Send_2_All called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    std::string message = lua_tostring(L, 2);

    int messageType = 0;

    if (nArgs == 3 && lua_isnumber(L, 3)) {
        messageType = lua_tointeger(L, 3);
    }

    NetworkFunctions::SystemMessageNetworkSend2All(mapId, message, messageType);
    return 0;
}

int LuaPlugin::LuaMessage(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: System_Message_Network_Send called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    std::string message = lua_tostring(L, 2);

    int messageType = 0;

    if (nArgs == 3 && lua_isnumber(L, 3)) {
        messageType = lua_tointeger(L, 3);
    }

    NetworkFunctions::SystemMessageNetworkSend(clientId, message, messageType);
    return 0;
}

void LuaPlugin::LoadFile(std::string path) {
    if(luaL_loadfile(this->state, path.c_str())) {
        bail(this->state, "Failed to load " + path);
        return;
    }

    if (lua_pcall(this->state, 0, 0, 0)) {
        bail(this->state, "Failed to load " + path);
    }
    // -- File loaded successfully.
}

void LuaPlugin::TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, std::string function, std::string args) {
    std::lock_guard<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(state, function.c_str());
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, static_cast<lua_Integer>(mapId));
        lua_pushinteger(state, static_cast<lua_Integer>(sizeX));
        lua_pushinteger(state, static_cast<lua_Integer>(sizeY));
        lua_pushinteger(state, static_cast<lua_Integer>(sizeZ));
        lua_pushstring(state, args.c_str());
        if (lua_pcall(state, 5, 0, 0)) {
            bail(state, "Failed to run mapfill");
        }
    }
}

void LuaPlugin::TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, std::string function) {
    std::lock_guard<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(state, function.c_str());
    if (!lua_checkstack(state, 1)) {
        Logger::LogAdd("Lua", "State is fucked", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, static_cast<lua_Integer>(mapId));
        lua_pushinteger(state, static_cast<lua_Integer>(X));
        lua_pushinteger(state, static_cast<lua_Integer>(Y));
        lua_pushinteger(state, static_cast<lua_Integer>(Z));
        if (lua_pcall(state, 4, 0, 0) != 0) {
            bail(state, "Failed to trig physics;");
        }
    }
}

void LuaPlugin::TriggerCommand(std::string function, int clientId, std::string parsedCmd, std::string text0,
                               std::string text1, std::string op1, std::string op2, std::string op3, std::string op4,
                               std::string op5) {
    std::lock_guard<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(state, function.c_str());
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, static_cast<lua_Integer>(clientId));
        lua_pushstring(state, parsedCmd.c_str());
        lua_pushstring(state, text0.c_str());
        lua_pushstring(state, text1.c_str());
        lua_pushstring(state, op1.c_str());
        lua_pushstring(state, op2.c_str());
        lua_pushstring(state, op3.c_str());
        lua_pushstring(state, op4.c_str());
        lua_pushstring(state, op5.c_str());
        if (lua_pcall(state, 9, 0, 0)) {
            bail(state, "Failed to run command.");
        }
    }
}

void LuaPlugin::TriggerBuildMode(std::string function, int clientId, int mapId, unsigned short X, unsigned short Y,
                                 unsigned short Z, unsigned char mode, unsigned char block) {
    std::lock_guard<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(state, function.c_str());
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, clientId);
        lua_pushinteger(state, mapId);
        lua_pushinteger(state, X);
        lua_pushinteger(state, Y);
        lua_pushinteger(state, Z);
        lua_pushinteger(state, mode);
        lua_pushinteger(state, block);
        if (lua_pcall(state, 7, 0, 0)) {
            bail(state, "Failed to run command.");
        }
    }
}

int LuaPlugin::LuaMapBlockGetPlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Get_Player_Last called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int X = lua_tointeger(L, 2);
    int Y = lua_tointeger(L, 3);
    int Z = lua_tointeger(L, 4);

    int result = -1;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        result = map->GetBlockPlayer(X, Y, Z);
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaClientGetTable(lua_State *L) {
    Network* nm = Network::GetInstance();
    int numClients = nm->roClients.size();
    int index = 1;

    lua_newtable(L);

    if (numClients > 0) {
        for (auto const &nc : nm->roClients) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, nc->Id);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numClients);

    return 2;
}

int LuaPlugin::LuaClientGetMapId(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Client_Get_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int result = -1;
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);
    if (client != nullptr) {
        result = client->player->MapId;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaClientGetIp(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CLient_Get_IP called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    std::string result = "";
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);
    if (client != nullptr) {
        result = client->IP;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlugin::LuaClientGetLoginName(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CLient_Get_Login_Name called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    std::string result = "";
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);

    if (client != nullptr) {
        result = client->player->LoginName;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlugin::LuaClientGetLoggedIn(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CLient_Get_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int result = -1;

    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);

    if (client != nullptr) {
        result = client->LoggedIn;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaClientGetEntity(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CLient_Get_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int result = -1;
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);

    if (client != nullptr) {
        if (client->player != nullptr && client->player->tEntity != nullptr)
            result = client->player->tEntity->Id;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaEntityGetTable(lua_State *L) {
    int numEntities = Entity::_entities.size();
    int index = 1;

    lua_newtable(L);

    if (numEntities > 0) {
        for (auto const &e : Entity::_entities) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, e.first);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numEntities);

    return 2;
}

int LuaPlugin::LuaEntityAdd(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: Entity_Add called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    std::string eName(lua_tostring(L, 1));
    int mapId = lua_tointeger(L, 2);
    float x = lua_tonumber(L, 3);
    float y = lua_tonumber(L, 4);
    float z = lua_tonumber(L, 5);
    float rotation = lua_tonumber(L, 6);
    float look = lua_tonumber(L, 7);
    std::shared_ptr<Entity> newEntity = std::make_shared<Entity>(eName, mapId, x, y, z, rotation, look);
    Entity::Add(newEntity);
    int result = newEntity->Id;

    lua_pushinteger(L, result);

    return 1;
}

int LuaPlugin::LuaEntityDelete(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Entity_Delete called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    Entity::Delete(lua_tointeger(L, 1));

    return 0;
}

int LuaPlugin::LuaBuildModeSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    std::string buildMode(lua_tostring(L, 2));

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetMode(clientId, buildMode);

    return 0;
}

int LuaPlugin::LuaBuildModeGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> networkClient= nm->GetClient(clientId);

    if (networkClient != nullptr) {
        std::string clientBuildMode = networkClient->player->tEntity->BuildMode;
        lua_pushstring(L, clientBuildMode.c_str());
    } else {
        lua_pushstring(L, "");
    }

    return 1;
}

int LuaPlugin::LuaBuildModeStateSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_State_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int buildState = lua_tointeger(L, 2);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetState(clientId, buildState);

    return 0;
}

int LuaPlugin::LuaBuildModeStateGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_State_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> networkClient= nm->GetClient(clientId);

    if (networkClient != nullptr) {
        lua_pushinteger(L, networkClient->player->tEntity->BuildState);
    } else {
        lua_pushinteger(L, -1);
    }

    return 1;
}

int LuaPlugin::LuaBuildModeCoordinateSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 5) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Coordinate_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    float X = lua_tonumber(L, 3);
    float Y = lua_tonumber(L, 4);
    float Z = lua_tonumber(L, 5);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetCoordinate(clientId, index, X, Y, Z);

    return 0;
}

int LuaPlugin::LuaBuildModeCoordinateGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Coordinate_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    int X = -1;
    int Y = -1;
    int Z = -1;

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    X = buildModeMain->GetCoordinateX(clientId, index);
    Y = buildModeMain->GetCoordinateY(clientId, index);
    Z = buildModeMain->GetCoordinateZ(clientId, index);

    lua_pushinteger(L, X);
    lua_pushinteger(L, Y);
    lua_pushinteger(L, Z);

    return 3;
}

int LuaPlugin::LuaBuildModeLongSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Long_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    int X = lua_tonumber(L, 3);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetInt(clientId, index, X);

    return 0;
}

int LuaPlugin::LuaBuildModeLongGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Long_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    int val = -1;

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    val = buildModeMain->GetInt(clientId, index);

    lua_pushinteger(L, val);
    return 1;
}

int LuaPlugin::LuaBuildModeFloatSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_float_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    float X = lua_tonumber(L, 3);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetFloat(clientId, index, X);
    return 0;
}

int LuaPlugin::LuaBuildModeFloatGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Float_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    float val = -1;

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    val = buildModeMain->GetFloat(clientId, index);

    lua_pushnumber(L, static_cast<lua_Number>(val));
    return 1;
}

int LuaPlugin::LuaBuildModeStringSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_String_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    std::string val(lua_tostring(L, 3));

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetString(clientId, index, val);

    return 0;
}

int LuaPlugin::LuaBuildModeStringGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_String_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    std::string val = "";

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    val = buildModeMain->GetString(clientId, index);

    lua_pushstring(L, val.c_str());
    return 1;
}

int LuaPlugin::LuaLanguageGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: Lang_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    std::string language(lua_tostring(L, 1));
    std::string input(lua_tostring(L, 2));
//    if (n)
//    std::string Field0(lua_tostring(L, 3));
//    std::string Field1(lua_tostring(L, 4));
//    std::string Field2(lua_tostring(L, 5));
//    std::string Field3(lua_tostring(L, 6));

    lua_pushstring(L, input.c_str());
    return 1;
}

int LuaPlugin::LuaEntityGetPlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_PLAYER called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    int result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->playerList->Number;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaEntityGetMapId(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    int result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->MapID;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaEntityGetX(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_X called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->X;
    }

    lua_pushnumber(L, static_cast<lua_Number>(result));
    return 1;
}

int LuaPlugin::LuaEntityGetY(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Y called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->Y;
    }

    lua_pushnumber(L, static_cast<lua_Number>(result));
    return 1;
}

int LuaPlugin::LuaEntityGetZ(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Z called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->Z;
    }

    lua_pushnumber(L, static_cast<lua_Number>(result));
    return 1;
}

int LuaPlugin::LuaEntityGetRotation(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Rotation called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->Rotation;
    }

    lua_pushnumber(L, static_cast<lua_Number>(result));
    return 1;
}

int LuaPlugin::LuaEntityGetLook(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Look called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->Look;
    }

    lua_pushnumber(L, result);
    return 1;
}

int LuaPlugin::LuaEntityResend(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Entity_Resend called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        foundEntity->Resend(entityId);
    }

    return 0;
}

int LuaPlugin::LuaEntityMessage2Clients(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Entity_Message_2_Clients called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityid = lua_tointeger(L, 1);
    std::string message(lua_tostring(L, 2));

    Entity::MessageToClients(entityid, message);
    return 0;
}

int LuaPlugin::LuaEntityDisplaynameGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_displayname_get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
       // std::string prefix(lua_tostring(L, 2));
       // std::string displayName(lua_tostring(L, 3));
       // std::string suffix(lua_tostring(L, 4));

        lua_pushstring(L, foundEntity->Prefix.c_str());
        lua_pushstring(L, foundEntity->Name.c_str());
        lua_pushstring(L, foundEntity->Suffix.c_str());
    }

    return 3;
}

int LuaPlugin::LuaEntityDisplaynameSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_displayname_set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    std::string prefix(lua_tostring(L, 2));
    std::string displayName(lua_tostring(L, 3));
    std::string suffix(lua_tostring(L, 4));

    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        Entity::SetDisplayName(entityId, prefix, displayName, suffix);
    }

    return 0;
}

int LuaPlugin::LuaEntityPositionSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_Position_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    float X = lua_tonumber(L, 3);
    float Y = lua_tonumber(L, 4);
    float Z = lua_tonumber(L, 5);
    float rotation = lua_tonumber(L, 6);
    float look = lua_tonumber(L, 7);

    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        foundEntity->PositionSet(mapId, X, Y, Z, rotation, look, 10, true);
    }

    return 0;
}

int LuaPlugin::LuaEntityKill(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_Kill called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        foundEntity->Kill();
    }

    return 0;
}

int LuaPlugin::LuaBuildLinePlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 12) {
        Logger::LogAdd("Lua", "LuaError: Build_Line_Player called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int playerNumber = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int x0 = lua_tointeger(L, 3);
    int y0 = lua_tointeger(L, 4);
    int z0 = lua_tointeger(L, 5);
    int x1 = lua_tointeger(L, 6);
    int y1 = lua_tointeger(L, 7);
    int z1 = lua_tointeger(L, 8);

    short material = lua_tointeger(L, 9);
    unsigned char priority = lua_tointeger(L, 10);
    bool undo = (lua_tointeger(L, 11) > 0);
    bool physics = lua_tointeger(L, 12);

    Build::BuildLinePlayer(static_cast<short>(playerNumber), mapId, x0, y0, z0, x1, y1, z1, material, priority, undo, physics);
    return 0;
}

int LuaPlugin::LuaBuildBoxPlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 14) {
        Logger::LogAdd("Lua", "LuaError: Build_Box_Player called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int x0 = lua_tointeger(L, 3);
    int y0 = lua_tointeger(L, 4);
    int z0 = lua_tointeger(L, 5);
    int x1 = lua_tointeger(L, 6);
    int y1 = lua_tointeger(L, 7);
    int z1 = lua_tointeger(L, 8);

    short material = lua_tointeger(L, 9);
    short replaceMaterial = lua_tointeger(L, 10);
    bool hollow = (lua_tointeger(L, 11) > 0);
    unsigned char priority = lua_tointeger(L, 12);
    bool undo = (lua_tointeger(L, 13) > 0);
    bool physics = lua_toboolean(L, 14);

    Build::BuildBoxPlayer(playerNumber, mapId, x0, y0, z0, x1, y1, z1, material, replaceMaterial, hollow, priority, undo, physics);
    return 0;
}

int LuaPlugin::LuaBuildSpherePlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 12) {
        Logger::LogAdd("Lua", "LuaError: Build_Sphere_Player called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int playerNumber = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int x = lua_tointeger(L, 3);
    int y = lua_tointeger(L, 4);
    int z = lua_tointeger(L, 5);
    float radius = lua_tonumber(L, 6);
    short material = lua_tointeger(L, 7);
    short replaceMaterial = lua_tointeger(L, 8);
    bool hollow = (lua_tointeger(L, 9) > 0);
    unsigned char priority = lua_tointeger(L, 10);
    bool undo = (lua_tointeger(L, 11) > 0);
    bool physics = lua_toboolean(L, 12);

    Build::BuildSpherePlayer(playerNumber, mapId, x, y, z, radius, material, replaceMaterial, hollow, priority, undo, physics);

    return 0;
}

int LuaPlugin::LuaBuildRankBox(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 9) {
        Logger::LogAdd("Lua", "LuaError: Build_Rank_Box called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    int X0 = lua_tointeger(L, 2);
    int Y0 = lua_tointeger(L, 3);
    int Z0 = lua_tointeger(L, 4);
    int X1 = lua_tointeger(L, 5);
    int Y1 = lua_tointeger(L, 6);
    int Z1 = lua_tointeger(L, 7);
    int Rank = lua_tointeger(L, 8);
    int MaxRank = lua_tointeger(L, 9);

    Build::BuildRankBox(mapId, X0, Y0, Z0, X1, Y1, Z1, Rank, MaxRank);

    return 0;
}

int LuaPlugin::LuaPlayerGetTable(lua_State *L) {
    Player_List* pll = Player_List::GetInstance();
    int numEntities = pll->_pList.size();
    int index = 1;

    lua_newtable(L);

    if (numEntities > 0) {
        for (auto const &e :  pll->_pList) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, e.Number);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numEntities);

    return 2;
}

int LuaPlugin::LuaPlayerGetPrefix(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Prefix called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    std::string result = "";

    if (ple != nullptr) {
        RankItem ri = rm->GetRank(ple->PRank, false);
        result = ri.Prefix;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlugin::LuaPlayerGetName(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Name called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    std::string result = "";

    if (ple != nullptr) {

        result = ple->Name;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlugin::LuaPlayerGetSuffix(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Suffix called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    std::string result = "";

    if (ple != nullptr) {
        RankItem ri = rm->GetRank(ple->PRank, false);
        result = ri.Suffix;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}
int LuaPlugin::LuaPlayerGetIp(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Ip called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    std::string result = "";

    if (ple != nullptr) {
        result = ple->IP;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlugin::LuaPlayerGetOntime(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Ontime called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    int result = -1;

    if (ple != nullptr) {
        result = ple->OntimeCounter;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaPlayerGetRank(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Rank called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    int result = -1;

    if (ple != nullptr) {
        result = ple->PRank;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaPlayerGetOnline(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Online called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    int result = 0;

    if (ple != nullptr) {
        result = ple->Online;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaPlayerGetMuteTime(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Mute_Time called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    int result = -1;

    if (ple != nullptr) {
        result = ple->MuteTime;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaPlayerSetRank(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Player_Set_Rank called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    int rankNumber = lua_tointeger(L, 2);
    std::string reason(lua_tostring(L, 3));

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->SetRank(rankNumber, reason);
    }

    return 0;
}

int LuaPlugin::LuaPlayerKick(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs > 2) {
        Logger::LogAdd("Lua", "LuaError: Player_Kick called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    std::string reason(lua_tostring(L, 2));
    int count = 0;
    bool log = true;
    bool show = true;

    if (nArgs >= 3)
        count = lua_tonumber(L, 3);

    if (nArgs >= 4)
        log = (lua_tonumber(L, 4) > 0);

    if (nArgs >= 5)
        show = (lua_tonumber(L, 5) > 0);

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Kick(reason, count, log, show);
    }

    return 0;
}

int LuaPlugin::LuaPlayerBan(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Player_Ban called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    std::string reason(lua_tostring(L, 2));

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Ban(reason);
    }

    return 0;
}

int LuaPlugin::LuaPlayerUnban(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Unban called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Unban();
    }

    return 0;
}

int LuaPlugin::LuaPlayerStop(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Player_Stop called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    std::string reason(lua_tostring(L, 2));

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Stop(reason);
    }

    return 0;
}

int LuaPlugin::LuaPlayerUnstop(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Unstop called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Unstop();
    }

    return 0;
}

int LuaPlugin::LuaPlayerMute(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Player_Mute called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    int minutes = lua_tointeger(L, 2);
    std::string reason(lua_tostring(L, 3));

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Mute(minutes, reason);
    }

    return 0;
}

int LuaPlugin::LuaPlayerUnmute(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Unmute called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Unmute();
    }

    return 0;
}

int LuaPlugin::LuaServerGetExtension(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaEventAdd(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 6) {
        Logger::LogAdd("Lua", "LuaError: Event_Add called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    std::string eventId(lua_tostring(L, 1));
    std::string function(lua_tostring(L, 2));
    std::string type(lua_tostring(L, 3));
    int setOrCheck = lua_tointeger(L, 4);
    int timed = lua_tointeger(L, 5);
    int mapId = lua_tointeger(L, 6);

    if (!Dispatcher::hasdescriptor(type)) {
        Logger::LogAdd("Lua", "LuaError: Invalid event type: " + type + ".", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    
    auto typeAsEvent = Dispatcher::getDescriptor(type);

    LuaEvent newEvent {
            function,
            typeAsEvent,
            clock(),
            timed
    };

    if (setOrCheck == 1) {
        if (_luaEvents.find(typeAsEvent) != _luaEvents.end()) {
            _luaEvents[typeAsEvent].push_back(newEvent);
        } else {
            _luaEvents.insert(std::make_pair(typeAsEvent, std::vector<LuaEvent>()));
            _luaEvents[typeAsEvent].push_back(newEvent);
        }
    } else {
        bool eventExists = false;

        if (_luaEvents.find(typeAsEvent) != _luaEvents.end()) {
            for(const auto &i : _luaEvents[typeAsEvent]) {
                if (i.type == typeAsEvent && i.functionName == function) {
                    eventExists = true;
                    break;
                }
            }
        } 

        lua_pushboolean(L, eventExists);
        return 1;
    }

    return 0;
}

int LuaPlugin::LuaEventDelete(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Event_Delete called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    std::string eventId(lua_tostring(L, 1));
    // -- TODO:
    return 0;
}

int LuaPlugin::LuaMapBlockMove(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 10) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Move called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    int x0 = lua_tointeger(L, 2);
    int y0 = lua_tointeger(L, 3);
    int z0 = lua_tointeger(L, 4);
    int x1 = lua_tointeger(L, 5);
    int y1 = lua_tointeger(L, 6);
    int z1 = lua_tointeger(L, 7);
    bool undo = lua_toboolean(L, 8);
    bool physic = lua_toboolean(L, 9);
    unsigned char priority = lua_tointeger(L, 10);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        map->BlockMove(x0, y0, z0, x1, y1, z1, undo, physic, priority);
    }

    return 0;
}

int LuaPlugin::LuaMapGetName(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Name called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        lua_pushstring(L, map->data.Name.c_str());
        return 1;
    }

    return 0;
}

int LuaPlugin::LuaMapGetDimensions(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Dimensions called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        lua_pushinteger(L, map->data.SizeX);
        lua_pushinteger(L, map->data.SizeY);
        lua_pushinteger(L, map->data.SizeZ);
        return 3;
    }

    return 0;
}

int LuaPlugin::LuaMapGetTable(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 0) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Table() called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    MapMain* mm = MapMain::GetInstance();
    int numEntities = mm->_maps.size();
    int index = 1;

    lua_newtable(L);

    if (numEntities > 0) {
        for (auto const &e :  mm->_maps) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, e.second->data.ID);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numEntities);

    return 2;
}

int LuaPlugin::LuaMapBlockChangePlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 10) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Change_Client called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerId = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int X = lua_tointeger(L, 3);
    int Y = lua_tointeger(L, 4);
    int Z = lua_tointeger(L, 5);
    unsigned char type = lua_tointeger(L, 6);
    bool undo = lua_toboolean(L, 7);
    bool physic = lua_toboolean(L, 8);
    bool send = lua_toboolean(L, 9);
    unsigned char priority = lua_tointeger(L, 10);

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* pEntry = pll->GetPointer(playerId);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (pEntry == nullptr || map== nullptr) {
        return 0;
    }

    map->BlockChange(playerId, X, Y, Z, type, undo, physic, send, priority);
    return 0;
}

int LuaPlugin::LuaMapBlockGetRank(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Get_Rank called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int X = lua_tointeger(L, 2);
    int Y = lua_tointeger(L, 3);
    int Z = lua_tointeger(L, 4);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    int result = map->BlockGetRank(X, Y, Z);
    lua_pushinteger(L, result);

    return 1;
}

int LuaPlugin::LuaMapGetUniqueId(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Unique_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushstring(L, map->data.UniqueID.c_str());
    return 1;
}

int LuaPlugin::LuaMapGetDirectory(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Directory called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushstring(L, map->data.Directory.c_str());
    return 1;
}

int LuaPlugin::LuaMapGetRankBuild(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Rank_Build called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushinteger(L, map->data.RankBuild);
    return 1;
}

int LuaPlugin::LuaMapGetRankShow(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Rank_Show called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushinteger(L, map->data.RankShow);
    return 1;
}

int LuaPlugin::LuaMapGetRankJoin(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Rank_Join called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushinteger(L, map->data.RankJoin);
    return 1;
}

int LuaPlugin::LuaMapGetSpawn(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Spawn called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushnumber(L, map->data.SpawnX);
    lua_pushnumber(L, map->data.SpawnY);
    lua_pushnumber(L, map->data.SpawnZ);
    lua_pushnumber(L, map->data.SpawnRot);
    lua_pushnumber(L, map->data.SpawnLook);
    return 5;
}

int LuaPlugin::LuaMapGetSaveInterval(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Get_Save_Intervall called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    lua_pushinteger(L, map->data.SaveInterval);
    return 1;
}

int LuaPlugin::LuaMapSetName(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Name called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    std::string newName(lua_tostring(L, 2));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.Name = newName;
    return 0;
}

int LuaPlugin::LuaMapSetDirectory(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Directory called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    std::string newName(lua_tostring(L, 2));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.Directory = newName;
    return 0;
}

int LuaPlugin::LuaMapSetRankBuild(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Rank_Build called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int newRank = lua_tointeger(L, 2);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.RankBuild = newRank;
    return 0;
}

int LuaPlugin::LuaMapSetRankJoin(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Rank_Join called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int newRank = lua_tointeger(L, 2);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.RankJoin = newRank;
    return 0;
}

int LuaPlugin::LuaMapSetRankShow(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Rank_Show called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int newRank = lua_tointeger(L, 2);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.RankShow = newRank;
    return 0;
}

int LuaPlugin::LuaMapSetSpawn(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 6) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Spawn called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    float newX = lua_tonumber(L, 2);
    float newY = lua_tonumber(L, 3);
    float newZ = lua_tonumber(L, 4);
    float newRot = lua_tonumber(L, 5);
    float newLook = lua_tonumber(L, 6);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.SpawnX = newX;
    map->data.SpawnY = newY;
    map->data.SpawnZ = newZ;
    map->data.SpawnRot = newRot;
    map->data.SpawnLook = newLook;

    return 0;
}

int LuaPlugin::LuaMapSetSaveInterval(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Set_Save_Interval called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int saveInterval = lua_tointeger(L, 2);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr) {
        return 0;
    }

    map->data.SaveInterval = saveInterval;
    return 0;
}

int LuaPlugin::LuaMapAdd(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 5) {
        Logger::LogAdd("Lua", "LuaError: Map_Add called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int sizeX = lua_tointeger(L, 2);
    int sizeY = lua_tointeger(L, 3);
    int sizeZ = lua_tointeger(L, 4);
    std::string name(lua_tostring(L, 5));
    MapMain* mm = MapMain::GetInstance();

    int resultId = mm->Add(mapId, sizeX, sizeY, sizeZ, name);
    lua_pushinteger(L, resultId);
    return 1;
}

int LuaPlugin::LuaMapActionAddResize(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map_Action_Add_Resize called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int sizeX = lua_tointeger(L, 2);
    int sizeY = lua_tointeger(L, 3);
    int sizeZ = lua_tointeger(L, 4);
    MapMain* mm = MapMain::GetInstance();

    mm->AddResizeAction(0, mapId, sizeX, sizeY, sizeZ);

    return 0;
}

int LuaPlugin::LuaMapActionAddFill(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Map_Action_Add_Fill called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    std::string functionName(lua_tostring(L, 2));
    std::string argumentString(lua_tostring(L, 3));
    MapMain* mm = MapMain::GetInstance();

    mm->AddFillAction(0, mapId, functionName, argumentString);
    return 0;
}

int LuaPlugin::LuaMapActionAddSave(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Map_Action_Add_Save called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    std::string directory(lua_tostring(L, 2));
    MapMain* mm = MapMain::GetInstance();

    mm->AddSaveAction(0, mapId, directory);
    return 0;
}

int LuaPlugin::LuaMapActionAddDelete(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Action_Add_Delete called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    mm->AddDeleteAction(0, mapId);

    return 0;
}

int LuaPlugin::LuaMapResend(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Map_Resend called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> givenMap = mm->GetPointer(mapId);

    if (givenMap != nullptr) {
        givenMap->Resend();
    }

    return 0;
}

int LuaPlugin::LuaMapExport(lua_State *L) {
        int nArgs = lua_gettop(L);

    if (nArgs != 8) {
        Logger::LogAdd("Lua", "LuaError: Map_Export called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    Vector3S startLoc;
    Vector3S endLoc;
    int mapId = lua_tointeger(L, 1);
    startLoc.X = lua_tointeger(L, 2);
    startLoc.Y = lua_tointeger(L, 3);
    startLoc.Z = lua_tointeger(L, 4);
    endLoc.X = lua_tointeger(L, 5);
    endLoc.Y = lua_tointeger(L, 6);
    endLoc.Z = lua_tointeger(L, 7);
    std::string fileName(lua_tostring(L, 8));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> givenMap = mm->GetPointer(mapId);
    
    MinecraftLocation start;
    start.SetAsBlockCoords(startLoc);
    
    MinecraftLocation end;
    end.SetAsBlockCoords(endLoc);

    if (givenMap != nullptr) {
        givenMap->MapExport(start, end, fileName);
    }
    return 0;
}

int LuaPlugin::LuaMapExportGetSize(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaMapImportPlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 9) {
        Logger::LogAdd("Lua", "LuaError: Map_Import called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int playerNumber = lua_tointeger(L, 1);
    std::string fileName(lua_tostring(L, 2));
    int mapId = lua_tointeger(L, 3);
    Vector3S placeLoc;
    placeLoc.X = lua_tointeger(L, 4);
    placeLoc.Y = lua_tointeger(L, 5);
    placeLoc.Z = lua_tointeger(L, 6);
    short scaleX = lua_tointeger(L, 7);
    short scaleY = lua_tointeger(L, 8);
    short scaleZ = lua_tointeger(L, 9);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> givenMap = mm->GetPointer(mapId);
    
    MinecraftLocation start;
    start.SetAsBlockCoords(placeLoc);

     if (givenMap != nullptr) {
        givenMap->MapImport(fileName, start, scaleX, scaleY, scaleZ);
    }

    return 0;
}

int LuaPlugin::LuaBlockGetTable(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 0) {
        Logger::LogAdd("Lua", "LuaError: BlocK_Get_Table() called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    Block* bm = Block::GetInstance();
    int numEntities = bm->Blocks.size();
    int index = 1;

    lua_newtable(L);

    if (numEntities > 0) {
        for (auto const &e :  bm->Blocks) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, e.Id);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numEntities);

    return 2;
}

int LuaPlugin::LuaBlockGetName(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Block_Get_Name() called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int blockId = lua_tointeger(L, 1);
    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);
    lua_pushstring(L, be.Name.c_str());

    return 1;
}

int LuaPlugin::LuaBlockGetRankPlace(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Block_Get_Rank_Place() called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int blockId = lua_tointeger(L, 1);
    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);
    lua_pushinteger(L, be.RankPlace);

    return 1;
}

int LuaPlugin::LuaBlockGetRankDelete(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Block_Get_Rank_Delete() called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int blockId = lua_tointeger(L, 1);
    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);
    lua_pushinteger(L, be.RankDelete);

    return 1;
}

int LuaPlugin::LuaBlockGetClientType(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Block_Get_Client_Type() called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int blockId = lua_tointeger(L, 1);
    Block* bm = Block::GetInstance();
    MapBlock be = bm->GetBlock(blockId);
    lua_pushinteger(L, be.OnClient);

    return 1;
}

int LuaPlugin::LuaClientGetExtension(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Block_Get_Client_Type() called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    std::string extension(lua_tostring(L, 2));
    int result = 0;
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(clientId);

    if (c == nullptr) {
        if (c->LoggedIn && c->CPE) {
            result = CPE::GetClientExtVersion(c, extension);
        }
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaFileGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Files_File_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    std::string fileName(lua_tostring(L, 1));
    Files* f = Files::GetInstance();
    lua_pushstring(L, f->GetFile(fileName).c_str());
    return 1;
}


int LuaPlugin::LuaFolderGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Files_Folder_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    std::string fileName(lua_tostring(L, 1));
    Files* f = Files::GetInstance();
    lua_pushstring(L, f->GetFolder(fileName).c_str());
    return 1;
}

int LuaPlugin::LuaRankGetTable(lua_State *L) {
    Rank* rm = Rank::GetInstance();
    int numRanks = rm->_ranks.size();
    int index = 1;

    lua_newtable(L);

    if (numRanks > 0) {
        for (auto const &nc : rm->_ranks) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, nc.first);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numRanks);

    return 2;
}

int LuaPlugin::LuaRankAdd(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Rank_Add called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int rankNumber = lua_tointeger(L, 1);
    std::string rankName(lua_tostring(L, 2));
    std::string rankPrefix(lua_tostring(L, 3));
    std::string rankSuffix(lua_tostring(L, 4));
    RankItem newRankItem;
    newRankItem.Name = rankName;
    newRankItem.Rank = rankNumber;
    newRankItem.Prefix = rankPrefix;
    newRankItem.Suffix = rankSuffix;
    
    Rank* r = Rank::GetInstance();
    r->Add(newRankItem);
    return 0;
}

int LuaPlugin::LuaRankDelete(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Rank_Delete called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int rankNumber = lua_tointeger(L, 1);
    int isExact = lua_tointeger(L, 2);

    Rank* r = Rank::GetInstance();
    r->Delete(rankNumber, isExact > 0);

    return 0;
}

int LuaPlugin::LuaRankGetName(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Rank_Get_Name called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int rankNumber = lua_tointeger(L, 1);
    int isExact = lua_tointeger(L, 2);

    Rank* r = Rank::GetInstance();
    RankItem ri = r->GetRank(rankNumber, isExact > 0);
    lua_pushstring(L, ri.Name.c_str());

    return 1;
}

int LuaPlugin::LuaRankGetPrefix(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Rank_Get_Prefix called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int rankNumber = lua_tointeger(L, 1);
    int isExact = lua_tointeger(L, 2);

    Rank* r = Rank::GetInstance();
    RankItem ri = r->GetRank(rankNumber, isExact > 0);
    lua_pushstring(L, ri.Prefix.c_str());

    return 1;
}

int LuaPlugin::LuaRankGetSuffix(lua_State *L) {
     int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Rank_Get_Suffix called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int rankNumber = lua_tointeger(L, 1);
    int isExact = lua_tointeger(L, 2);

    Rank* r = Rank::GetInstance();
    RankItem ri = r->GetRank(rankNumber, isExact > 0);
    lua_pushstring(L, ri.Suffix.c_str());

    return 1;
}

int LuaPlugin::LuaRankGetRoot(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Rank_Get_Name called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int rankNumber = lua_tointeger(L, 1);
    int isExact = lua_tointeger(L, 2);

    Rank* r = Rank::GetInstance();
    RankItem ri = r->GetRank(rankNumber, isExact > 0);
    lua_pushinteger(L, ri.Rank);

    return 1;
}

int LuaPlugin::LuaTeleporterGetTable(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Teleporter_Get_Table called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> chosenMap = mm->GetPointer(mapId);

    if (chosenMap == nullptr)
        return 0;

    int numRanks = chosenMap->data.Teleporter.size();
    int index = 1;

    lua_newtable(L);

    if (numRanks > 0) {
        for (auto const &nc : chosenMap->data.Teleporter) {
            lua_pushinteger(L, index++);
            lua_pushstring(L, nc.second.Id.c_str());
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numRanks);

    return 2;
}

int LuaPlugin::LuaTeleporterGetBox(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Teleporter_Get_Box called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    std::string tpId(lua_tostring(L, 2));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> chosenMap = mm->GetPointer(mapId);

    if (chosenMap == nullptr)
        return 0;
    
    if (chosenMap->data.Teleporter.find(tpId) == chosenMap->data.Teleporter.end())
        return 0;

    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].X0);
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].Y0);
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].Z0);
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].X1);
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].Y1);
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].Z1);
    return 6;
}

int LuaPlugin::LuaTeleporterGetDestination(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Teleporter_Get_Destination called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    std::string tpId(lua_tostring(L, 2));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> chosenMap = mm->GetPointer(mapId);

    if (chosenMap == nullptr)
        return 0;
    
    if (chosenMap->data.Teleporter.find(tpId) == chosenMap->data.Teleporter.end())
        return 0;

    lua_pushstring(L, chosenMap->data.Teleporter[tpId].DestMapUniqueId.c_str());
    lua_pushinteger(L, chosenMap->data.Teleporter[tpId].DestMapId);
    lua_pushnumber(L, chosenMap->data.Teleporter[tpId].DestX);
    lua_pushnumber(L, chosenMap->data.Teleporter[tpId].DestY);
    lua_pushnumber(L, chosenMap->data.Teleporter[tpId].DestZ);
    lua_pushnumber(L, chosenMap->data.Teleporter[tpId].DestRot);
    lua_pushnumber(L, chosenMap->data.Teleporter[tpId].DestLook);
    return 7;
}

int LuaPlugin::LuaTeleporterAdd(lua_State *L) {
        int nArgs = lua_gettop(L);

    if (nArgs != 15) {
        Logger::LogAdd("Lua", "LuaError: Teleporter_Add called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    std::string tpId(lua_tostring(L, 2));
    Vector3S start;
    Vector3S end;
    start.X = lua_tointeger(L, 3);
    start.Y = lua_tointeger(L, 4);
    start.Z = lua_tointeger(L, 5);

    end.X = lua_tointeger(L, 6);
    end.Y = lua_tointeger(L, 7);
    end.Z = lua_tointeger(L, 8);

    std::string destMapUniqueId(lua_tostring(L, 9));
    int destMapId = lua_tointeger(L, 10);
    float DestX = lua_tonumber(L, 11);
    float DestY = lua_tonumber(L, 12);
    float DestZ = lua_tonumber(L, 13);
    float DestRot = lua_tonumber(L, 14);
    float DestLook = lua_tonumber(L, 15);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> chosenMap = mm->GetPointer(mapId);

    if (chosenMap == nullptr)
        return 0;
    
    if (chosenMap->data.Teleporter.find(tpId) != chosenMap->data.Teleporter.end())
        return 0;

    MapTeleporterElement newTp {
        tpId,
        start.X,
        start.Y,
        start.Z,
        end.X,
        end.Y,
        end.Z,
        destMapUniqueId,
        destMapId,
        DestX,
        DestY,
        DestZ,
        DestRot,
        DestLook
    };

    chosenMap->data.Teleporter.insert(std::make_pair(tpId, newTp));

    return 0;
}

int LuaPlugin::LuaTeleporterDelete(lua_State *L) {
     int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Teleporter_Delete called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    std::string tpId(lua_tostring(L, 2));

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> chosenMap = mm->GetPointer(mapId);

    if (chosenMap == nullptr)
        return 0;
    
    if (chosenMap->data.Teleporter.find(tpId) == chosenMap->data.Teleporter.end())
        return 0;
    
    chosenMap->data.Teleporter.erase(tpId);
    return 0;
}

int LuaPlugin::LuaServerGetExtensions(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaClientGetExtensions(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Client_Get_Extensions called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);
    
    if (client == nullptr || !client->LoggedIn)
        return 0;

    int number = client->Extensions.size();
     lua_newtable(L);

    if (number > 0) {
        for (auto const &nc : client->Extensions) {
            lua_pushstring(L, nc.first.c_str());
            lua_pushinteger(L, nc.second);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, number);

    return 2;
}

int LuaPlugin::LuaSelectionCuboidAdd(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 13) {
        Logger::LogAdd("Lua", "LuaError: CPE_Selection_Cuboid_Add called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int selectionId = lua_tointeger(L, 2);
    std::string label(lua_tostring(L, 3));
    Vector3S start;
    Vector3S end;
    start.X = lua_tointeger(L, 4);
    start.Y = lua_tointeger(L, 5);
    start.Z = lua_tointeger(L, 6);
    end.X = lua_tointeger(L, 7);
    end.Y = lua_tointeger(L, 8);
    end.Z = lua_tointeger(L, 9);
    int red = lua_tointeger(L, 10);
    int green = lua_tointeger(L, 11);
    int blue = lua_tointeger(L, 12);
    float opacity = lua_tonumber(L, 13);

    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> nc = nm->GetClient(clientId);
    if (nc == nullptr)
        return 0;
    
    nc->CreateSelection(static_cast<unsigned char>(selectionId), label, start.X, start.Y, start.Z, end.X, end.Y, end.Z, red, green, blue, static_cast<short>(opacity));
    return 0;
}

int LuaPlugin::LuaSelectionCuboidDelete(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: CPE_Selection_Cuboid_Delete called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int selectionId = lua_tointeger(L, 2);
     Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> nc = nm->GetClient(clientId);
    if (nc == nullptr)
        return 0;

    nc->DeleteSelection(static_cast<unsigned char>(selectionId));
    return 0;
}

int LuaPlugin::LuaGetHeldBlock(lua_State *L) {
        int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CPE_Get_Held_Block called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> nc = nm->GetClient(clientId);
    if (nc == nullptr)
        return 0;

    if (nc->LoggedIn && nc->player && nc->player->tEntity) {
        lua_pushinteger(L, nc->player->tEntity->heldBlock);
        return 1;
    }

    return 0;
}

int LuaPlugin::LuaSetHeldBlock(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: CPE_Set_Held_Block called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int blockId = lua_tointeger(L, 2);
    int canChange = lua_tointeger(L, 3);

    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> nc = nm->GetClient(clientId);

    if (nc == nullptr)
        return 0;

    if (nc->LoggedIn && nc->player && nc->player->tEntity) {
        nc->HoldThis(blockId, canChange);
        return 1;
    }

    return 0;
}

int LuaPlugin::LuaChangeModel(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaSetWeather(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaMapSetEnvColors(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaClientSetBlockPermissions(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaMapEnvSet(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaClientHackcontrolSend(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaHotkeyAdd(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaHotkeyRemove(lua_State *L) {
    return 0;
}

int LuaPlugin::LuaMapHackcontrolSet(lua_State *L) {
    return 0;
}



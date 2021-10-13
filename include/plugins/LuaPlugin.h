#ifndef D3PP_LUA_PLUGIN_H
#define D3PP_LUA_PLUGIN_H

#include <string>
#include <chrono>
#include <memory>
#include <map>
#include <lua.hpp>
#include <EventSystem.h>

#include "TaskScheduler.h"

struct LuaFile {
    std::string FilePath;
    time_t LastLoaded;
};

struct LuaEvent {
    std::string functionName;
    Event::DescriptorType type;
    clock_t lastRun;
    long duration;
};

class LuaPlugin : TaskItem {
public:
    LuaPlugin();
    ~LuaPlugin();

    static LuaPlugin* Instance;
    static LuaPlugin* GetInstance();
    void TriggerCommand(const std::string& function, int clientId, const std::string& parsedCmd, const std::string &text0, const std::string &text1, const std::string& op1, const std::string &op2, const std::string &op3, const std::string &op4, const std::string &op5);
    void TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, const std::string& function, const std::string& args);
    void TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, const std::string& function);
    void TriggerBuildMode(const std::string &function, int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char block);
private:
    std::recursive_mutex executionMutex;
    lua_State* state;
    std::map<std::string, LuaFile> _files;
    std::map<Event::DescriptorType, std::vector<LuaEvent>> _luaEvents;

    static void Init();
    void TimerMain();
    void MainFunc();
    void BindFunctions();
    void LoadFile(const std::string& path);
    // -- Lua interface functions :)
    // -- Client Functions
    int LuaClientGetTable(lua_State *L);
    int LuaClientGetMapId(lua_State *L);
    int LuaClientGetIp(lua_State *L);
    int LuaClientGetLoginName(lua_State *L);
    int LuaClientGetLoggedIn(lua_State *L);
    int LuaClientGetEntity(lua_State *L);
    // -- Build Mode
    int LuaBuildModeSet(lua_State *L);
    int LuaBuildModeGet(lua_State *L);
    int LuaBuildModeStateSet(lua_State *L);
    int LuaBuildModeStateGet(lua_State *L);
    int LuaBuildModeCoordinateSet(lua_State *L);
    int LuaBuildModeCoordinateGet(lua_State *L);
    int LuaBuildModeLongSet(lua_State *L);
    int LuaBuildModeLongGet(lua_State *L);
    int LuaBuildModeFloatSet(lua_State *L);
    int LuaBuildModeFloatGet(lua_State *L);
    int LuaBuildModeStringSet(lua_State *L);
    int LuaBuildModeStringGet(lua_State *L);
    // -- Build Functions
    int LuaBuildLinePlayer(lua_State *L);
    int LuaBuildBoxPlayer(lua_State *L);
    int LuaBuildSpherePlayer(lua_State *L);
    int LuaBuildRankBox(lua_State *L);
    // -- Entity Functions
    int LuaEntityGetTable(lua_State *L);
    int LuaEntityAdd(lua_State *L);
    int LuaEntityDelete(lua_State *L);
    int LuaEntityGetPlayer(lua_State *L);
    int LuaEntityGetMapId(lua_State *L);
    int LuaEntityGetX(lua_State *L);
    int LuaEntityGetY(lua_State *L);
    int LuaEntityGetZ(lua_State *L);
    int LuaEntityGetRotation(lua_State *L);
    int LuaEntityGetLook(lua_State *L);
    int LuaEntityResend(lua_State *L);
    int LuaEntityMessage2Clients(lua_State *L);
    int LuaEntityDisplaynameGet(lua_State *L);
    int LuaEntityDisplaynameSet(lua_State *L);
    int LuaEntityPositionSet(lua_State *L);
    int LuaEntityKill(lua_State *L);
    // -- Player Functions
    int LuaPlayerGetTable(lua_State *L);
    int LuaPlayerGetPrefix(lua_State *L);
    int LuaPlayerGetName(lua_State *L);
    int LuaPlayerGetSuffix(lua_State *L);
    int LuaPlayerGetIp(lua_State *L);
    int LuaPlayerGetRank(lua_State *L);
    int LuaPlayerGetOnline(lua_State *L);
    int LuaPlayerGetOntime(lua_State *L);
    int LuaPlayerGetMuteTime(lua_State *L);
    int LuaPlayerSetRank(lua_State *L);
    int LuaPlayerKick(lua_State *L);
    int LuaPlayerBan(lua_State *L);
    int LuaPlayerUnban(lua_State *L);
    int LuaPlayerStop(lua_State *L);
    int LuaPlayerUnstop(lua_State *L);
    int LuaPlayerMute(lua_State *L);
    int LuaPlayerUnmute(lua_State *L);
    // -- Map functions
    int LuaMapGetTable(lua_State *L);
    int LuaMapBlockChange(lua_State *L);
    int LuaMapBlockChangeClient(lua_State *L);
    int LuaMapBlockChangePlayer(lua_State *L);
    int LuaMapBlockMove(lua_State *L);
    int LuaMapBlockSend(lua_State *L);
    int LuaMapBlockGetType(lua_State *L);
    int LuaMapBlockGetRank(lua_State *L);
    int LuaMapBlockGetPlayer(lua_State *L);
    int LuaMapGetName(lua_State *L);
    int LuaMapGetUniqueId(lua_State *L);
    int LuaMapGetDirectory(lua_State *L);
    int LuaMapGetRankBuild(lua_State *L);
    int LuaMapGetRankShow(lua_State *L);
    int LuaMapGetRankJoin(lua_State *L);
    int LuaMapGetDimensions(lua_State *L);
    int LuaMapGetSpawn(lua_State *L);
    int LuaMapGetSaveInterval(lua_State *L);
    int LuaMapSetName(lua_State *L);
    int LuaMapSetDirectory(lua_State *L);
    int LuaMapSetRankBuild(lua_State *L);
    int LuaMapSetRankJoin(lua_State *L);
    int LuaMapSetRankShow(lua_State *L);
    int LuaMapSetSpawn(lua_State *L);
    int LuaMapSetSaveInterval(lua_State *L);
    int LuaMapAdd(lua_State *L);
    int LuaMapActionAddResize(lua_State *L);
    int LuaMapActionAddFill(lua_State *L);
    int LuaMapActionAddSave(lua_State *L);
    int LuaMapActionAddDelete(lua_State *L);
    int LuaMapResend(lua_State *L);
    int LuaMapExport(lua_State *L);
    int LuaMapExportGetSize(lua_State *L);
    int LuaMapImportPlayer(lua_State *L);
    // -- Block functions
    int LuaBlockGetTable(lua_State *L);
    int LuaBlockGetName(lua_State *L);
    int LuaBlockGetRankPlace(lua_State *L);
    int LuaBlockGetRankDelete(lua_State *L);
    int LuaBlockGetClientType(lua_State *L);
    // -- Rank functions
    int LuaRankGetTable(lua_State *L);
    int LuaRankAdd(lua_State *L);
    int LuaRankDelete(lua_State *L);
    int LuaRankGetName(lua_State *L);
    int LuaRankGetPrefix(lua_State *L);
    int LuaRankGetSuffix(lua_State *L);
    int LuaRankGetRoot(lua_State *L);
    // -- Portal functions
    int LuaTeleporterGetTable(lua_State *L);
    int LuaTeleporterGetBox(lua_State *L);
    int LuaTeleporterGetDestination(lua_State *L);
    int LuaTeleporterAdd(lua_State *L);
    int LuaTeleporterDelete(lua_State *L);
    // -- System Functions
    int LuaMessageToAll(lua_State *L);
    int LuaMessage(lua_State *L);
    // -- Language
    int LuaLanguageGet(lua_State *L);
    // -- Files
    int LuaFileGet(lua_State *L);
    int LuaFolderGet(lua_State *L);
    // -- Event
    int LuaEventAdd(lua_State *L);
    int LuaEventDelete(lua_State *L);
    // -- CPE Functions
    int LuaServerGetExtensions(lua_State *L);
    int LuaServerGetExtension(lua_State *L);
    int LuaClientGetExtensions(lua_State *L);
    int LuaClientGetExtension(lua_State *L);
    int LuaSelectionCuboidAdd(lua_State *L);
    int LuaSelectionCuboidDelete(lua_State *L);
    int LuaGetHeldBlock(lua_State *L);
    int LuaSetHeldBlock(lua_State *L);
    int LuaChangeModel(lua_State *L);
    int LuaSetWeather(lua_State *L);
    int LuaMapSetEnvColors(lua_State *L);
    int LuaClientSetBlockPermissions(lua_State *L);
    int LuaMapEnvSet(lua_State *L);
    int LuaClientHackcontrolSend(lua_State *L);
    int LuaHotkeyAdd(lua_State *L);
    int LuaHotkeyRemove(lua_State *L);
    int LuaMapHackcontrolSet(lua_State *L);

    // -- Event executors
    void LuaDoEventTimer();

    void RegisterEventListener();

    void HandleEvent(const Event &event);
};

#endif
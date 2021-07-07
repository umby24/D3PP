//
// Created by unknown on 4/2/21.
//

#include "Map.h"

#include "Files.h"
#include "Network.h"
#include "System.h"
#include "TaskScheduler.h"
#include "Mem.h"
#include "Logger.h"
#include "compression.h"
#include "common/PreferenceLoader.h"
#include "Utils.h"
#include "Block.h"
#include "watchdog.h"
#include "Network_Functions.h"
#include "Packets.h"
#include "Entity.h"
#include "Player.h"
#include "Player_List.h"
#include "plugins/LuaPlugin.h"
#include "Physics.h"

using namespace std;

const std::string MODULE_NAME = "Map";
MapMain* MapMain::Instance = nullptr;

MapMain::MapMain() {
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);
    TaskScheduler::RegisterTask(MODULE_NAME, *this);
    LastWriteTime = 0;
    SaveFile = false;
    SaveFileTimer = 0;
    mapSettingsLastWriteTime = 0;
    mapSettingsMaxChangesSec = 1100;
    mapSettingsTimerFileCheck = 0;
    StatsTimer = 0;

    mbcStarted = false;
    maStarted = false;
}

void MapMain::MainFunc() {
    Files* f = Files::GetInstance();
    std::string mapListFile = f->GetFile(MAP_LIST_FILE);
    long fileTime = Utils::FileModTime(mapListFile);
    
    if (System::IsRunning && !mbcStarted) {
        std::thread mbcThread([this]() { this->MapBlockChange(); });
        std::swap(BlockchangeThread, mbcThread);
        mbcStarted = true;
    } 

    if (System::IsRunning && !maStarted) {
        std::thread maThread([this]() {this->ActionProcessor(); });
        std::swap(ActionThread, maThread);
        maStarted = true;
    }

    if (System::IsRunning && !phStarted) {
        std::thread physThread([this]() {this->MapBlockPhysics();});
        std::swap(PhysicsThread, physThread);
        phStarted = true;
    }

    if (LastWriteTime != fileTime) {
        MapListLoad();
    }
    if (SaveFile && SaveFileTimer < time(nullptr)) {
        SaveFileTimer = time(nullptr) + 5;
        SaveFile = false;
        MapListSave();
    }
    for(auto const &m : _maps) {

        if (m.second->data.SaveInterval > 0 && m.second->data.SaveTime + m.second->data.SaveInterval*600 < time(nullptr) && m.second->data.loaded) {
            m.second->data.SaveTime = time(nullptr);
            AddSaveAction(0, m.first, "");
        }
        if (m.second->data.Clients > 0) {
            m.second->data.LastClient = time(nullptr);
            if (!m.second->data.loaded) {
                m.second->Reload();
            }
        }
        if (m.second->data.loaded && (time(nullptr) - m.second->data.LastClient) > 200) { // -- 3 minutes
            m.second->Unload();
        }
    }
    fileTime = Utils::FileModTime(f->GetFile(MAP_SETTINGS_FILE));
    if (fileTime != mapSettingsLastWriteTime) {
        MapSettingsLoad();
    }
    if (StatsTimer < time(nullptr)) {
        StatsTimer = time(nullptr) + 5;
        HtmlStats(time(nullptr));
    }
}

MapMain* MapMain::GetInstance() {
    if (Instance == nullptr)
        Instance = new MapMain();

    return Instance;
}

void MapMain::AddSaveAction(int clientId, int mapId, std::string directory) {
    int newActionId = GetMaxActionId();
    bool found = false;
    for(auto const &act : _mapActions) {
        if (act.MapID == mapId && act.Action == MapAction::SAVE && act.Directory == directory) {
            found = true;
            break;
        }
    }

    if (!found) {
        MapActionItem newAction {newActionId, clientId, mapId, MapAction::SAVE, "", directory};
        _mapActions.push_back(newAction);
    }
}

void MapMain::AddLoadAction(int clientId, int mapId, std::string directory) {
    int newActionId = GetMaxActionId();
    bool found = false;
    for(auto const &act : _mapActions) {
        if (act.MapID == mapId && act.Action == MapAction::LOAD && act.Directory == directory) {
            found = true;
            break;
        }
    }

    if (!found) {
        MapActionItem newAction {newActionId, clientId, mapId, MapAction::LOAD, "", directory};
        _mapActions.push_back(newAction);
    }
}

void MapMain::AddResizeAction(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z) {
    int newActionId = GetMaxActionId();
    bool found = false;
    for(auto const &act : _mapActions) {
        if (act.MapID == mapId && act.Action == MapAction::RESIZE) {
            found = true;
            break;
        }
    }

    if (!found) {
        MapActionItem newAction {newActionId, clientId, mapId, MapAction::RESIZE, "", "", X, Y, Z};
        _mapActions.push_back(newAction);
    }
}

void MapMain::AddFillAction(int clientId, int mapId, std::string functionName, std::string argString) {
    int newActionId = GetMaxActionId();
    bool found = false;
    for(auto const &act : _mapActions) {
        if (act.MapID == mapId && act.Action == MapAction::FILL) {
            found = true;
            break;
        }
    }

    if (!found) {
        MapActionItem newAction {newActionId, clientId, mapId, MapAction::FILL, functionName, "", 0, 0, 0, argString};
        _mapActions.push_back(newAction);
    }
}

void MapMain::AddDeleteAction(int clientId, int mapId) {
    int newActionId = GetMaxActionId();
    bool found = false;
    for(auto const &act : _mapActions) {
        if (act.MapID == mapId && act.Action == MapAction::DELETE) {
            found = true;
            break;
        }
    }

    if (!found) {
        MapActionItem newAction {newActionId, clientId, mapId, MapAction::DELETE};
        _mapActions.push_back(newAction);
    }
}

void MapMain::ActionProcessor() {
    while (System::IsRunning) {
        watchdog::Watch("Map_Action", "Begin thread-slope", 0);
        if (_mapActions.size() > 0) {
            MapActionItem item = _mapActions.at(0);
            _mapActions.erase(_mapActions.begin());

            std::shared_ptr<Map> trigMap = GetPointer(item.MapID);

            switch(item.Action) {
                case MapAction::SAVE:
                    watchdog::Watch("Map_Action", "Begin map-save", 1);
                    trigMap->Save(item.Directory);
                    if (item.ClientID > 0) {
                        NetworkFunctions::SystemMessageNetworkSend(item.ClientID, "&eMap Saved.");
                    }
                    watchdog::Watch("Map_Action", "End map-save", 1);
                    // -- TODO: Map_Overview_Save
                    break;
                case MapAction::LOAD:
                    watchdog::Watch("Map_Action", "Begin map-load", 1);
                    trigMap->Load(item.Directory);
                    if (item.ClientID > 0) {
                        NetworkFunctions::SystemMessageNetworkSend(item.ClientID, "&eMap Loaded.");
                    }
                    watchdog::Watch("Map_Action", "end map-load", 1);
                    break;
                case MapAction::FILL:
                    watchdog::Watch("Map_Action", "Begin map-fill", 1);
                    trigMap->Fill(item.FunctionName, item.ArgumentString);
                    if (item.ClientID > 0) {
                        NetworkFunctions::SystemMessageNetworkSend(item.ClientID, "&eMap Filled.");
                    }
                    watchdog::Watch("Map_Action", "end map-load", 1);
                    break;
                case MapAction::RESIZE:
                    watchdog::Watch("Map_Action", "Begin map-resize", 1);
                    trigMap->Resize(item.X, item.Y, item.Z);
                    if (item.ClientID > 0) {
                        NetworkFunctions::SystemMessageNetworkSend(item.ClientID, "&eMap Resized.");
                    }
                    watchdog::Watch("Map_Action", "end map-resize", 1);
                    break;
                case MapAction::DELETE:
                    watchdog::Watch("Map_Action", "Begin map-delete", 1);
                    Delete(item.MapID);
                     if (item.ClientID > 0) {
                        NetworkFunctions::SystemMessageNetworkSend(item.ClientID, "&eMap Deleted.");
                    }
                    watchdog::Watch("Map_Action", "end map-delete", 1);
                break;
            }
        }
        watchdog::Watch("Map_Action", "End thread-slope", 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int MapMain::GetMaxActionId() {
    int result = 0;
    for(auto const &action : _mapActions) {
        if (result <= action.ID)
            result = action.ID + 1;
    }
    return result;
}

void MapMain::MapBlockChange() {
    clock_t blockChangeTimer = clock();

    while (System::IsRunning) {
        watchdog::Watch("Map_Blockchanging", "Begin thread-slope", 0);
        while (blockChangeTimer < clock()) {
            blockChangeTimer += 100;
            for(auto const &m : _maps) {
                if (m.second->data.BlockchangeStopped)
                    continue;
                
                int maxChangedSec = 1100 / 10;
                int toRemove = 0;
                m.second->BlockChangeMutex.lock();
                for(auto const &chg : m.second->data.ChangeQueue) {
                    unsigned short x = chg.X;
                    unsigned short y = chg.Y;
                    unsigned short z = chg.Z;
                    short oldMat = chg.OldMaterial;
                    unsigned char priority = chg.Priority;
                    unsigned char currentMat = m.second->GetBlockType(x, y, z);
                    toRemove++;
                    int blockChangeOffset = GetMapOffset(x, y, z, m.second->data.SizeX, m.second->data.SizeY, m.second->data.SizeZ, 1);
                    int bitmaskSize = GetMapSize(m.second->data.SizeX, m.second->data.SizeY, m.second->data.SizeZ, 1);
                    
                    if (blockChangeOffset < bitmaskSize) { // -- remove from bitmask
                        m.second->data.BlockchangeData[blockChangeOffset/8] = m.second->data.BlockchangeData[blockChangeOffset/8] & ~(1 << (blockChangeOffset % 8));
                    }
                    if (currentMat != oldMat) {
                        NetworkFunctions::NetworkOutBlockSet2Map(m.first, x, y, z, currentMat);
                        maxChangedSec--;
                        if (maxChangedSec <= 0) {
                            break;
                        }
                    }
                }

                m.second->data.ChangeQueue.erase(m.second->data.ChangeQueue.begin(), m.second->data.ChangeQueue.begin() + toRemove);
                m.second->BlockChangeMutex.unlock();
            }
        }
        watchdog::Watch("Map_Blockchanging", "End thread-slope", 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool comparePhysicsTime(MapBlockDo first, MapBlockDo second) {
    return (first.time < second.time);
}

void MapMain::MapBlockPhysics() {
    while (System::IsRunning) {
        watchdog::Watch("Map_Physic", "Begin Thread-Slope", 0);

        for(auto const &map : _maps) {
            if (map.second->data.PhysicsStopped)
                continue;

            std::sort(map.second->data.PhysicsQueue.begin(), map.second->data.PhysicsQueue.end(), comparePhysicsTime);
            watchdog::Watch("Map_Physic", "After: std::sort", 1);
            int counter = 0;
            while (!map.second->data.PhysicsQueue.empty()) {
                MapBlockDo item = map.second->data.PhysicsQueue.at(0);
                if (item.time < clock()) {
                    map.second->data.PhysicsQueue.erase(map.second->data.PhysicsQueue.begin());
                    int offset = MapMain::GetMapOffset(item.X, item.Y, item.Z, map.second->data.SizeX, map.second->data.SizeY, map.second->data.SizeZ, 1);

                    map.second->data.PhysicData[offset/8] = map.second->data.PhysicData[offset/8] & ~(1 << (offset % 8)); // -- Set bitmask
                    map.second->ProcessPhysics(item.X, item.Y, item.Z);
                    counter++;

                } else {
                    break;
                }

                if (counter > 1000)
                    break;
            }
        }

        watchdog::Watch("Map_Physic", "End Thread-Slope", 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
}

shared_ptr<Map> MapMain::GetPointer(int id) {
    if (_maps.find(id) != _maps.end())
        return _maps[id];

    return nullptr;
}

shared_ptr<Map> MapMain::GetPointer(std::string name) {
    shared_ptr<Map> result = nullptr;
    for (auto const &mi : _maps) {
        if (Utils::InsensitiveCompare(mi.second->data.Name, name)) {
            result = mi.second;
            break;
        }
    }
    
    return result;
}

std::shared_ptr<Map> MapMain::GetPointerUniqueId(std::string uniqueId) {
    shared_ptr<Map> result = nullptr;
    for (auto const &mi : _maps) {
        if (Utils::InsensitiveCompare(mi.second->data.UniqueID, uniqueId)) {
            result = mi.second;
            break;
        }
    }
    
    return result;
}

int MapMain::GetMapId() {
    int result = 0;

    while (true) {
        if (GetPointer(result) != nullptr) {
            result++;
            continue;
        }

        return result;
    }
}

std::string MapMain::GetUniqueId() {
    std::string result;

    for(auto i = 1; i < 16; i++)
        result += char(65 + Utils::RandomNumber(25));

    return result;
}

std::string MapMain::GetMapMOTDOverride(int mapId) {
    std::string result = "";
    shared_ptr<Map> mapPtr = GetPointer(mapId);
    if (mapPtr == nullptr)
        return result;
    
    result = mapPtr->data.MotdOverride;
    return result;
}

void MapMain::HtmlStats(time_t time_) {
    time_t startTime = time(nullptr);
    std::string result = MAP_HTML_TEMPLATE;

    // -- Map Table Generation
    std::string mapTable;
    for (auto const &m : _maps) {
        mapTable += "<tr>\n";
        mapTable += "<td>" + stringulate(m.first) + "</td>\n";
        mapTable += "<td>" + m.second->data.UniqueID + "</td>\n";
        mapTable += "<td>" + m.second->data.Name + "</td>\n";
        mapTable += "<td>" + m.second->data.Directory + "</td>\n";
        mapTable += "<td>" + stringulate(m.second->data.SaveInterval) + "min</td>\n";
        mapTable += "<td>" + stringulate(m.second->data.RankBuild) + "," + stringulate(m.second->data.RankJoin) + "," + stringulate(m.second->data.RankShow) + "</td>\n";
        mapTable += "<td>" + stringulate(m.second->data.SizeX) + "x" + stringulate(m.second->data.SizeY) + "x" + stringulate(m.second->data.SizeZ) + "</td>\n";
        if (m.second->data.loaded)
            mapTable += "<td><i>See Memory.html</i></td>\n";
        else
            mapTable += "<td>0 MB (unloaded)</td>\n";

        mapTable += "<td>" + stringulate(m.second->data.PhysicsQueue.size()) + "</td>\n";
        mapTable += "<td>" + stringulate(m.second->data.ChangeQueue.size()) + "</td>\n";
        if (m.second->data.PhysicsStopped)
            mapTable += "<td><font color=\"#FF0000\">Stopped</font></td>\n";
        else
            mapTable += "<td><font color=\"#00FF00\">Started</font></td>\n";
        
        if (m.second->data.BlockchangeStopped)
            mapTable += "<td><font color=\"#FF0000\">Stopped</font></td>\n";
        else
            mapTable += "<td><font color=\"#00FF00\">Started</font></td>\n";

        mapTable += "</tr>\n";
    }
    Utils::replaceAll(result, "[MAP_TABLE]", mapTable);

    time_t finishTime = time(nullptr);
    long duration = finishTime - startTime;
    char buffer[255];
    strftime(buffer, sizeof(buffer), "%H:%M:%S  %m-%d-%Y", localtime(reinterpret_cast<const time_t *>(&finishTime)));
    std::string meh(buffer);
    Utils::replaceAll(result, "[GEN_TIME]", stringulate(duration));
    Utils::replaceAll(result, "[GEN_TIMESTAMP]", meh);

    Files* files = Files::GetInstance();
    std::string memFile = files->GetFile(MAP_HTML_FILE);

    ofstream oStream(memFile, std::ios::out | std::ios::trunc);
    if (oStream.is_open()) {
        oStream << result;
        oStream.close();
    } else {
        Logger::LogAdd(MODULE_NAME, "Couldn't open file :<" + memFile, LogType::WARNING, __FILE__, __LINE__, __FUNCTION__ );
    }
}

void MapMain::MapListSave() {
    Files* f = Files::GetInstance();
    std::string fName = f->GetFile(MAP_LIST_FILE);
    PreferenceLoader pl(fName, "");

    for(auto const &m : _maps) {
        pl.SelectGroup(stringulate(m.first));
        pl.Write("Name", m.second->data.Name);
        pl.Write("Directory", m.second->data.Directory);
        pl.Write("Delete", 0);
        pl.Write("Reload", 0);
    }
    pl.SaveFile();

    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File saved. [" + fName + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void MapMain::MapListLoad() {
    Files* f = Files::GetInstance();
    std::string fName = f->GetFile(MAP_LIST_FILE);
    PreferenceLoader pl(fName, "");
    pl.LoadFile();

    for(auto const &m : pl.SettingsDictionary) {
        if (m.first.empty())
            continue;
        int mapId = stoi(m.first);

        pl.SelectGroup(m.first);
        std::string mapName = pl.Read("Name", m.first);
        std::string directory = pl.Read("Directory", f->GetFolder("Maps") + m.first + "/");
        bool mapDelete = (pl.Read("Delete", 0) == 1);
        bool mapReload = (pl.Read("Reload", 0) == 1);
        if (mapDelete) {
            AddDeleteAction(0, mapId);
        } else {
            shared_ptr<Map> mapPtr = GetPointer(mapId);
            if (mapPtr == nullptr) {
                Add(mapId, 64, 64, 64, mapName);
                mapReload = true;
                mapPtr = GetPointer(mapId);
            }
            mapPtr->data.Directory = directory;
            if ((mapReload)) {
                AddLoadAction(0, mapId, "");
            }
        }
    }
    SaveFile = true;

    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File loaded. [" + fName + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

int MapMain::Add(int id, short x, short y, short z, std::string name) {
    if (id == -1)
        id = GetMapId();

    if (name.empty())
        return -1;

    if (GetPointer(id) != nullptr)
        return -1;

    Files *f = Files::GetInstance();

    shared_ptr<Map> newMap = std::make_shared<Map>();
    int mapSize = GetMapSize(x, y, z, MAP_BLOCK_ELEMENT_SIZE);
    newMap->data.Data = Mem::Allocate(mapSize, __FILE__, __LINE__, "Map_ID = " + stringulate(id));
    newMap->data.BlockchangeData = Mem::Allocate(1+mapSize/8, __FILE__, __LINE__, "Map_ID(Blockchange) = " + stringulate(id));
    newMap->data.PhysicData = Mem::Allocate(1+mapSize/8, __FILE__, __LINE__, "Map_ID(Physics) = " + stringulate(id));
    newMap->data.ID = id;
    newMap->data.UniqueID = GetUniqueId();
    newMap->data.Name = name;
    newMap->data.Directory = f->GetFolder("Maps") + name + "/";
    newMap->data.overviewType = OverviewType::Isomap;
    newMap->data.SaveInterval = 10;
    newMap->data.SaveTime = time(nullptr);
    newMap->data.SizeX = x;
    newMap->data.SizeY = y;
    newMap->data.SizeZ = z;
    newMap->data.SpawnX = x/2;
    newMap->data.SpawnY = y/2;
    newMap->data.SpawnZ = z/1.5;
    newMap->data.loaded = true;
    newMap->data.loading = false;
    newMap->data.BlockchangeStopped = false;
    newMap->data.PhysicsStopped = false;
    newMap->data.Clients = 0;
    newMap->data.LastClient = time(nullptr);
    for(auto i = 1; i < 255; i++)
        newMap->data.blockCounter[i] = 0;

    newMap->data.blockCounter[0] = x*y*z;
    _maps.insert(std::make_pair(id, newMap));
    SaveFile = true;
    return id;
}

void MapMain::Delete(int id) {
    shared_ptr<Map> mp = GetPointer(id);
    if (mp == nullptr)
        return;

    Mem::Free(mp->data.Data);
    Mem::Free(mp->data.BlockchangeData);
    Mem::Free(mp->data.PhysicData);
    _maps.erase(mp->data.ID);
    SaveFile = true;
}

void MapMain::MapSettingsLoad() {
    Files* fm = Files::GetInstance();
    std::string mapSettingsFile = fm->GetFile("Map_Settings");
    json j;
    std::ifstream iStream(mapSettingsFile);
    if (!iStream.is_open()) {
        Logger::LogAdd(MODULE_NAME, "Failed to load Map settings, generating...", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        MapSettingsSave();
        return;
    }

    try {
        iStream >> j;
    } catch (int exception) {
        return;
    }
    iStream.close();

    mapSettingsMaxChangesSec = j["Max_Changes_s"];
    mapSettingsLastWriteTime = Utils::FileModTime(mapSettingsFile);

    Logger::LogAdd(MODULE_NAME, "File Loaded [" + mapSettingsFile + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void MapMain::MapSettingsSave() {
    Files* fm = Files::GetInstance();
    std::string hbSettingsFile = fm->GetFile("Map_Settings");
    json j;
    j["Max_Changes_s"] = mapSettingsMaxChangesSec;

    std::ofstream ofstream(hbSettingsFile);

    ofstream << std::setw(4) << j;
    ofstream.flush();
    ofstream.close();

    mapSettingsLastWriteTime = Utils::FileModTime(hbSettingsFile);
    Logger::LogAdd(MODULE_NAME, "File Saved [" + hbSettingsFile + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

bool Map::Resize(short x, short y, short z) {
    if (!data.loaded) {
        Reload();
    }
    bool result = false;
    if (x >= 16 && y >= 16 && z >= 16 && x <= 32767 && y <= 32767 && z <= 32767) {
        int newMapSize = MapMain::GetMapSize(x, y, z, MAP_BLOCK_ELEMENT_SIZE);
        char* newMapData = Mem::Allocate(newMapSize, __FILE__, __LINE__, "Map_ID = " + stringulate(data.ID));
        char* newBCData = Mem::Allocate(1+newMapSize/8, __FILE__, __LINE__, "Map_ID(Blockchange) = " + stringulate(data.ID));
        char* newPhysData = Mem::Allocate(1+newMapSize/8, __FILE__, __LINE__, "Map_ID(Physics) = " + stringulate(data.ID));

        int copyAreaX = data.SizeX;
        int copyAreaY = data.SizeY;
        int copyAreaZ = data.SizeZ;
        if (copyAreaX > x)
            copyAreaX = x;
        if (copyAreaY > y)
            copyAreaY = y;
        if (copyAreaZ > z)
            copyAreaZ = z;

        for(auto i = 1; i < 255; i++)
            data.blockCounter[i] = 0;

        data.blockCounter[0] = x*y*z - (copyAreaX * copyAreaY * copyAreaZ);
        for (auto ix = 0; ix < copyAreaX; ix++) {
            for(auto iy = 0; iy < copyAreaY; iy++) {
                for (auto iz = 0; iz < copyAreaZ; iz++) {
                    char* pointerOld = (&data.Data[MapMain::GetMapOffset(ix, iy, iz, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE)]);
                    char* pointerNew = &newMapData[MapMain::GetMapOffset(ix, iy, iz, x, y, z, MAP_BLOCK_ELEMENT_SIZE)];
                    data.blockCounter[(int)((unsigned char)(*pointerOld))] ++;

                    memcpy(pointerNew, pointerOld, MAP_BLOCK_ELEMENT_SIZE);
                }
            }
        }
        // -- Copy physics queue
        for(auto const &pi : data.PhysicsQueue) {
            if (pi.X < copyAreaX && pi.Y < copyAreaY && pi.Z < copyAreaZ) {
                int boolOffset = MapMain::GetMapOffset(pi.X, pi.Y, pi.Z, x, y, z, 1);
                char* boolPointer = newPhysData + (boolOffset / 8);
                newPhysData[(boolOffset / 8)] = newPhysData[(boolOffset / 8)] | (1 << (boolOffset % 8));
            }
        }
        data.PhysicsQueue.clear();
        for (auto const bc : data.ChangeQueue) {
            if (bc.X < copyAreaX && bc.Y < copyAreaY && bc.Z < copyAreaZ) {
                int boolOffset = MapMain::GetMapOffset(bc.X, bc.Y, bc.Z, x, y, z, 1);
                char* boolPointer = newPhysData + (boolOffset / 8);
                newBCData[(boolOffset / 8)] = newPhysData[(boolOffset / 8)] | (1 << (boolOffset % 8));
            }
        }
        data.ChangeQueue.clear();
        // -- Spawn limiting..
        if (data.SpawnX > x)
            data.SpawnX = x-1;
        if (data.SpawnY > y)
            data.SpawnY = y - 1;
        result = true;
        data.SizeX = x;
        data.SizeY = y;
        data.SizeZ = z;
        Mem::Free(data.Data);
        Mem::Free(data.PhysicData);
        Mem::Free(data.BlockchangeData);
        data.Data = newMapData;
        data.BlockchangeData = newBCData;
        data.PhysicData = newPhysData;
        Resend();
        MapMain* mm = MapMain::GetInstance();
        mm->SaveFile = true;
    }
    return result;
}

void Map::Fill(std::string functionName, std::string paramString) {
    if (!data.loaded) {
        Reload();
    }

    for(auto i = 1; i < 256; i++) {
        data.blockCounter[i] = 0;
    }
    data.blockCounter[0] = data.SizeX * data.SizeY * data.SizeZ;

    for(auto x = 0; x < data.SizeX; x++) { // -- Clear the map
        for (auto y = 0; y <data.SizeY; y++) {
            for (auto z = 0; z < data.SizeZ; z++) {
                auto* point = (MapBlockData*)(data.Data + MapMain::GetMapOffset(x, y, z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE));
                point->type = 0;
                point->metadata = 0;
                point->lastPlayer = -1;
            }
        }
    }
    data.UniqueID = MapMain::GetUniqueId();
    data.RankBoxes.clear();
    data.Teleporter.clear();

    LuaPlugin* lp = LuaPlugin::GetInstance();
    lp->TriggerMapFill(data.ID, data.SizeX, data.SizeY, data.SizeZ, "Mapfill_" + functionName, paramString);

    Resend();
    Logger::LogAdd(MODULE_NAME, "Map '" + data.Name + "' filled.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

bool Map::Save(std::string directory) {
    if (!this->data.loaded)
        return true;

    int mapDataSize = MapMain::GetMapSize(data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
    if (directory.empty())
        directory = data.Directory;

    std::filesystem::create_directory(directory);
    std::string configName = directory + MAP_FILENAME_CONFIG;
    ofstream oStream(configName);
    if (oStream.is_open()) {
        oStream << "; Overview_Types: 0=Nothing, 1=2D, 2=Iso(fast)" << endl;
        oStream << "; Save_Intervall: in minutes (0 = Disabled)" << endl;
        oStream << "; Jumpheight: -1 = Default" << endl;
        oStream << ";" << endl;
        oStream << "Server_Version = 1018" << endl;
        oStream << "Unique_ID = " << data.UniqueID << endl;
        oStream << "Name = " + data.Name << endl;
        oStream << "Rank_Build = " << data.RankBuild << endl;
        oStream << "Rank_Join = " << data.RankJoin << endl;
        oStream << "Rank_Show = " << data.RankShow << endl;
        oStream << "Physic_Stopped = " << data.PhysicsStopped << endl;
        oStream << "MOTD_Override = " << data.MotdOverride << endl;
        oStream << "Save_Intervall = " << data.SaveInterval << endl;
        oStream << "Overview_Type = " << data.overviewType << endl;
        oStream << "Size_X = " << data.SizeX << endl;
        oStream << "Size_Y = " << data.SizeY << endl;
        oStream << "Size_Z = " << data.SizeZ << endl;
        oStream << "Spawn_X = " << data.SpawnX << endl;
        oStream << "Spawn_Y = " << data.SpawnY << endl;
        oStream << "Spawn_Z = " << data.SpawnZ << endl;
        oStream << "Spawn_Rot = " << data.SpawnRot << endl;
        oStream << "Spawn_Look = " << data.SpawnLook << endl;
        oStream << "Colors_Set = " << data.ColorsSet << endl;
        oStream << "Sky_Color = " << data.SkyColor << endl;
        oStream << "Cloud_Color = " << data.CloudColor << endl;
        oStream << "Fog_Color = " << data.FogColor << endl;
        oStream << "A_Light = " << data.alight << endl;
        oStream << "D_Light = " << data.dlight << endl;
        oStream << "Custom_Appearance = " << data.CustomAppearance << endl;
        oStream << "Custom_Texture_Url = " << data.CustomURL << endl;
        oStream << "Custom_Side_Block = " << data.SideBlock << endl;
        oStream << "Custom_Edge_Block = " << data.EdgeBlock << endl;
        oStream << "Custom_Side_Level = " << data.SideLevel << endl;
        oStream << "Allow_Flying = " << data.Flying << endl;
        oStream << "Allow_Noclip = " << data.NoClip << endl;
        oStream << "Allow_Fastwalk = " << data.Speeding << endl;
        oStream << "Allow_Respawn = " << data.SpawnControl << endl;
        oStream << "Allow_Thirdperson = " << data.ThirdPerson << endl;
        oStream << "Allow_Weatherchange = " << data.Weather << endl;
        oStream << "Jumpheight = " << data.JumpHeight << endl;
        oStream.close();
    }
    configName = directory + MAP_FILENAME_RANK; // -- TODO: Rank boxes
    configName = directory + MAP_FILENAME_TELEPORTER; // -- TODO: Teleporters
    GZIP::GZip_CompressToFile((unsigned char*)data.Data, mapDataSize, "temp.gz");
    try {
        if (std::filesystem::exists(directory + MAP_FILENAME_DATA))
            std::filesystem::remove(directory + MAP_FILENAME_DATA);

        std::filesystem::copy("temp.gz", directory + MAP_FILENAME_DATA, std::filesystem::copy_options::overwrite_existing);
    } catch (std::filesystem::filesystem_error& e) {
        Logger::LogAdd(MODULE_NAME, "Could not copy mapfile: " + stringulate(e.what()), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
    }
    Logger::LogAdd(MODULE_NAME, "File saved [" + directory + MAP_FILENAME_DATA + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);

    return true;
}

void Map::Load(std::string directory) {
    Block* blockMain = Block::GetInstance();

    if (directory.empty()) {
        directory = data.Directory;
    }

    PreferenceLoader pLoader(MAP_FILENAME_CONFIG, directory);
    pLoader.LoadFile();
    int sizeX = pLoader.Read("Size_X", 0);
    int sizeY = pLoader.Read("Size_Y", 0);
    int sizeZ = pLoader.Read("Size_Z", 0);
    int mapSize = sizeX * sizeY * sizeZ;

    Resize(sizeX, sizeY, sizeZ);
    data.UniqueID = pLoader.Read("Unique_ID", MapMain::GetUniqueId());
    data.RankBuild = pLoader.Read("Rank_Build", 0);
    data.RankBuild = pLoader.Read("Rank_Join", 0);
    data.RankBuild = pLoader.Read("Rank_Show", 0);
    data.RankBuild = pLoader.Read("Physic_Stopped", 0);
    data.MotdOverride = pLoader.Read("MOTD_Override", "");
    data.SaveInterval = pLoader.Read("Save_Intervall", 10);
    data.overviewType = static_cast<OverviewType>(pLoader.Read("Overview_Type", 2));
    data.SpawnX = stof(pLoader.Read("Spawn_X", "1"));
    data.SpawnY = stof(pLoader.Read("Spawn_Y", "1"));
    data.SpawnZ = stof(pLoader.Read("Spawn_Z", "0"));
    data.SpawnRot = stof(pLoader.Read("Spawn_Rot", "0"));
    data.SpawnLook = stof(pLoader.Read("Spawn_Look", "0"));

    int dSize = GZIP::GZip_DecompressFromFile(reinterpret_cast<unsigned char*>(data.Data), mapSize * MAP_BLOCK_ELEMENT_SIZE, directory + MAP_FILENAME_DATA);
    if (dSize == (mapSize * MAP_BLOCK_ELEMENT_SIZE)) {
        // -- TODO: Clear undo map
        for(int i = 0; i < 255; i++) {
            data.blockCounter[i] = 0;
        }
        for (int iz = 0; iz < data.SizeZ; iz++) {
            for (int iy = 0; iy < data.SizeY; iy++) {
                for (int ix = 0; ix < data.SizeX; ix++) {
                    int index = MapMain::GetMapOffset(ix, iy, iz, sizeX, sizeY, sizeZ, MAP_BLOCK_ELEMENT_SIZE);
                    unsigned char point = data.Data[index];
                    MapBlock b = blockMain->GetBlock(point);

                    if (b.ReplaceOnLoad >= 0)
                        data.Data[index] = static_cast<char>(b.ReplaceOnLoad);

                    data.blockCounter[point] += 1;
                    
                    if (b.PhysicsOnLoad) {
                        QueueBlockPhysics(ix, iy, iz);
                    }
                }
            }
        }

        Logger::LogAdd(MODULE_NAME, "Map Loaded [" + directory + MAP_FILENAME_DATA + "] (" + stringulate(sizeX) + "x" + stringulate(sizeY) + "x" + stringulate(sizeZ) + ")", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }

    // -- Load RankBox file
    PreferenceLoader rBoxLoader(MAP_FILENAME_RANK, directory);
    rBoxLoader.LoadFile();
    for (auto const &fi : rBoxLoader.SettingsDictionary) {
        rBoxLoader.SelectGroup(fi.first);
        MapRankElement mre;
        mre.X0 = rBoxLoader.Read("X_0", 0);
        mre.Y0 = rBoxLoader.Read("Y_0", 0);
        mre.Z0 = rBoxLoader.Read("Z_0", 0);
        mre.X1 = rBoxLoader.Read("X_1", 0);
        mre.Y1 = rBoxLoader.Read("Y_1", 0);
        mre.Z1 = rBoxLoader.Read("Z_1", 0);
        mre.Rank = rBoxLoader.Read("Rank", 0);
        data.RankBoxes.push_back(mre);
    }
    // -- Load Teleporter file
    PreferenceLoader tpLoader(MAP_FILENAME_TELEPORTER, directory);
    tpLoader.LoadFile();
    for (auto const &tp : tpLoader.SettingsDictionary) {
        tpLoader.SelectGroup(tp.first);
        MapTeleporterElement tpe;
        tpe.Id = tp.first;
        tpe.X0 = tpLoader.Read("X_0", 0);
        tpe.Y0 = tpLoader.Read("Y_0", 0);
        tpe.Z0 = tpLoader.Read("Z_0", 0);
        tpe.X1 = tpLoader.Read("X_1", 0);
        tpe.Y1 = tpLoader.Read("Y_1", 0);
        tpe.Z1 = tpLoader.Read("Z_1", 0);

        tpe.DestMapUniqueId = tpLoader.Read("Dest_Map_Unique_ID", "");
        tpe.DestMapId = tpLoader.Read("Dest_Map_ID", -1);
        tpe.DestX = tpLoader.Read("Dest_X", -1);
        tpe.DestY = tpLoader.Read("Dest_Y", -1);
        tpe.DestZ = tpLoader.Read("Dest_Z", -1);
        tpe.DestRot = tpLoader.Read("Dest_Rot", 0);
        tpe.DestLook = tpLoader.Read("Dest_Look", 0);
        data.Teleporter.insert(std::make_pair(tpe.Id, tpe));
    }
}

void Map::Reload() {
    if (data.loaded)
        return;
    Block* blockMain = Block::GetInstance();

    data.loading = true;
    int mapSize = data.SizeX * data.SizeY * data.SizeZ;
    data.Data = Mem::Allocate(mapSize * MAP_BLOCK_ELEMENT_SIZE, __FILE__, __LINE__, "Map_ID = " + stringulate(data.ID));
    data.BlockchangeData = Mem::Allocate(1+mapSize/8, __FILE__, __LINE__, "Map_ID(Blockchange) = " + stringulate(data.ID));
    data.PhysicData = Mem::Allocate(1+mapSize/8, __FILE__, __LINE__, "Map_ID(Physics) = " + stringulate(data.ID));

    std::string filenameData = data.Directory + MAP_FILENAME_DATA;
    int unzipResult = GZIP::GZip_DecompressFromFile(reinterpret_cast<unsigned char*>(data.Data), mapSize * MAP_BLOCK_ELEMENT_SIZE, data.Directory + MAP_FILENAME_DATA);
    if (unzipResult > 0) {
    // -- UNDO Clear Map
        for(int i = 0; i < 255; i++) {
            data.blockCounter[i] = 0;
        }
        for (int iz = 0; iz < data.SizeZ; iz++) {
            for (int iy = 0; iy < data.SizeY; iy++) {
                for (int ix = 0; ix < data.SizeX; ix++) {
                    int offset = MapMain::GetMapOffset(ix, iy, iz, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
                    unsigned char rawBlock = data.Data[offset];
                    MapBlock b = blockMain->GetBlock(rawBlock);

                    if (b.ReplaceOnLoad >= 0)
                        data.Data[offset] = static_cast<char>(b.ReplaceOnLoad);

                    data.blockCounter[rawBlock] += 1;

                    if (b.PhysicsOnLoad) {
                        QueueBlockPhysics(ix, iy, iz);
                    }
                }
            }
        }

        data.loaded = true;
        data.loading = false;
        data.PhysicsStopped = false;
        data.BlockchangeStopped = false;
        Logger::LogAdd(MODULE_NAME, "Map Reloaded [" + filenameData + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
}

void Map::Unload() {
    if (!data.loaded)
        return;

    Save("");
    data.PhysicsStopped = true;
    data.BlockchangeStopped = true;
    Mem::Free(data.Data);
    Mem::Free(data.BlockchangeData);
    Mem::Free(data.PhysicData);
    data.loaded = false;
    Logger::LogAdd(MODULE_NAME, "Map unloaded (" + data.Name + ")", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Map::Send(int clientId) {
    Network* nMain = Network::GetInstance();
    Block* bMain = Block::GetInstance();
    shared_ptr<NetworkClient> nc = nMain->GetClient(clientId);

    if (nc == nullptr)
        return;

    if (!data.loaded)
        Reload();

    int mapSize = data.SizeX * data.SizeY * data.SizeZ;
    char* tempBuf = Mem::Allocate(mapSize + 10, __FILE__, __LINE__, "Temp");
    int tempBufferOffset = 0;

    tempBuf[tempBufferOffset++] = mapSize / 16777216;
    tempBuf[tempBufferOffset++] = mapSize / 65536;
    tempBuf[tempBufferOffset++] = mapSize / 256;
    tempBuf[tempBufferOffset++] = mapSize;
    std::vector<unsigned char> seenIds;
    std::vector<int> returnedIds;

    for (int i = 0; i < mapSize-1; i++) {
        int index = i * MAP_BLOCK_ELEMENT_SIZE;

        auto rawBlock = static_cast<unsigned char>(data.Data[index]);
        MapBlock mb = bMain->GetBlock(rawBlock);

        if (mb.CpeLevel > nc->CustomBlocksLevel)
            tempBuf[tempBufferOffset++] = mb.CpeReplace;
        else     
            tempBuf[tempBufferOffset++] = mb.OnClient;
    }
    int tempBuffer2Size = GZIP::GZip_CompressBound(tempBufferOffset) + 1024 + 512;
    char* tempBuf2 = Mem::Allocate(tempBuffer2Size, __FILE__, __LINE__, "Temp");

    int compressedSize = GZIP::GZip_Compress(reinterpret_cast<unsigned char*>(tempBuf2), tempBuffer2Size, reinterpret_cast<unsigned char*>(tempBuf), tempBufferOffset);
    Mem::Free(tempBuf);
    if (compressedSize == -1) {
        Logger::LogAdd(MODULE_NAME, "Can't send the map: GZip Error", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        nc->Kick("Mapsend error", false);
        Mem::Free(tempBuf2);
        return;
    }

    compressedSize += (1024 - (compressedSize % 1024));
    Packets::SendMapInit(clientId);
    int bytes2Send = compressedSize;
    int bytesSent = 0;

    while (bytes2Send > 0) {
        int bytesInBlock = bytes2Send;
        if (bytesInBlock > 1024)
            bytesInBlock = 1024;
        Packets::SendMapData(clientId, bytesInBlock, tempBuf2 + bytesSent, bytesSent*100/compressedSize);
        bytesSent += bytesInBlock;
        bytes2Send -= bytesInBlock;
    }

    Packets::SendMapFinalize(clientId, data.SizeX, data.SizeY, data.SizeZ);
    // -- TODO: CPE::AfterMapActions();
    Mem::Free(tempBuf2);
}

void Map::Resend() {
    Network* nMain = Network::GetInstance();
    for(auto const &nc : nMain->_clients) {
        if (nc.second->player->MapId == data.ID)
            nc.second->player->MapId = -1;
    }
    for (auto const &me : Entity::_entities) {
        if (me.second->MapID == data.ID) {
            if (me.second->X > data.SizeX-0.5)
                me.second->X = data.SizeX-0.5;
            
            if (me.second->X < 0.5)
                me.second->X = 0.5;

            if (me.second->Y > data.SizeY-0.5)
                me.second->Y = data.SizeY-0.5;
            
            if (me.second->Y < 0.5)
                me.second->Y = 0.5;
        }
    }

    for(auto const &bc : data.ChangeQueue) {
        int blockCHangeOffset = MapMain::GetMapOffset(bc.X, bc.Y, bc.Z, data.SizeX, data.SizeY, data.SizeZ, 1);
        if (blockCHangeOffset < MapMain::GetMapSize(data.SizeX, data.SizeY, data.SizeZ, 1)) {
            data.BlockchangeData[blockCHangeOffset/8] = (data.BlockchangeData[blockCHangeOffset/8]&255) & ~(1 << (blockCHangeOffset % 8));
        }
    }
    data.ChangeQueue.clear();
}

void Map::BlockChange(std::shared_ptr<NetworkClient> client, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char type) {
    if (client == nullptr || client->player->tEntity == nullptr || client->player->tEntity->playerList == nullptr)
        return;

    if (client->player->tEntity->playerList->Stopped) {
        NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to build (stopped).");
        return;
    }
    
    Block* bm = Block::GetInstance();

    if (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ) {
        unsigned char rawBlock = static_cast<unsigned char>(data.Data[MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE)]);
        unsigned char rawNewType = 0;
        MapBlock oldType = bm->GetBlock(rawBlock);
        client->player->tEntity->lastMaterial = type;
        // -- Map_Block_Get_Rank (RBOX!!)
        // -- TODO:

        if (mode > 0)
            rawNewType = type;
        else
            rawNewType = oldType.AfterDelete;

        MapBlock newType = bm->GetBlock(rawNewType);
        if (client->player->tEntity->playerList->PRank < oldType.RankDelete) {
            NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to delete this block type.");
            return;
        } else if (client->player->tEntity->playerList->PRank < newType.RankPlace) {
            NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to build this block type.");
            return;
        }
        BlockChange(client->player->tEntity->playerList->Number, X, Y, Z, rawNewType, true, true, true, 250);
        QueueBlockChange(X, Y, Z, 250, -1);
        // -- PluginEventBlockCreate (one for delete, one for create.)

    }
}

void Map::BlockChange (short playerNumber, unsigned short X, unsigned short Y, unsigned short Z, unsigned char type, bool undo, bool physic, bool send, unsigned char priority) {
    if (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ) {
        Block* bm = Block::GetInstance();
        int blockOffset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
        auto* atLoc = (MapBlockData*)(data.Data + blockOffset);

        // -- Plugin Event: Block change
        MapBlock oldType = bm->GetBlock(atLoc->type);
        MapBlock newType = bm->GetBlock(type);
        
        if (type != atLoc->type && undo) {
            // -- TODO: Undo_Add()
        }

        data.blockCounter[atLoc->type]--;
        data.blockCounter[type]++;
        atLoc->type = type;
        atLoc->lastPlayer = playerNumber;

        if (physic) {
            for (int ix = -1; ix < 2; ix++) {
                for (int iy = -1; iy < 2; iy++) {
                    for (int iz = -1; iz < 2; iz++) {
                        QueueBlockPhysics(X + ix, Y + iy, Z + iz);
                    }
                }
            }
        }
        if (oldType.Id != newType.Id && send) {
            QueueBlockChange(X, Y, Z, priority, oldType.Id);
        }
    }
}

unsigned char Map::GetBlockType(unsigned short X, unsigned short Y, unsigned short Z) {
    // if (data.loaded = false && data.loading == false) {
    //     data.LastClient = time(nullptr);
    //     Reload();
    // }
    // if (data.loading) {
    //     while (data.loaded == false) {
    //         std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //     }
    // }
    if (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ) {
        int oneOffset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
        int twoOffset = ((Y*data.SizeZ+Z)*data.SizeX+X)*MAP_BLOCK_ELEMENT_SIZE;
        int threeOffset = ((Z*data.SizeY+Y)*data.SizeX+X)*MAP_BLOCK_ELEMENT_SIZE;
        if (oneOffset != threeOffset)
            std::cout<< (int)(data.Data[oneOffset]) << "vs " << (int)(data.Data[threeOffset]) << std::endl;

        auto* stuff = (MapBlockData*)(data.Data + MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE));
        
        return stuff->type;
    }

    return -1;
}

void Map::QueueBlockChange(unsigned short X, unsigned short Y, unsigned short Z, unsigned char priority, unsigned char oldType) {
    if (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ) {
        int offset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, 1);
        bool blockChangeFound = data.BlockchangeData[offset/8] & (1 << (offset % 8));
        if (!blockChangeFound) {
            BlockChangeMutex.lock();
            data.BlockchangeData[offset/8] |= (1 << (offset % 8)); // -- Set bitmask
            MapBlockChanged changeItem { X, Y, Z, priority, oldType};
            int insertIndex = 0;
            
            for(int i = 0; i < data.ChangeQueue.size(); i++) {
                if (data.ChangeQueue[i].Priority >= priority) {
                    insertIndex++;
                    break;
                }
                insertIndex++;
            }
            data.ChangeQueue.insert(data.ChangeQueue.begin() + insertIndex, changeItem);
            BlockChangeMutex.unlock();
        }
    }
}

Map::Map() {
    data.SaveInterval = 0;
    data.UniqueID = "";
    data.Name = "";
    data.Directory = "";
    data.RankBuild = 0;
    data.RankJoin = 0;
    data.RankShow = 0;
    data.SizeX = 0;
    data.SizeY = 0;
    data.SizeZ = 0;
    data.loaded = false;
    data.loading = false;
    data.overviewType = OverviewType::Isomap;
    data.PhysicsStopped = false;
    data.BlockchangeStopped = false;
}

void Map::BlockMove(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1,
                    unsigned short Z1, bool undo, bool physic, unsigned char priority) {
    bool isFirstInBounds = (X0 >= 0 && X0 < data.SizeX && Y0 >= 0 && Y0 < data.SizeY && Z0 >= 0 && Z0 < data.SizeZ);
    bool isSecondInBounds = (X1 >= 0 && X1 < data.SizeX && Y1 >= 0 && Y1 < data.SizeY && Z1 >= 0 && Z1 < data.SizeZ);

    if (isFirstInBounds && isSecondInBounds) {
        MapBlockData* blockdata0 = (MapBlockData*)(data.Data + MapMain::GetMapOffset(X0, Y0, Z0, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE));
        MapBlockData* blockdata1 = (MapBlockData*)(data.Data + MapMain::GetMapOffset(X1, Y1, Z1, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE));

        int oldType0 = blockdata0->type;
        int oldType1 = blockdata1->type;

        if (undo) {

        }

        blockdata1->type = blockdata0->type;
        blockdata1->lastPlayer = blockdata0->lastPlayer;
        blockdata0->type = 0;
        blockdata0->lastPlayer = -1;

        if (oldType0 != 0) {
            QueueBlockChange(X0, Y0, Z0, priority, oldType0);
        }
        if (oldType1 != oldType0) {
            QueueBlockChange(X1, Y1, Z1, priority, oldType1);
        }

        if (physic) {
            for (int ix = -1; ix < 2; ix++) {
                for (int iy = -1; iy < 2; iy++) {
                    for (int iz = -1; iz < 2; iz++) {
                        QueueBlockPhysics(X0 + ix, Y0 + iy, Z0 + iz);
                        QueueBlockPhysics(X1 + ix, Y1 + iy, Z1 + iz);
                    }
                }
            }
        }
    }
}

unsigned short Map::GetBlockPlayer(unsigned short X, unsigned short Y, unsigned short Z) {
    bool blockInBounds = (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ);

    if (blockInBounds) {
        auto* blockdata0 = (MapBlockData*)(data.Data + MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE));
        return blockdata0->lastPlayer;
    }

    return -1;
}

void Map::QueueBlockPhysics(unsigned short X, unsigned short Y, unsigned short Z) {
    bool isBlockInBounds = (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ);

    if (!isBlockInBounds)
        return;

    int offset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, 1);
    bool physItemFound = (data.PhysicData[offset/8] & (1 << (offset % 8)) != 0);

    if (!physItemFound) {
        auto* mapBlockData= (MapBlockData*) (data.Data + MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE));

        Block* bm = Block::GetInstance();
        MapBlock blockEntry = bm->GetBlock(mapBlockData->type);
        unsigned char blockPhysics = blockEntry.Physics;
        std::string physPlugin = blockEntry.PhysicsPlugin;

        if (blockPhysics > 0 || !physPlugin.empty()) {
            data.PhysicData[offset/8] |= (1 << (offset % 8)); // -- Set bitmask
            int physTime = clock() + blockEntry.PhysicsTime + Utils::RandomNumber(blockEntry.PhysicsRandom);
            MapBlockDo physicItem { physTime, X, Y, Z};
            data.PhysicsQueue.push_back(physicItem);
        }
    }
}

void Map::ProcessPhysics(unsigned short X, unsigned short Y, unsigned short Z) {
    Block* bm = Block::GetInstance();
    MapMain* mapMain= MapMain::GetInstance();

    int offset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);

    if (offset < MapMain::GetMapSize(data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE)) {
        auto* mapBlockData = (MapBlockData*) (data.Data + offset);
        MapBlock blockEntry = bm->GetBlock(mapBlockData->type);

        switch (blockEntry.Physics) {
            case 10:
                Physics::BlockPhysics10(mapMain->GetPointer(data.ID), X, Y, Z);
                break;
            case 11:
                Physics::BlockPhysics11(mapMain->GetPointer(data.ID), X, Y, Z);
                break;
            case 20:
                Physics::BlockPhysics20(mapMain->GetPointer(data.ID), X, Y, Z);
                break;
            case 21:
                Physics::BlockPhysics21(mapMain->GetPointer(data.ID), X, Y, Z);
                break;
        }

        if (!blockEntry.PhysicsPlugin.empty()) {
            LuaPlugin* luaPlugin = LuaPlugin::GetInstance();
            std::string pluginName = blockEntry.PhysicsPlugin;
            Utils::replaceAll(pluginName, "Lua:", "");
            luaPlugin->TriggerPhysics(data.ID, X, Y, Z, pluginName);
        }

        if (blockEntry.PhysicsRepeat) {
            QueueBlockPhysics(X, Y, Z);
        }
    }
}

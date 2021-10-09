//
// Created by unknown on 4/2/21.
//

#include "world/Map.h"

#include <utility>

#include "common/Files.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "System.h"
#include "common/TaskScheduler.h"
#include "common/ByteBuffer.h"
#include "common/Logger.h"
#include "compression.h"
#include "common/PreferenceLoader.h"
#include "Utils.h"
#include "Block.h"
#include "watchdog.h"
#include "network/Network_Functions.h"
#include "network/Packets.h"
#include "world/Entity.h"
#include "world/Player.h"
#include "common/Player_List.h"
#include "plugins/LuaPlugin.h"
#include "world/Physics.h"
#include "Undo.h"
#include "CPE.h"
#include "EventSystem.h"
#include "events/EventMapActionDelete.h"
#include "events/EventMapActionFill.h"
#include "events/EventMapActionSave.h"
#include "events/EventMapActionResize.h"
#include "events/EventMapActionLoad.h"
#include "events/EventMapBlockChange.h"
#include "events/EventMapBlockChangeClient.h"
#include "events/EventMapBlockChangePlayer.h"
#include "events/EventMapAdd.h"

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

    phStarted = false;
    mbcStarted = false;
    maStarted = false;
    TempId = 0;
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
        if (m.second->data.SaveInterval > 0 && m.second->data.SaveTime + m.second->data.SaveInterval*60 < time(nullptr) && m.second->data.loaded) {
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
void MapMain::AddAction(int clientId, int mapId, const MapActionItem &action) {
    bool found = false;

    for(auto const &act : _mapActions) {
        if (act.MapID == mapId && act.Action == action.Action) {

            if (action.Action == MapAction::SAVE || action.Action == MapAction::LOAD) {
                if (action.Directory == act.Directory)
                    found = true;
                else
                    continue;
            }

            found = true;
            break;
        }
    }

    if (!found) {
        _mapActions.push_back(action);
    }
}

void MapMain::AddSaveAction(int clientId, int mapId, const std::string& directory) {
    int newActionId = GetMaxActionId();
    MapActionItem newAction {newActionId, clientId, mapId, MapAction::SAVE, "", directory};
    AddAction(clientId, mapId, newAction);
}

void MapMain::AddLoadAction(int clientId, int mapId, const std::string& directory) {
    int newActionId = GetMaxActionId();
    MapActionItem newAction {newActionId, clientId, mapId, MapAction::LOAD, "", directory};
    _mapActions.push_back(newAction);
    
}

void MapMain::AddResizeAction(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z) {
    int newActionId = GetMaxActionId();
    MapActionItem newAction {newActionId, clientId, mapId, MapAction::RESIZE, "", "", X, Y, Z};
    _mapActions.push_back(newAction);
    
}

void MapMain::AddFillAction(int clientId, int mapId, std::string functionName, std::string argString) {
    int newActionId = GetMaxActionId();
    MapActionItem newAction {newActionId, clientId, mapId, MapAction::FILL, std::move(functionName), "", 0, 0, 0, std::move(argString)};
    _mapActions.push_back(newAction);
    
}

void MapMain::AddDeleteAction(int clientId, int mapId) {
    int newActionId = GetMaxActionId();
    MapActionItem newAction {newActionId, clientId, mapId, MapAction::DELETE};
    _mapActions.push_back(newAction);
    
}

void MapMain::ActionProcessor() {
    while (System::IsRunning) {
        watchdog::Watch("Map_Action", "Begin thread-slope", 0);

        if (!_mapActions.empty()) {
            MapActionItem item = _mapActions.at(0);
            _mapActions.erase(_mapActions.begin());

            std::shared_ptr<Map> trigMap = GetPointer(item.MapID);

            switch(item.Action) {
                case MapAction::SAVE:
                {
                    watchdog::Watch("Map_Action", "Begin map-save", 1);
                    trigMap->Save(item.Directory);
                    if (item.ClientID > 0) {
                        NetworkFunctions::SystemMessageNetworkSend(item.ClientID, "&eMap Saved.");
                    }
                    EventMapActionSave mas{};
                    mas.actionId = item.ID;
                    mas.mapId = item.MapID;
                    Dispatcher::post(mas);

                    watchdog::Watch("Map_Action", "End map-save", 1);
                }
                    break;
                case MapAction::LOAD:
                {
                    watchdog::Watch("Map_Action", "Begin map-load", 1);
                    trigMap->Load(item.Directory);
                    if (item.ClientID > 0) {
                        NetworkFunctions::SystemMessageNetworkSend(item.ClientID, "&eMap Loaded.");
                    }
                    EventMapActionLoad mal{};
                    mal.actionId = item.ID;
                    mal.mapId = item.MapID;
                    Dispatcher::post(mal);

                    watchdog::Watch("Map_Action", "end map-load", 1);
                }
                    break;
                case MapAction::FILL:
                    {
                    watchdog::Watch("Map_Action", "Begin map-fill", 1);
                    trigMap->Fill(item.FunctionName, item.ArgumentString);
                    if (item.ClientID > 0) {
                        NetworkFunctions::SystemMessageNetworkSend(item.ClientID, "&eMap Filled.");
                    }
                    EventMapActionFill maf{};
                    maf.actionId = item.ID;
                    maf.mapId = item.MapID;
                    Dispatcher::post(maf);
                    watchdog::Watch("Map_Action", "end map-load", 1);
                    }
                    break;
                case MapAction::RESIZE:
                {
                    watchdog::Watch("Map_Action", "Begin map-resize", 1);
                    trigMap->Resize(item.X, item.Y, item.Z);
                    if (item.ClientID > 0) {
                        NetworkFunctions::SystemMessageNetworkSend(item.ClientID, "&eMap Resized.");
                    }
                    EventMapActionResize mar{};
                    mar.actionId = item.ID;
                    mar.mapId = item.MapID;
                    Dispatcher::post(mar);
                    watchdog::Watch("Map_Action", "end map-resize", 1);
                }
                    break;
                case MapAction::DELETE:
                {
                    watchdog::Watch("Map_Action", "Begin map-delete", 1);

                    EventMapActionDelete delAct{};
                    delAct.actionId = item.ID;
                    delAct.mapId = item.MapID;
                    Dispatcher::post(delAct);

                    Delete(item.MapID);
                     if (item.ClientID > 0) {
                        NetworkFunctions::SystemMessageNetworkSend(item.ClientID, "&eMap Deleted.");
                    }
                    watchdog::Watch("Map_Action", "end map-delete", 1);
                }
                break;
            }
        }
        watchdog::Watch("Map_Action", "End thread-slope", 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
    while (System::IsRunning) {
            watchdog::Watch("Map_Blockchanging", "Begin thread-slope", 0);

            for(auto const &m : _maps) {
                if (m.second->data.BlockchangeStopped) {
                    continue;
                }
                int maxChangedSec = 1100 / 10;
                while (maxChangedSec > 0 && !m.second->data.ChangeQueue.empty()) {
                    MapBlockChanged chg{};
                    {
                        const std::scoped_lock<std::mutex> sLock(m.second->data.bcMutex);
                        if (m.second->data.ChangeQueue.empty())
                            continue;

                        chg = m.second->data.ChangeQueue.top();

                        unsigned short x = chg.X;
                        unsigned short y = chg.Y;
                        unsigned short z = chg.Z;
                        short oldMat = chg.OldMaterial;
                        unsigned char priority = chg.Priority;
                        unsigned char currentMat = m.second->GetBlockType(x, y, z);

                        //if (currentMat != oldMat) {
                            NetworkFunctions::NetworkOutBlockSet2Map(m.first, x, y, z, currentMat);
                      //  }
//                        int offset = MapMain::GetMapOffset(x, y, z, m.second->data.SizeX, m.second->data.SizeY, m.second->data.SizeZ, 1);
//                        if (std::ceil(offset / 8) < m.second->data.BlockchangeData.size()) {
//                            m.second->data.BlockchangeData.at(std::ceil(offset / 8)) = m.second->data.BlockchangeData.at(std::ceil(offset / 8)) & ~(1 << (offset % 8)); // -- Set bitmask
//                        }
                        m.second->data.ChangeQueue.pop();
                    }
                }
            }
            watchdog::Watch("Map_Blockchanging", "End thread-slope", 2);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
                if (item.time < std::chrono::steady_clock::now()) {
                    map.second->data.PhysicsQueue.erase(map.second->data.PhysicsQueue.begin());
                    bool isBlockInBounds = map.second->BlockInBounds(item.X, item.Y, item.Z);

                    if (!isBlockInBounds)
                        continue;
                    // -- range can go out of bounds here.
                    int offset = MapMain::GetMapOffset(item.X, item.Y, item.Z, map.second->data.SizeX, map.second->data.SizeY, map.second->data.SizeZ, 1);
                    if ((std::ceil(offset / 8)) >= map.second->data.PhysicData.size()) {
                        counter++;
                        continue;
                    }
                    map.second->data.PhysicData.at(std::ceil(offset / 8)) = map.second->data.PhysicData.at(std::ceil(offset / 8)) & ~(1 << (offset % 8)); // -- Set bitmask
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::shared_ptr<Map> MapMain::GetPointer(int id) {
    if (_maps.find(id) != _maps.end())
        return _maps[id];

    return nullptr;
}

std::shared_ptr<Map> MapMain::GetPointer(std::string name) {
    std::shared_ptr<Map> result = nullptr;
    for (auto const &mi : _maps) {
        if (Utils::InsensitiveCompare(mi.second->data.Name, name)) {
            result = mi.second;
            break;
        }
    }
    
    return result;
}

std::shared_ptr<Map> MapMain::GetPointerUniqueId(std::string uniqueId) {
    std::shared_ptr<Map> result = nullptr;
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
    MapMain* mmInstance = GetInstance();
    std::string result;
    std::shared_ptr<Map> mapPtr = mmInstance->GetPointer(mapId);

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
        {
            const std::scoped_lock<std::mutex> sLock(m.second->data.bcMutex);
            mapTable += "<td>" + stringulate(m.second->data.ChangeQueue.size()) + "</td>\n";
        }
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
    std::string mapTableString = "[MAP_TABLE]";
    Utils::replaceAll(result, mapTableString, mapTable);

    time_t finishTime = time(nullptr);
    time_t duration = finishTime - startTime;
    char buffer[255];
    strftime(buffer, sizeof(buffer), "%H:%M:%S  %m-%d-%Y", localtime(reinterpret_cast<const time_t *>(&finishTime)));
    std::string meh(buffer);
    Utils::replaceAll(result, "[GEN_TIME]", stringulate(duration));
    Utils::replaceAll(result, "[GEN_TIMESTAMP]", meh);

    Files* files = Files::GetInstance();
    std::string memFile = files->GetFile(MAP_HTML_FILE);

    std::ofstream oStream(memFile, std::ios::out | std::ios::trunc);
    if (oStream.is_open()) {
        oStream << result;
        oStream.close();
    } else {
        Logger::LogAdd(MODULE_NAME, "Couldn't open file :<" + memFile, LogType::WARNING, GLF );
    }
}

void MapMain::MapListSave() {
    Files* f = Files::GetInstance();
    std::string fName = f->GetFile(MAP_LIST_FILE);
    PreferenceLoader pl(fName, "");

    for(auto const &m : _maps) {
        pl.SelectGroup(stringulate(m.first));
        if (m.second->data.Name == "0") {
            throw std::exception();
        }
        pl.Write("Name", m.second->data.Name);
        pl.Write("Directory", m.second->data.Directory);
        pl.Write("Delete", 0);
        pl.Write("Reload", 0);
    }
    pl.SaveFile();

    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File saved. [" + fName + "]", LogType::NORMAL, GLF);
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
            std::shared_ptr<Map> mapPtr = GetPointer(mapId);
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
    Logger::LogAdd(MODULE_NAME, "File loaded. [" + fName + "]", LogType::NORMAL, GLF);
}

int MapMain::Add(int id, short x, short y, short z, const std::string& name) {
    if (id == -1)
        id = GetMapId();

    if (name.empty())
        return -1;

    if (GetPointer(id) != nullptr)
        return -1;

    Files *f = Files::GetInstance();

    std::shared_ptr<Map> newMap = std::make_shared<Map>();
    int mapSize = GetMapSize(x, y, z, MAP_BLOCK_ELEMENT_SIZE);
    newMap->data.Data.resize(mapSize);
    newMap->data.BlockchangeData.resize(1+mapSize/8);
    newMap->data.PhysicData.resize(1+mapSize/8);
    newMap->data.blockCounter.resize(256);
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
        newMap->data.blockCounter.at(i) = 0;

    newMap->data.blockCounter.at(0) = x*y*z;
    _maps.insert(std::make_pair(id, newMap));
    SaveFile = true;

    EventMapAdd ema;
    ema.mapId = id;
    Dispatcher::post(ema);

    return id;
}

void MapMain::Delete(int id) {
    std::shared_ptr<Map> mp = GetPointer(id);
    if (mp == nullptr)
        return;
    Network* nm = Network::GetInstance();
    if (mp->data.Clients > 0) {
        for(auto const &nc : nm->roClients) {
            if (nc->LoggedIn && nc->player->tEntity != nullptr&& nc->player->tEntity->MapID == id) {
                MinecraftLocation somewhere {0, 0, 0, 0, 0};
                nc->player->tEntity->PositionSet(0, somewhere, 4, true);
            }
        }
    }
    mp->data.Data.clear();
    mp->data.BlockchangeData.clear();
    mp->data.PhysicData.clear();
    _maps.erase(mp->data.ID);
    SaveFile = true;
}

void MapMain::MapSettingsLoad() {
    Files* fm = Files::GetInstance();
    std::string mapSettingsFile = fm->GetFile("Map_Settings");
    json j;
    std::ifstream iStream(mapSettingsFile);
    if (!iStream.is_open()) {
        Logger::LogAdd(MODULE_NAME, "Failed to load Map settings, generating...", LogType::WARNING, GLF);
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

    Logger::LogAdd(MODULE_NAME, "File Loaded [" + mapSettingsFile + "]", LogType::NORMAL, GLF);
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
    Logger::LogAdd(MODULE_NAME, "File Saved [" + hbSettingsFile + "]", LogType::NORMAL, GLF);
}

bool Map::Resize(short x, short y, short z) {
    if (!data.loaded) {
        Reload();
    }
    bool result = false;

    if (x >= 16 && y >= 16 && z >= 16 && x <= 32767 && y <= 32767 && z <= 32767) {
        int newMapSize = MapMain::GetMapSize(x, y, z, MAP_BLOCK_ELEMENT_SIZE);
       
        for(auto i = 1; i < 255; i++)
           data.blockCounter.at(i) = 0;
        {
            const std::scoped_lock<std::mutex> sLock(data.bcMutex);
            while (!data.ChangeQueue.empty())
                data.ChangeQueue.pop();
        }
        // -- Spawn limiting..
        if (data.SpawnX > x)
            data.SpawnX = x-1;

        if (data.SpawnY > y)
            data.SpawnY = y - 1;

        result = true;
        data.SizeX = x;
        data.SizeY = y;
        data.SizeZ = z;

        data.Data.resize(newMapSize);
        data.BlockchangeData.resize(1+newMapSize/8);
        data.PhysicData.resize(1+newMapSize/8);

        Resend();
        MapMain* mm = MapMain::GetInstance();
        mm->SaveFile = true;
    }

    return result;
}

void Map::Fill(const std::string& functionName, std::string paramString) {
    if (!data.loaded) {
        Reload();
    }

    for(auto i = 1; i < 256; i++) {
        data.blockCounter.at(i) = 0;
    }
    data.blockCounter.at(0) = data.SizeX * data.SizeY * data.SizeZ;

    for(auto x = 0; x < data.SizeX; x++) { // -- Clear the map
        for (auto y = 0; y <data.SizeY; y++) {
            for (auto z = 0; z < data.SizeZ; z++) {
                int index = MapMain::GetMapOffset(x, y, z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
                data.Data[index] = 0;
                data.Data[index+1] = 0;
                data.Data[index+2] = 128;
                data.Data[index+3] = 0;
            }
        }
    }
    data.UniqueID = MapMain::GetUniqueId();
    data.RankBoxes.clear();
    data.Teleporter.clear();

    LuaPlugin* lp = LuaPlugin::GetInstance();
    lp->TriggerMapFill(data.ID, data.SizeX, data.SizeY, data.SizeZ, "Mapfill_" + functionName, std::move(paramString));

    Resend();
    Logger::LogAdd(MODULE_NAME, "Map '" + data.Name + "' filled.", LogType::NORMAL, GLF);
}

bool Map::Save(std::string directory) {
    if (!this->data.loaded)
        return true;

    int mapDataSize = MapMain::GetMapSize(data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
    
    if (directory.empty())
        directory = data.Directory;

    std::filesystem::create_directory(directory);
    SaveConfigFile(directory);
    GZIP::GZip_CompressToFile(data.Data.data(), mapDataSize, "temp.gz");

    try {
        if (std::filesystem::exists(directory + MAP_FILENAME_DATA))
            std::filesystem::remove(directory + MAP_FILENAME_DATA);

        std::filesystem::copy("temp.gz", directory + MAP_FILENAME_DATA, std::filesystem::copy_options::overwrite_existing);
    } catch (std::filesystem::filesystem_error& e) {
        Logger::LogAdd(MODULE_NAME, "Could not copy mapfile: " + stringulate(e.what()), LogType::L_ERROR, GLF);
    }

    Logger::LogAdd(MODULE_NAME, "File saved [" + directory + MAP_FILENAME_DATA + "]", LogType::NORMAL, GLF);

    return true;
}

void Map::SaveConfigFile(const std::string& directory) {
    std::string configName = directory + MAP_FILENAME_CONFIG;
    std::ofstream oStream(configName);
    if (oStream.is_open()) {
        oStream << "; Overview_Types: 0=Nothing, 1=2D, 2=Iso(fast)" << "\n";
        oStream << "; Save_Interval: in minutes (0 = Disabled)" << "\n";
        oStream << "; Jumpheight: -1 = Default" << "\n";
        oStream << ";" << "\n";
        oStream << "Server_Version = 1018" << "\n";
        oStream << "Unique_ID = " << data.UniqueID << "\n";
        oStream << "Name = " + data.Name << "\n";
        oStream << "Rank_Build = " << data.RankBuild << "\n";
        oStream << "Rank_Join = " << data.RankJoin << "\n";
        oStream << "Rank_Show = " << data.RankShow << "\n";
        oStream << "Physic_Stopped = " << data.PhysicsStopped << "\n";
        oStream << "MOTD_Override = " << data.MotdOverride << "\n";
        oStream << "Save_Intervall = " << data.SaveInterval << "\n";
        oStream << "Overview_Type = " << data.overviewType << "\n";
        oStream << "Size_X = " << data.SizeX << "\n";
        oStream << "Size_Y = " << data.SizeY << "\n";
        oStream << "Size_Z = " << data.SizeZ << "\n";
        oStream << "Spawn_X = " << data.SpawnX << "\n";
        oStream << "Spawn_Y = " << data.SpawnY << "\n";
        oStream << "Spawn_Z = " << data.SpawnZ << "\n";
        oStream << "Spawn_Rot = " << data.SpawnRot << "\n";
        oStream << "Spawn_Look = " << data.SpawnLook << "\n";
        oStream << "Colors_Set = " << data.ColorsSet << "\n";
        oStream << "Sky_Color = " << data.SkyColor << "\n";
        oStream << "Cloud_Color = " << data.CloudColor << "\n";
        oStream << "Fog_Color = " << data.FogColor << "\n";
        oStream << "A_Light = " << data.alight << "\n";
        oStream << "D_Light = " << data.dlight << "\n";
        oStream << "Custom_Appearance = " << data.CustomAppearance << "\n";
        oStream << "Custom_Texture_Url = " << data.CustomURL << "\n";
        oStream << "Custom_Side_Block = " << (int)data.SideBlock << "\n";
        oStream << "Custom_Edge_Block = " << (int)data.EdgeBlock << "\n";
        oStream << "Custom_Side_Level = " << data.SideLevel << "\n";
        oStream << "Allow_Flying = " << data.Flying << "\n";
        oStream << "Allow_Noclip = " << data.NoClip << "\n";
        oStream << "Allow_Fastwalk = " << data.Speeding << "\n";
        oStream << "Allow_Respawn = " << data.SpawnControl << "\n";
        oStream << "Allow_Thirdperson = " << data.ThirdPerson << "\n";
        oStream << "Allow_Weatherchange = " << data.Weather << "\n";
        oStream << "Jumpheight = " << data.JumpHeight << "\n";
        oStream.close();
    }
}

void Map::SaveRankBoxFile(const std::string& directory) {
    PreferenceLoader pLoader(MAP_FILENAME_RANK, directory, true);
    int rBoxNumber = 0;
    for(auto const &rb : data.RankBoxes) {
        pLoader.SelectGroup(stringulate(rBoxNumber));
        pLoader.Write("X_0", rb.X0);
        pLoader.Write("Y_0", rb.Y0);
        pLoader.Write("Z_0", rb.Z0);
        pLoader.Write("X_1", rb.X1);
        pLoader.Write("Y_1", rb.Y1);
        pLoader.Write("Z_1", rb.Z1);
        pLoader.Write("Rank", rb.Rank);
    }
    pLoader.SaveFile();
    Logger::LogAdd(MODULE_NAME, "File saved [" + directory + MAP_FILENAME_RANK + "]", LogType::NORMAL, GLF);
}

void Map::SaveTeleporterFile(const std::string& directory) {
    PreferenceLoader pLoader(MAP_FILENAME_TELEPORTER, directory, true);
    for (auto const &tp : data.Teleporter) {
        pLoader.SelectGroup(tp.first);
        pLoader.Write("X_0", tp.second.X0);
        pLoader.Write("Y_0", tp.second.Y0);
        pLoader.Write("Z_0", tp.second.Z0);
        pLoader.Write("X_1", tp.second.X1);
        pLoader.Write("Y_1", tp.second.Y1);
        pLoader.Write("Z_1", tp.second.Z1);
        pLoader.Write("Dest_Map_Unique_ID", tp.second.DestMapUniqueId);
        pLoader.Write("Dest_Map_ID", tp.second.DestMapId);
        pLoader.Write("Dest_X", tp.second.DestX);
        pLoader.Write("Dest_Y", tp.second.DestY);
        pLoader.Write("Dest_Z", tp.second.DestZ);
        pLoader.Write("Dest_Rot", tp.second.DestRot);
        pLoader.Write("Dest_Look", tp.second.DestLook);
    }
    pLoader.SaveFile();
    Logger::LogAdd(MODULE_NAME, "File saved [" + directory + MAP_FILENAME_TELEPORTER + "]", LogType::NORMAL, GLF);
}

void Map::Load(std::string directory) {
    Block* blockMain = Block::GetInstance();

    if (directory.empty()) {
        directory = data.Directory;
    }
    if (!std::filesystem::exists(directory)) {
        return;
    }
    if (directory[directory.size()-1] != '/') {
        directory = directory + "/";
    }
    data.loading = true;
    LoadConfigFile(directory);

    int sizeX = data.SizeX;
    int sizeY = data.SizeY;
    int sizeZ = data.SizeZ;
    int mapSize = sizeX * sizeY * sizeZ;

    int dSize = GZIP::GZip_DecompressFromFile(reinterpret_cast<unsigned char*>(data.Data.data()), mapSize * MAP_BLOCK_ELEMENT_SIZE, directory + MAP_FILENAME_DATA);

    if (dSize == (mapSize * MAP_BLOCK_ELEMENT_SIZE)) {
        Undo::ClearMap(data.ID);

        for (int iz = 0; iz < data.SizeZ; iz++) {
            for (int iy = 0; iy < data.SizeY; iy++) {
                for (int ix = 0; ix < data.SizeX; ix++) {
                    int index = MapMain::GetMapOffset(ix, iy, iz, sizeX, sizeY, sizeZ, MAP_BLOCK_ELEMENT_SIZE);
                    unsigned char point = data.Data.at(index);
                    MapBlock b = blockMain->GetBlock(point);

                    if (b.ReplaceOnLoad >= 0)
                        data.Data.at(index) = static_cast<char>(b.ReplaceOnLoad);

                    data.blockCounter.at(point) += 1;
                    
                    if (b.PhysicsOnLoad) {
                        QueueBlockPhysics(ix, iy, iz);
                    }
                }
            }
        }

        Logger::LogAdd(MODULE_NAME, "Map Loaded [" + directory + MAP_FILENAME_DATA + "] (" + stringulate(sizeX) + "x" + stringulate(sizeY) + "x" + stringulate(sizeZ) + ")", LogType::NORMAL, GLF);
    }

    // -- Load RankBox file
    LoadRankBoxFile(directory);
    // -- Load Teleporter file
    LoadTeleporterFile(directory);
    data.loading = false;
}

void Map::LoadConfigFile(std::string directory) {
    PreferenceLoader pLoader(MAP_FILENAME_CONFIG, directory);
    pLoader.LoadFile();
    int sizeX = pLoader.Read("Size_X", 0);
    int sizeY = pLoader.Read("Size_Y", 0);
    int sizeZ = pLoader.Read("Size_Z", 0);
    int mapSize = sizeX * sizeY * sizeZ;

    Resize(sizeX, sizeY, sizeZ);

    data.UniqueID = pLoader.Read("Unique_ID", MapMain::GetUniqueId());
    data.RankBuild = pLoader.Read("Rank_Build", 0);
    data.RankJoin = pLoader.Read("Rank_Join", 0);
    data.RankShow = pLoader.Read("Rank_Show", 0);
    data.PhysicsStopped = pLoader.Read("Physic_Stopped", 0);
    data.MotdOverride = pLoader.Read("MOTD_Override", "");
    data.SaveInterval = pLoader.Read("Save_Intervall", 10);
    data.overviewType = static_cast<OverviewType>(pLoader.Read("Overview_Type", 2));
    data.SpawnX = stof(pLoader.Read("Spawn_X", "1"));
    data.SpawnY = stof(pLoader.Read("Spawn_Y", "1"));
    data.SpawnZ = stof(pLoader.Read("Spawn_Z", "0"));
    data.SpawnRot = stof(pLoader.Read("Spawn_Rot", "0"));
    data.SpawnLook = stof(pLoader.Read("Spawn_Look", "0"));
    data.ColorsSet = (pLoader.Read("Colors_Set", 0) > 0);
    data.SkyColor = pLoader.Read("Sky_Color", -1);
    data.CloudColor = pLoader.Read("Cloud_Color", -1);
    data.FogColor = pLoader.Read("Fog_Color", -1);
    data.alight = pLoader.Read("A_Light", -1);
    data.dlight = pLoader.Read("D_Light", -1);
    data.CustomAppearance = (pLoader.Read("Custom_Appearance", 0) > 0);
    data.CustomURL = pLoader.Read("Custom_Texture_Url", "");
    data.SideLevel = pLoader.Read("Custom_Side_Level", -1);
    data.EdgeBlock = pLoader.Read("Custom_Edge_Block", -1);
    data.SideBlock = pLoader.Read("Custom_Side_Block", -1);
    data.Flying = (pLoader.Read("Allow_Flying", 1) > 0);
    data.NoClip = (pLoader.Read("Allow_Noclip", 1) > 0);
    data.Speeding = (pLoader.Read("Allow_Fastwalk", 1) > 0);
    data.SpawnControl = (pLoader.Read("Allow_Respawn", 1) > 0);
    data.ThirdPerson = (pLoader.Read("Allow_Thirdperson", 1) > 0);
    data.Weather = (pLoader.Read("Allow_Weatherchange", 1) > 0);
    data.JumpHeight = pLoader.Read("Jumpheight", -1);
}

void Map::LoadRankBoxFile(std::string directory) {
    PreferenceLoader rBoxLoader(MAP_FILENAME_RANK, directory);
    rBoxLoader.LoadFile();
    for (auto const &fi : rBoxLoader.SettingsDictionary) {
        rBoxLoader.SelectGroup(fi.first);
        MapRankElement mre{};
        mre.X0 = rBoxLoader.Read("X_0", 0);
        mre.Y0 = rBoxLoader.Read("Y_0", 0);
        mre.Z0 = rBoxLoader.Read("Z_0", 0);
        mre.X1 = rBoxLoader.Read("X_1", 0);
        mre.Y1 = rBoxLoader.Read("Y_1", 0);
        mre.Z1 = rBoxLoader.Read("Z_1", 0);
        mre.Rank = rBoxLoader.Read("Rank", 0);
        data.RankBoxes.push_back(mre);
    }
}

void Map::LoadTeleporterFile(std::string directory) {
    PreferenceLoader tpLoader(MAP_FILENAME_TELEPORTER, directory);
    tpLoader.LoadFile();
    for (auto const &tp : tpLoader.SettingsDictionary) {
        tpLoader.SelectGroup(tp.first);
        MapTeleporterElement tpe{};
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
    data.Data.resize(mapSize * MAP_BLOCK_ELEMENT_SIZE);
    data.BlockchangeData.resize(1+mapSize/8);
    data.PhysicData.resize(1+mapSize/8);

    std::string filenameData = data.Directory + MAP_FILENAME_DATA;
    int unzipResult = GZIP::GZip_DecompressFromFile(reinterpret_cast<unsigned char*>(data.Data.data()), mapSize * MAP_BLOCK_ELEMENT_SIZE, data.Directory + MAP_FILENAME_DATA);
    if (unzipResult > 0) {
        Undo::ClearMap(data.ID);
        for(int i = 0; i < 255; i++) {
            data.blockCounter.at(i) = 0;
        }
        for (int iz = 0; iz < data.SizeZ; iz++) {
            for (int iy = 0; iy < data.SizeY; iy++) {
                for (int ix = 0; ix < data.SizeX; ix++) {
                    int offset = MapMain::GetMapOffset(ix, iy, iz, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
                    unsigned char rawBlock = data.Data.at(offset);
                    MapBlock b = blockMain->GetBlock(rawBlock);

                    if (b.ReplaceOnLoad >= 0)
                        data.Data.at(offset) = b.ReplaceOnLoad;

                    data.blockCounter.at(rawBlock) += 1;

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
        Logger::LogAdd(MODULE_NAME, "Map Reloaded [" + filenameData + "]", LogType::NORMAL, GLF);
    }
}

void Map::Unload() {
    if (!data.loaded)
        return;

    Save("");
    data.PhysicsStopped = true;
    data.BlockchangeStopped = true;
    data.Data.resize(1);
    data.BlockchangeData.resize(1);
    data.PhysicData.resize(1);
    data.loaded = false;
    Logger::LogAdd(MODULE_NAME, "Map unloaded (" + data.Name + ")", LogType::NORMAL, GLF);
}

void Map::Send(int clientId) {
    Network* nMain = Network::GetInstance();
    Block* bMain = Block::GetInstance();
    std::shared_ptr<NetworkClient> nc = nMain->GetClient(clientId);

    if (nc == nullptr)
        return;

    if (!data.loaded)
        Reload();

    int mapSize = data.SizeX * data.SizeY * data.SizeZ;
    std::vector<unsigned char> tempBuf(mapSize + 10);
    int tempBufferOffset = 0;

    tempBuf.at(tempBufferOffset++) = static_cast<unsigned char>(mapSize >> 24);
    tempBuf.at(tempBufferOffset++) = static_cast<unsigned char>(mapSize >> 16);
    tempBuf.at(tempBufferOffset++) = static_cast<unsigned char>(mapSize >> 8);
    tempBuf.at(tempBufferOffset++) = static_cast<unsigned char>(mapSize);

    int cbl = nc->CustomBlocksLevel;

    for (int i = 0; i < mapSize-1; i++) {
        int index = i * MAP_BLOCK_ELEMENT_SIZE;
        unsigned char blockAt = data.Data[index];
        if (blockAt < 49) { // -- If its an original block, Dont bother checking. Just speed past.
            tempBuf[tempBufferOffset++] = blockAt;
            continue;
        }

        MapBlock mb = bMain->GetBlock(blockAt);

        if (mb.CpeLevel > cbl)
            tempBuf[tempBufferOffset++] = static_cast<char>(mb.CpeReplace);
        else     
            tempBuf[tempBufferOffset++] = static_cast<char>(mb.OnClient);
    }

    int tempBuffer2Size = GZIP::GZip_CompressBound(tempBufferOffset) + 1024 + 512;
    std::vector<unsigned char> tempBuf2(tempBuffer2Size);

    int compressedSize = GZIP::GZip_Compress(tempBuf2.data(), tempBuffer2Size, tempBuf.data(), tempBufferOffset);

    if (compressedSize == -1) {
        Logger::LogAdd(MODULE_NAME, "Can't send the map: GZip Error", LogType::L_ERROR, GLF);
        nc->Kick("Mapsend error", false);
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
        Packets::SendMapData(clientId, static_cast<short>(bytesInBlock),
                             reinterpret_cast<char *>(tempBuf2.data() + bytesSent), static_cast<unsigned char>(bytesSent * 100.0 / compressedSize));
        bytesSent += bytesInBlock;
        bytes2Send -= bytesInBlock;
    }
    tempBuf.clear();
    tempBuf2.clear();
    Packets::SendMapFinalize(clientId, data.SizeX, data.SizeY, data.SizeZ);
    CPE::AfterMapActions(nc);
}

void Map::Resend() {
    Network* nMain = Network::GetInstance();
    for(auto const &nc : nMain->roClients) {
        if (nc->player == nullptr)
            continue;

        if (nc->player->MapId == data.ID) {
            nc->player->SendMap();
        }
    }

    for (auto const &me : Entity::AllEntities) {
        if (me.second->MapID == data.ID) {
            Vector3S newLocation{data.SizeX+16, data.SizeY+16, data.SizeZ+16};
            me.second->Location.SetAsPlayerCoords(newLocation);
        }
    }

    const std::scoped_lock<std::mutex> sLock(data.bcMutex);
    while (!data.ChangeQueue.empty())
        data.ChangeQueue.pop();

}

void Map::BlockChange(const std::shared_ptr<NetworkClient>& client, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char type) {
    if (client == nullptr || client->player->tEntity == nullptr || client->player->tEntity->playerList == nullptr)
        return;

    if (client->player->tEntity->playerList->Stopped) {
        NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to build (stopped).");
        return;
    }
    
    Block* bm = Block::GetInstance();

    if (BlockInBounds(X, Y, Z)) {
        int blockOffset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
        auto &rawBlock = data.Data.at(blockOffset);
        unsigned char rawNewType = 0;

        MapBlock oldType = bm->GetBlock(rawBlock);
        client->player->tEntity->lastMaterial = type;
        int blockRank = BlockGetRank(X, Y, Z);

        if (mode > 0)
            rawNewType = type;
        else
            rawNewType = oldType.AfterDelete;

        MapBlock newType = bm->GetBlock(rawNewType);
        if (rawBlock == rawNewType) {
            return;
        }

        if (client->player->tEntity->playerList->PRank < oldType.RankDelete) {
            NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to delete this block type.");
            NetworkFunctions::NetworkOutBlockSet(client->Id, X, Y, Z, oldType.OnClient);
            return;
        } else if (client->player->tEntity->playerList->PRank < newType.RankPlace) {
            NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to build this block type.");
            NetworkFunctions::NetworkOutBlockSet(client->Id, X, Y, Z, oldType.OnClient);
            return;
        } else if (client->player->tEntity->playerList->PRank < blockRank) {
            NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to build here.");
            NetworkFunctions::NetworkOutBlockSet(client->Id, X, Y, Z, oldType.OnClient);
            return;
        }
        EventMapBlockChangeClient mbc;
        mbc.playerId = client->player->tEntity->playerList->Number;
        mbc.mapId = data.ID;
        mbc.X = X;
        mbc.Y = Y;
        mbc.Z =  Z;
        mbc.bType = type;
        mbc.mode = mode;
        Dispatcher::post(mbc);

        BlockChange(client->player->tEntity->playerList->Number, X, Y, Z, rawNewType, true, true, true, 250);
        QueueBlockChange(X, Y, Z, 250, -1);
        // -- PluginEventBlockCreate (one for delete, one for create.)

    }
}

void Map::BlockChange (short playerNumber, unsigned short X, unsigned short Y, unsigned short Z, unsigned char type, bool undo, bool physic, bool send, unsigned char priority) {
    if (!BlockInBounds(X, Y, Z)) {
        return;
    }
    Vector3S locationVector {static_cast<short>(X), static_cast<short>(Y), static_cast<short>(Z)};
    Block* bm = Block::GetInstance();
    int blockOffset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
    auto roData = GetBlockData(locationVector);

    EventMapBlockChange event;
    event.playerNumber = playerNumber;
    event.mapId = data.ID;
    event.X = X;
    event.Y = Y;
    event.Z = Z;
    event.bType = type;
    event.undo = undo;
    event.send = send;
    Dispatcher::post(event); // -- Post this event out!

    MapBlock oldType = bm->GetBlock(roData.type);
    MapBlock newType = bm->GetBlock(type);

    if (type != roData.type && undo) {
        Undo::Add(playerNumber, data.ID, X, Y, Z, oldType.Id, roData.lastPlayer);
    }
    if (type != roData.type && send) {
        QueueBlockChange(X, Y, Z, priority, oldType.Id);
    }
    data.blockCounter.at(roData.type)--;
    data.blockCounter.at(type)++;
    roData.type = type;
    roData.lastPlayer = playerNumber;
    SetBlockData(locationVector, roData);

    if (physic) {
        for (int ix = -1; ix < 2; ix++) {
            for (int iy = -1; iy < 2; iy++) {
                for (int iz = -1; iz < 2; iz++) {
                    QueueBlockPhysics(X + ix, Y + iy, Z + iz);
                }
            }
        }
    }


}

unsigned char Map::GetBlockType(unsigned short X, unsigned short Y, unsigned short Z) {
     if (!data.loaded && !data.loading) {
         data.LastClient = time(nullptr);
         Reload();
     }
     if (data.loading) {
         while (data.loading) {
             std::this_thread::sleep_for(std::chrono::milliseconds(100));
         }
     }
    if (BlockInBounds(X, Y, Z)) {
        int oneOffset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
        
        return data.Data.at(oneOffset);
    }

    return -1;
}

void Map::QueueBlockChange(unsigned short X, unsigned short Y, unsigned short Z, unsigned char priority, unsigned char oldType) {
    if (!BlockInBounds(X, Y, Z)) {
        return;
    }
    int offset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, 1);
    MapBlockChanged changeItem { X, Y, Z, priority, oldType};
    const std::scoped_lock<std::mutex> sLock(data.bcMutex);
    data.ChangeQueue.push(changeItem);
    data.BlockchangeData.at(std::ceil(offset / 8)) |= (1 << (offset % 8));
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
    data.CustomAppearance = false;
    data.ColorsSet = false;
    data.SkyColor = -1;
    data.CloudColor = -1;
    data.FogColor = -1;
    data.alight = -1;
    data.dlight = -1;
    data.SideBlock = 0;
    data.EdgeBlock = 0;
    data.SideLevel = 64;
}

void Map::BlockMove(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1,
                    unsigned short Z1, bool undo, bool physic, unsigned char priority) {
    bool isFirstInBounds = BlockInBounds(X0, Y0, Z0);
    bool isSecondInBounds = BlockInBounds(X1, Y1, Z1);

    if (isFirstInBounds && isSecondInBounds) {
        Vector3S location0 {static_cast<short>(X0), static_cast<short>(Y0), static_cast<short>(Z0)};
        Vector3S location1 {static_cast<short>(X1), static_cast<short>(Y1), static_cast<short>(Z1)};

        auto oldRo0 = GetBlockData(location0);
        auto oldRo1 = GetBlockData(location1);

        if (undo) {
            if (oldRo0.type != 0)
                Undo::Add(oldRo0.lastPlayer, data.ID, X0, Y0, Z0, oldRo0.type, oldRo0.lastPlayer);

            if (oldRo0.type != oldRo1.type)
                Undo::Add(oldRo0.lastPlayer, data.ID, X1, Y1, Z1, oldRo1.type, oldRo1.lastPlayer);
        }


        if (oldRo0.type != 0) {
            QueueBlockChange(X0, Y0, Z0, priority, oldRo0.type);
        }
        if (oldRo1.type != oldRo0.type) {
            QueueBlockChange(X1, Y1, Z1, priority, oldRo1.type);
        }
        oldRo1.type = oldRo0.type;
        oldRo1.lastPlayer = oldRo0.lastPlayer;
        oldRo0.type = 0;
        oldRo0.lastPlayer = -1;

        SetBlockData(location0, oldRo0);
        SetBlockData(location1, oldRo1);
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
    if (BlockInBounds(X, Y, Z)) {
        int index = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
        short player = 0;
        player |= data.Data.at(index+2);
        player |= data.Data.at(index+3) << 8;

        return player;
    }

    return -1;
}

void Map::QueueBlockPhysics(unsigned short X, unsigned short Y, unsigned short Z) {
    if (!BlockInBounds(X, Y, Z))
        return;

    int offset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, 1);

    bool physItemFound = (data.PhysicData.at(std::ceil(offset / 8)) & (1 << (offset % 8)) != 0);

    if (!physItemFound) {
        int mbdIndex = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);

        Block* bm = Block::GetInstance();
        MapBlock blockEntry = bm->GetBlock(data.Data.at(mbdIndex));
        unsigned char blockPhysics = blockEntry.Physics;
        std::string physPlugin = blockEntry.PhysicsPlugin;

        if (blockPhysics > 0 || !physPlugin.empty()) {
            data.PhysicData.at(std::ceil(offset / 8)) |= (1 << (offset % 8)); // -- Set bitmask
            auto physTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(blockEntry.PhysicsTime + Utils::RandomNumber(blockEntry.PhysicsRandom));

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

        MapBlock blockEntry = bm->GetBlock(data.Data.at(offset));

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

Vector3S MapMain::GetMapExportSize(const std::string& filename) {
    std::vector<unsigned char> tempData(10);
    int outputLen = GZIP::GZip_DecompressFromFile(tempData.data(), 10, filename);
    if (outputLen != 10) {
        Logger::LogAdd(MODULE_NAME, "Map not imported: Error unzipping.", LogType::L_ERROR, GLF);
        return Vector3S{};
    }
    // -- Read version and size info
    int versionNumber = 0;
    versionNumber = tempData[0];
    versionNumber |= tempData[1] << 8;
    versionNumber |= tempData[2] << 16;
    versionNumber |= tempData[3] << 24;
    if (versionNumber != 1000) {
        Logger::LogAdd(MODULE_NAME, "Map not imported, unknown version [" + filename + "]", LogType::L_ERROR, GLF);
        return Vector3S{};
    }
    Vector3S result;
    result.X = tempData[4];
    result.X |= tempData[5] << 8;
    result.Y = tempData[6];
    result.Y |= tempData[7] << 8;
    result.Z = tempData[8];
    result.Z |= tempData[9] << 8;
    return result;
}

int Map::BlockGetRank(unsigned short X, unsigned short Y, unsigned short Z) {
    int result = data.RankBuild;

    for(auto const &r : data.RankBoxes) {
        bool matches = (X >= r.X0 && X < r.X1 && Y >= r.Y0 && Y < r.Y1 && Z >= r.Z0 && Z < r.Z1);

        if (r.Rank > result && matches) {
            result = r.Rank;
        }
    }

    return result;
}

void Map::SetRankBox(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1,
                     unsigned short Z1, short rank) {

    for(auto i = 0; i < data.RankBoxes.size(); i++) {
        auto item = data.RankBoxes.at(i);
        if (item.X0 >= X0 && item.X1 <= X1 && item.Y0 >= Y0 && item.Y1 <= Y1 && item.Z0 >= Z0 && item.Z1 <= Z1) {
            data.RankBoxes.erase(data.RankBoxes.begin() + i);
            i--;
        }
    }

    MapRankElement mre{};
    mre.Rank = rank;
    mre.X0 = X0;
    mre.Y0 = Y0;
    mre.Z0 = Z0;

    mre.Z1 = Z1;
    mre.Y1 = Y1;
    mre.X1 = X1;
    data.RankBoxes.push_back(mre);
}

void Map::AddTeleporter(std::string id, MinecraftLocation start, MinecraftLocation end, MinecraftLocation destination, std::string destMapUniqueId, int destMapId) {
    MapTeleporterElement mte;
    Vector3S startVec = start.GetAsBlockCoords();
    Vector3S endVec = end.GetAsBlockCoords();
    Vector3S destVec = end.GetAsBlockCoords();

    if (startVec.X > endVec.X) {
        int tmp = startVec.X;
        startVec.X = endVec.X;
        endVec.X = tmp;
    }
    if (startVec.Y > endVec.Y) {
        int tmp = startVec.Y;
        startVec.Y = endVec.Y;
        endVec.Y = tmp;
    }
    if (startVec.Z > endVec.Z) {
        int tmp = startVec.Z;
        startVec.Z = endVec.Z;
        endVec.Z = tmp;
    }

    mte.Id = id;
    mte.X0 = startVec.X;
    mte.Y0 = startVec.Y;
    mte.Z0 = startVec.Z;
    mte.X1 = endVec.X;
    mte.Y1 = endVec.Y;
    mte.Z1 = endVec.Z;
    mte.DestX = destVec.X;
    mte.DestY = destVec.Y;
    mte.DestZ = destVec.Z;
    mte.DestLook = destination.Look;
    mte.DestRot = destination.Rotation;
    mte.DestMapId = destMapId;
    mte.DestMapUniqueId = destMapUniqueId;

    data.Teleporter.insert(std::make_pair(id, mte));
}

void Map::DeleteTeleporter(std::string id) {
    if (data.Teleporter.find(id) == data.Teleporter.end()) {
        return;
    }

    data.Teleporter.erase(id);
}

void Map::MapExport(MinecraftLocation start, MinecraftLocation end, std::string filename) {
    Vector3S startVec = start.GetAsBlockCoords();
    Vector3S endVec = end.GetAsBlockCoords();

    if (startVec.X > endVec.X) {
        int tmp = startVec.X;
        startVec.X = endVec.X;
        endVec.X = tmp;
    }
    if (startVec.Y > endVec.Y) {
        int tmp = startVec.Y;
        startVec.Y = endVec.Y;
        endVec.Y = tmp;
    }
    if (startVec.Z > endVec.Z) {
        int tmp = startVec.Z;
        startVec.Z = endVec.Z;
        endVec.Z = tmp;
    }
    int sizeX = endVec.X - startVec.X + 1;
    int sizeY = endVec.Y - startVec.Y + 1;
    int sizeZ = endVec.Z - startVec.Z + 1;
    int mapSize = sizeX * sizeY * sizeZ;
    std::vector<unsigned char> tempData(mapSize+10);
    int offset = 0;
    // -- Version Number: 1000
    tempData[offset++] = 232;
    tempData[offset++] = 3;
    tempData[offset++] = 0;
    tempData[offset++] = 0;
    // -- Sizes as shorts.
    tempData[offset++] = sizeX & 0xFF;
    tempData[offset++] = (sizeX & 0xFF00) >> 8;
    tempData[offset++] = sizeY & 0xFF;
    tempData[offset++] = (sizeY & 0xFF00) >> 8;
    tempData[offset++] = sizeZ & 0xFF;
    tempData[offset++] = (sizeZ & 0xFF00) >> 8;
    // -- now block data.
    for (int iz = startVec.Z; iz <= endVec.Z; iz++) {
        for (int iy = startVec.Y; iy <= endVec.Y; iy++) {
            for (int ix = startVec.X; ix <= endVec.X; ix++) {
                if (BlockInBounds(ix, iy, iz)) {
                    unsigned char currentBlock = GetBlockType(ix, iy, iz);
                    tempData[offset++] = currentBlock;
                } else {
                    tempData[offset++] = 0;
                }
            }
        }
    }
    // -- compress it
    GZIP::GZip_CompressToFile(tempData.data(), mapSize+10, filename);
    tempData.clear();
    Logger::LogAdd(MODULE_NAME, "Map exported (" + filename + ")", LogType::NORMAL, GLF);
}

void Map::MapImport(std::string filename, MinecraftLocation location, short scaleX, short scaleY, short scaleZ) {
    // -- Decompress
    std::vector<unsigned char> tempData(10);
    int outputLen = GZIP::GZip_DecompressFromFile(tempData.data(), 10, filename);
    if (outputLen != 10) {
        Logger::LogAdd(MODULE_NAME, "Map not imported: Error unzipping.", LogType::L_ERROR, GLF);
        return;
    }
    // -- Read version and size info
    int versionNumber = 0;
    versionNumber = tempData[0];
    versionNumber |= tempData[1] << 8;
    versionNumber |= tempData[2] << 16;
    versionNumber |= tempData[3] << 24;
    if (versionNumber != 1000) {
        Logger::LogAdd(MODULE_NAME, "Map not imported, unknown version [" + filename + "]", LogType::L_ERROR, GLF);
        return;
    }
    Vector3S startLoc = location.GetAsBlockCoords();
    short sizeX = 0;
    short sizeY = 0;
    short sizeZ = 0;
    sizeX = tempData[4];
    sizeX |= tempData[5] << 8;
    sizeY = tempData[6];
    sizeY |= tempData[7] << 8;
    sizeZ = tempData[8];
    sizeZ |= tempData[9] << 8;
    int mapSize = sizeX * sizeY * sizeZ;
    int X1 = startLoc.X + sizeX * scaleX;
    int Y1 = startLoc.Y + sizeY * scaleY;
    int Z1 = startLoc.Z + sizeZ * scaleZ;
    tempData.resize(mapSize + 10);
    GZIP::GZip_DecompressFromFile(tempData.data(), mapSize + 10, filename);
    for (int jz = startLoc.Z; jz < Z1; jz++) {
        int iz = (jz-startLoc.Z) / scaleZ;
        for (int jy = startLoc.Y; jy < Y1; jy++) {
            int iy = (jy-startLoc.Y) / scaleY;
            for (int jx = startLoc.X; jx < X1; jx++) {
                int ix = (jx - startLoc.X) / scaleX;
                int location = MapMain::GetMapOffset(ix, iy, iz, sizeX, sizeY, sizeZ, 1)+10;
                BlockChange(-1, jx, jy, jz, tempData.at(location), true, false, true, 10);
            }
        }
    }
    Logger::LogAdd(MODULE_NAME, "Map imported. (" + filename + ").", LogType::NORMAL, GLF);
    // -- If correct version, iterate through, apply scale, and map_block_change.
}

bool Map::BlockInBounds(unsigned short X, unsigned short Y, unsigned short Z) {
    return (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ);
}

void Map::SetEnvColors(int red, int green, int blue, int type) {
    data.ColorsSet = true;

    switch(type) {
        case 0:
            data.SkyColor = Utils::Rgb(red, green, blue);
            break;
        case 1:
            data.CloudColor = Utils::Rgb(red, green, blue);
            break;
        case 2:
            data.FogColor = Utils::Rgb(red, green, blue);
            break;
        case 3:
            data.alight = Utils::Rgb(red, green, blue);
            break;
        case 4:
            data.dlight = Utils::Rgb(red, green, blue);
            break;
    }

    Network* nm = Network::GetInstance();

    for(auto const &nc : nm->roClients) {
        CPE::AfterMapActions(nc);
    }
}

void Map::SetMapAppearance(std::string url, int sideblock, int edgeblock, int sidelevel) {
    if (url.find("https://") == std::string::npos && url.find("http://") == std::string::npos)
        url = "";

    data.CustomAppearance = true;
    data.CustomURL = url;
    data.SideBlock = sideblock;
    data.EdgeBlock = edgeblock;
    data.SideLevel = sidelevel;

    Network* nm = Network::GetInstance();

    for(auto const &nc : nm->roClients) {
        CPE::AfterMapActions(nc);
    }
}

void Map::SetHackControl(bool canFly, bool noclip, bool speeding, bool spawnControl, bool thirdperson, int jumpHeight) {
    data.Flying = canFly;
    data.NoClip = noclip;
    data.Speeding = speeding;
    data.SpawnControl = spawnControl;
    data.ThirdPerson = thirdperson;
    data.JumpHeight = jumpHeight;

    Network* nm = Network::GetInstance();

    for(auto const &nc : nm->roClients) {
        CPE::AfterMapActions(nc);
    }
}

MapBlockData Map::GetBlockData(Vector3S location) {
    int index = MapMain::GetMapOffset(location.X, location.Y, location.Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);

    MapBlockData result {
        data.Data.at(index),
        data.Data.at(index+1),
        static_cast<short>((data.Data.at(index+2) << 8) | data.Data.at(index+3))
    };

    return result;
}

void Map::SetBlockData(Vector3S location, MapBlockData mbData) {
    int index = MapMain::GetMapOffset(location.X, location.Y, location.Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
    data.Data.at(index) = mbData.type;
    data.Data.at(index+1) = mbData.metadata;
    data.Data.at(index + 2 ) = (mbData.lastPlayer & 0xFF00) >> 8;
    data.Data.at(index + 3) = mbData.lastPlayer & 0xFF;
}

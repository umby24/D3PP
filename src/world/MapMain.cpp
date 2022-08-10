#include "world/MapMain.h"

#include <string>
#include <world/ClassicWorldMapProvider.h>
#include "common/Files.h"
#include "common/PreferenceLoader.h"
#include "common/Logger.h"

#include "EventSystem.h"
#include "events/EventMapActionDelete.h"
#include "events/EventMapActionFill.h"
#include "events/EventMapActionSave.h"
#include "events/EventMapActionLoad.h"
#include "events/EventMapAdd.h"
#include "network/Network_Functions.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/Server.h"
#include "world/Entity.h"
#include "world/Map.h"
#include "world/D3MapProvider.h"
#include "world/Teleporter.h"
#include "world/Player.h"
#include "compression.h"
#include "System.h"
#include "watchdog.h"
#include "Utils.h"

const std::string MODULE_NAME = "MapMain";
D3PP::world::MapMain* D3PP::world::MapMain::Instance = nullptr;

D3PP::world::MapMain::MapMain() {
    this->Setup = [this] { Init(); };
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);
    TaskScheduler::RegisterTask(MODULE_NAME, *this);
    LastWriteTime = 0;
    SaveFile = false;
    SaveFileTimer = 0;
    mapSettingsLastWriteTime = 0;
    mapSettingsMaxChangesSec = 1100;
    mapSettingsTimerFileCheck = 0;

    phStarted = false;
    mbcStarted = false;
    TempId = 0;
}
void D3PP::world::MapMain::Init() {
    MapListLoad();
}

void D3PP::world::MapMain::MainFunc() {
    std::string mapListFile = Files::GetFile(MAP_LIST_FILE);
    long fileTime = Utils::FileModTime(mapListFile);
    
    if (System::IsRunning && !mbcStarted) {
        std::thread mbcThread([this]() { this->MapBlockChange(); });
        std::swap(BlockchangeThread, mbcThread);
        mbcStarted = true;
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
        // -- Auto save maps every 5 minutes
        if (m.second->SaveTime + 5*60 < time(nullptr) && m.second->loaded) {
            m.second->SaveTime = time(nullptr);
            AddSaveAction(0, m.first, "");
        }
        if (m.second->Clients > 0) {
            m.second->LastClient = time(nullptr);
            if (!m.second->loaded) {
                m.second->Reload();
            }
        }
        if (m.second->loaded && (time(nullptr) - m.second->LastClient) > 200) { // -- Unload unused maps after 3 minutes
            m.second->Unload();
        }
    }
    fileTime = Utils::FileModTime(Files::GetFile(MAP_SETTINGS_FILE));
    if (fileTime != mapSettingsLastWriteTime) {
        MapSettingsLoad();
    }
}

D3PP::world::MapMain* D3PP::world::MapMain::GetInstance() {
    if (Instance == nullptr)
        Instance = new MapMain();

    return Instance;
}

void D3PP::world::MapMain::AddSaveAction(int clientId, int mapId, const std::string& directory) {
    std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    std::function<void()> saveAction = [thisMap, clientId](){
        thisMap->Save("");
        if (clientId > 0) {
            NetworkFunctions::SystemMessageNetworkSend(clientId, "&eMap Saved.");
        }
        EventMapActionSave mas{};
        mas.actionId = -1;
        mas.mapId = thisMap->ID;
        Dispatcher::post(mas);
    };

    thisMap->m_actions.AddTask(saveAction);
}

void D3PP::world::MapMain::AddLoadAction(int clientId, int mapId, const std::string& directory) {
    std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    std::function<void()> loadAction = [thisMap, clientId, directory](){
        thisMap->Load(directory);
        thisMap->filePath = directory;

        if (clientId > 0) {
            NetworkFunctions::SystemMessageNetworkSend(clientId, "&eMap Loaded.");
        }
        EventMapActionLoad mal{};
        mal.actionId = -1;
        mal.mapId = thisMap->ID;
        Dispatcher::post(mal);
    };

    thisMap->m_actions.AddTask(loadAction);
}

void D3PP::world::MapMain::LoadImmediately(int mapId, const std::string& directory) {
    std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    thisMap->Load(directory);
    thisMap->filePath = directory;
}

void D3PP::world::MapMain::AddResizeAction(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z) {
    std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    Common::Vector3S newSize(X, Y, Z);

    std::function<void()> resizeAction = [thisMap, clientId, newSize](){
        thisMap->Resize(newSize.X, newSize.Y, newSize.Z);
        if (clientId > 0) {
            NetworkFunctions::SystemMessageNetworkSend(clientId, "&eMap Resized.");
        }
        EventMapActionFill maf{};
        maf.actionId = -1;
        maf.mapId = thisMap->ID;
        Dispatcher::post(maf);
    };

    thisMap->m_actions.AddTask(resizeAction);
    
}

void D3PP::world::MapMain::AddFillAction(int clientId, int mapId, std::string functionName, std::string argString) {
    std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    std::function<void()> fillAction = [thisMap, clientId, functionName, argString](){
        thisMap->Fill(functionName, argString);
        if (clientId > 0) {
            NetworkFunctions::SystemMessageNetworkSend(clientId, "&eMap Filled.");
        }
        EventMapActionFill mas{};
        mas.actionId = -1;
        mas.mapId = thisMap->ID;
        Dispatcher::post(mas);
    };

    thisMap->m_actions.AddTask(fillAction);
}

void D3PP::world::MapMain::AddDeleteAction(int clientId, int mapId) {
    EventMapActionDelete delAct{};
    delAct.actionId = -1;
    delAct.mapId = mapId;
    Dispatcher::post(delAct);

    Delete(mapId);
    if (clientId > 0) {
        NetworkFunctions::SystemMessageNetworkSend(clientId, "&eMap Deleted.");
    }
}

void D3PP::world::MapMain::MapBlockChange() {
    while (System::IsRunning) {
        watchdog::Watch("Map_Blockchanging", "Begin thread-slope", 0);

        for(auto const &m : _maps) {
            if (m.second->BlockchangeStopped || !m.second->loaded)
                continue;

            int maxChangedSec = 1100 / 10;
            ChangeQueueItem i{};

            while (maxChangedSec > 0) {
                if (m.second->bcQueue == nullptr) {
                    continue;
                }
                if (m.second->bcQueue->TryDequeue(i)) {
                    unsigned char currentMat = m.second->GetBlockType(i.Location.X, i.Location.Y, i.Location.Z);
                    NetworkFunctions::NetworkOutBlockSet2Map(m.first, i.Location.X, i.Location.Y, i.Location.Z,
                                                             currentMat);
                }
                maxChangedSec--;
            }
        }

        watchdog::Watch("Map_Blockchanging", "End thread-slope", 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//bool comparePhysicsTime(const TimeQueueItem& first, const TimeQueueItem& second) {
//    return (first.Time < second.Time);
//}

void D3PP::world::MapMain::MapBlockPhysics() {
    while (System::IsRunning) {
        watchdog::Watch("Map_Physic", "Begin Thread-Slope", 0);

        for(auto const &map : _maps) {
            if (map.second->PhysicsStopped)
                continue;
//            {
//                std::scoped_lock<std::mutex> myLock(map.second->physicsQueueMutex);
//                std::sort(map.second->PhysicsQueue.begin(), map.second->PhysicsQueue.end(), comparePhysicsTime);
//            }

            int counter = 0;
            TimeQueueItem physItem;

            while (counter < 1000) { // -- TODO: May need adjustment.
                if (map.second->pQueue == nullptr) { // -- Avoid edge cases while the map is still loading
                    continue;
                }
                if (map.second->pQueue != nullptr && map.second->pQueue->TryDequeue(physItem)) {
                    if (physItem.Time < std::chrono::steady_clock::now()) {
                        map.second->ProcessPhysics(physItem.Location.X, physItem.Location.Y, physItem.Location.Z);
                        counter++;
                    } else {
                        // -- Not ready yet, push it back to the end.
                        map.second->pQueue->TryQueue(physItem);
                    }
                }
                counter++;
            }

        }
        watchdog::Watch("Map_Physic", "End Thread-Slope", 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
}

std::shared_ptr<D3PP::world::Map> D3PP::world::MapMain::GetPointer(int id) {
    if (_maps.find(id) != _maps.end())
        return _maps[id];

    return nullptr;
}

std::shared_ptr<D3PP::world::Map> D3PP::world::MapMain::GetPointer(const std::string& name) {
    std::shared_ptr<Map> result = nullptr;
    for (auto const &mi : _maps) {
        if (Utils::InsensitiveCompare(mi.second->m_mapProvider->MapName, name)) {
            result = mi.second;
            break;
        }
    }
    
    return result;
}

int D3PP::world::MapMain::GetMapId() {
    int result = 0;

    while (true) {
        if (GetPointer(result) != nullptr) {
            result++;
            continue;
        }

        return result;
    }
}

std::string D3PP::world::MapMain::GetMapMOTDOverride(int mapId) {
    MapMain* mmInstance = GetInstance();
    std::string result;
    std::shared_ptr<Map> mapPtr = mmInstance->GetPointer(mapId);

    if (mapPtr == nullptr)
        return result;
    
    //result = mapPtr->MotdOverride;
    return result;
}

void D3PP::world::MapMain::MapListSave() {
    std::string fName = Files::GetFile(MAP_LIST_FILE);
    PreferenceLoader pl(fName, "");

    for(auto const &m : _maps) {
        pl.SelectGroup(stringulate(m.first));
        if (!m.second->m_mapProvider)
            continue;
        pl.Write("Name", m.second->m_mapProvider->MapName);
        pl.Write("Directory", m.second->filePath);
        pl.Write("Delete", 0);
        pl.Write("Reload", 0);
    }
    pl.SaveFile();

    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File saved. [" + fName + "]", LogType::NORMAL, GLF);
}

void D3PP::world::MapMain::MapListLoad() {
    std::string fName = Files::GetFile(MAP_LIST_FILE);
    PreferenceLoader pl(fName, "");
    pl.LoadFile();

    for(auto const &m : pl.SettingsDictionary) {
        if (m.first.empty())
            continue;
        int mapId = stoi(m.first);

        pl.SelectGroup(m.first);
        std::string mapName = pl.Read("Name", m.first);
        std::string directory = pl.Read("Directory", Files::GetFolder("Maps") + m.first + "/");
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
                mapPtr->filePath = directory;
            }
            if ((mapReload)) {
                LoadImmediately(mapId, directory);
            }
        }
    }
    SaveFile = false;
    // -- Load of CW Maps :)
    if (std::filesystem::is_directory("CWMaps")) {
        for (const auto &entry : std::filesystem::directory_iterator("CWMaps")) {
            std::string fileName = entry.path().filename().string();

            if (fileName.length() < 3) // -- exclude . and ..
                continue;

            if (entry.is_directory()) {
                continue;
            }

            if (fileName.ends_with(".cw")) {
                int newId = GetMapId();
                Add(newId, 64, 64, 64, "CWMaps/" + fileName);
                LoadImmediately(newId, "CWMaps/" + fileName);
            }
        }
    }

    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File loaded. [" + fName + "]", LogType::NORMAL, GLF);
}

int D3PP::world::MapMain::Add(int id, short x, short y, short z, const std::string& name) {
    bool createNew = false;
    if (id == -1) {
        id = GetMapId();
        createNew = true;
    }

    if (name.empty())
        return -1;

    if (GetPointer(id) != nullptr)
        return -1;


    std::shared_ptr<Map> newMap = std::make_shared<Map>();
    int mapSize = GetMapSize(x, y, z, MAP_BLOCK_ELEMENT_SIZE);
    newMap->ID = id;
    newMap->SaveTime = time(nullptr);
    newMap->loading = false;
    newMap->BlockchangeStopped = false;
    newMap->Clients = 0;
    newMap->LastClient = time(nullptr);
    Common::Vector3S sizeVector {x, y, z};
    if (!name.ends_with(".cw")) {
        newMap->m_mapProvider = std::make_unique<D3MapProvider>();
        newMap->filePath = Files::GetFolder("Maps") + name + "/";
    } else {
        newMap->m_mapProvider = std::make_unique<ClassicWorldMapProvider>();
        newMap->filePath = name;
    }

    if (createNew) {
        newMap->m_mapProvider->CreateNew(sizeVector, newMap->filePath, name);
        newMap->loaded = true;
    }

    newMap->bcQueue = std::make_unique<BlockChangeQueue>(sizeVector);
    newMap->pQueue = std::make_unique<PhysicsQueue>(sizeVector);


    _maps.insert(std::make_pair(id, newMap));
    SaveFile = true;
    

    EventMapAdd ema;
    ema.mapId = id;
    Dispatcher::post(ema);

    return id;
}

void D3PP::world::MapMain::Delete(int id) {
    std::shared_ptr<Map> mp = GetPointer(id);

    if (mp == nullptr)
        return;

    Network* nm = Network::GetInstance();

    if (mp->Clients > 0) {
        for(auto const &nc : D3PP::network::Server::roClients) {
            if (nc->GetLoggedIn() && nc->GetPlayerInstance()->GetEntity() != nullptr&& nc->GetPlayerInstance()->GetEntity()->MapID == id) {
                MinecraftLocation somewhere {0, 0, D3PP::Common::Vector3S{(short)0, (short)0, (short)0}};
                nc->GetPlayerInstance()->GetEntity()->PositionSet(0, somewhere, 4, true);
            }
        }
    }

    mp->Unload();
    _maps.erase(mp->ID);
    SaveFile = true;
}

void D3PP::world::MapMain::MapSettingsLoad() {
    std::string mapSettingsFile = Files::GetFile("Map_Settings");
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

void D3PP::world::MapMain::MapSettingsSave() {
    std::string hbSettingsFile = Files::GetFile("Map_Settings");
    json j;
    j["Max_Changes_s"] = mapSettingsMaxChangesSec;

    std::ofstream ofstream(hbSettingsFile);

    ofstream << std::setw(4) << j;
    ofstream.flush();
    ofstream.close();

    mapSettingsLastWriteTime = Utils::FileModTime(hbSettingsFile);
    Logger::LogAdd(MODULE_NAME, "File Saved [" + hbSettingsFile + "]", LogType::NORMAL, GLF);
}

D3PP::Common::Vector3S D3PP::world::MapMain::GetMapExportSize(const std::string& filename) {
    std::vector<unsigned char> tempData(10);
    int outputLen = GZIP::GZip_DecompressFromFile(tempData.data(), 10, filename);
    if (outputLen != 10) {
        Logger::LogAdd(MODULE_NAME, "Map not imported: Error unzipping.", LogType::L_ERROR, GLF);
        return Common::Vector3S{};
    }
    // -- Read version and size info
    int versionNumber = 0;
    versionNumber = tempData[0];
    versionNumber |= tempData[1] << 8;
    versionNumber |= tempData[2] << 16;
    versionNumber |= tempData[3] << 24;
    if (versionNumber != 1000) {
        Logger::LogAdd(MODULE_NAME, "Map not imported, unknown version [" + filename + "]", LogType::L_ERROR, GLF);
        return Common::Vector3S{};
    }
    Common::Vector3S result;
    result.X = tempData[4];
    result.X |= tempData[5] << 8;
    result.Y = tempData[6];
    result.Y |= tempData[7] << 8;
    result.Z = tempData[8];
    result.Z |= tempData[9] << 8;
    return result;
}



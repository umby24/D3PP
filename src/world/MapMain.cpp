
#include <ranges>
#include <string>
#include <world/ClassicWorldMapProvider.h>
#include "common/Files.h"
#include "common/PreferenceLoader.h"
#include "common/Logger.h"
#include "common/Configuration.h"

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
#include "world/Player.h"
#include "world/MapMain.h"

#include "Block.h"
#include "compression.h"
#include "System.h"
#include "watchdog.h"
#include "Utils.h"
#include "common/ByteBuffer.h"
#include "network/packets/BulkBlockChangePacket.h"


const std::string MODULE_NAME = "MapMain";
D3PP::world::MapMain* D3PP::world::MapMain::Instance = nullptr;

D3PP::world::MapMain::MapMain() {
    this->Setup = [this] { Init(); };
    this->Main = [this] { MainFunc(); };
    this->Teardown = [this] { Shutdown(); };
    this->Interval = std::chrono::seconds(1);
    this->LastRun = std::chrono::system_clock::now();

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
    LoadD3Maps();
    LoadMaps();
}

void D3PP::world::MapMain::Shutdown() {
    for (auto& [id, map] : _maps) {
        map->Unload();
    }

    if (BlockchangeThread.joinable()) {
        BlockchangeThread.join();
    }

    if (PhysicsThread.joinable()) {
        PhysicsThread.join();
    }
}

void D3PP::world::MapMain::MainFunc() {
    std::string mapListFile = Files::GetFile(MAP_LIST_FILE);
    
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

    if (_maps.empty()) {
        LoadMaps();
        LoadD3Maps();
    }

    for(const auto &[mapId, mapPtr] : _maps) {
        // -- Auto save maps every 5 minutes
        if ((std::chrono::system_clock::now() - mapPtr->SaveTime) > std::chrono::minutes(5) && mapPtr->loaded) {
            mapPtr->SaveTime = std::chrono::system_clock::now();
            AddSaveAction(0, mapId, "");
        }
        if (mapPtr->Clients > 0) {
            mapPtr->LastClient = std::chrono::system_clock::now();
            if (!mapPtr->loaded) {
                mapPtr->Reload();
            }
        }
        if (mapPtr->loaded && (std::chrono::system_clock::now() - mapPtr->LastClient) > std::chrono::minutes(3) && mapPtr->Clients == 0) { // -- Unload unused maps after 3 minutes
            mapPtr->Unload();
        }
    }

    if (const long fileTime = Utils::FileModTime(Files::GetFile(MAP_SETTINGS_FILE));
        fileTime != mapSettingsLastWriteTime) {
        MapSettingsLoad();
    }
}

D3PP::world::MapMain* D3PP::world::MapMain::GetInstance() {
    return Instance;
}

void D3PP::world::MapMain::AddSaveAction(int clientId, const int mapId, const std::string& directory) {
    std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    const std::function saveAction = [thisMap, clientId, directory](){
        if (const bool saveResult = thisMap->Save(directory); clientId > 0 && saveResult) {
            NetworkFunctions::SystemMessageNetworkSend(clientId, "&eMap Saved.");
        } else if (!saveResult) {
            Logger::LogAdd(MODULE_NAME, "Error saving map!", L_ERROR, GLF);
            NetworkFunctions::SystemMessageNetworkSend(clientId, "&eMap NOT SAVED!!");
        }

        EventMapActionSave mas{};
        mas.actionId = -1;
        mas.mapId = thisMap->ID;
        Dispatcher::post(mas);
    };

    thisMap->m_actions.AddTask(saveAction);
}

void D3PP::world::MapMain::AddLoadAction(int clientId, const int mapId, const std::string& directory) {
    std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    const std::function loadAction = [thisMap, clientId, directory](){
        // Import the file's contents into the current map but keep this map's
        // own name and save location (filePath) untouched.
        thisMap->Load(directory);

        if (clientId > 0) {
            NetworkFunctions::SystemMessageNetworkSend(clientId, "&eMap Loaded.");
        }
        thisMap->Resend();
        EventMapActionLoad mal{};
        mal.actionId = -1;
        mal.mapId = thisMap->ID;
        Dispatcher::post(mal);
    };

    thisMap->m_actions.AddTask(loadAction);
}

void D3PP::world::MapMain::LoadImmediately(const int mapId, const std::string& directory) {
    const std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    thisMap->Load(directory);
    thisMap->SetName(GetUniqueName(thisMap->Name(), thisMap->ID));
    thisMap->filePath = directory;
}

void D3PP::world::MapMain::AddResizeAction(int clientId, const int mapId, const unsigned short X, const unsigned short Y, const unsigned short Z) {
    std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    Vector3S newSize(X, Y, Z);

    const std::function resizeAction = [thisMap, clientId, newSize](){
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

void D3PP::world::MapMain::AddFillAction(int clientId, const int mapId, const std::string& functionName, const std::string& argString) {
    std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    const std::function fillAction = [thisMap, clientId, functionName, argString](){
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

void D3PP::world::MapMain::AddDeleteAction(const int clientId, const int mapId) {
    EventMapActionDelete delAct{};
    delAct.actionId = -1;
    delAct.mapId = mapId;
    Dispatcher::post(delAct);

    Delete(mapId);
    if (clientId > 0) {
        NetworkFunctions::SystemMessageNetworkSend(clientId, "&eMap Deleted.");
    }
}
inline int GetBulkIndex(D3PP::Common::Vector3S location, D3PP::Common::Vector3S mapSize) {
    return (location.Z * mapSize.Y + location.Y) * mapSize.X + location.X;
}

void D3PP::world::MapMain::MapBlockChange() const {
    while (System::IsRunning) {
        watchdog::Watch("Map_Blockchanging", "Begin thread-slope", 0);
        Block* b = Block::GetInstance();
        for(const auto &[mapId, mapPtr] : _maps) {
            if (mapPtr->BlockchangeStopped || !mapPtr->loaded)
                continue;
            ChangeQueueItem i{};
            int maxChangedSec = 5000;


            while (maxChangedSec > 0) {
                if (mapPtr->bcQueue == nullptr) {
                    continue;
                }

                if (mapPtr->bcQueue->GetSize() > 256) { // -- We have enough to build a bulk block update.
                    ByteBuffer indicies(nullptr);
                    unsigned char count = -1; // -- Because the count is minus one for some reason
                    std::vector<unsigned char> blocks;
                    for (int il = 0; il < 256; il++) {
                        if (mapPtr->bcQueue->TryDequeue(i)) {
                            unsigned char currentMat = mapPtr->GetBlockType(i.Location.X, i.Location.Y, i.Location.Z);
                            MapBlock mb = b->GetBlock(currentMat);
                            int index = GetBulkIndex(i.Location, mapPtr->GetSize());
                            indicies.Write(index);
                            blocks.push_back(mb.OnClient);
                            count++;
                        }
                    }

                    network::BulkBlockChangePacket packet(count, indicies.GetAllBytes(), blocks);
                    NetworkFunctions::PacketToMap(mapId, packet, "BulkBlockUpdate", 1);
                    maxChangedSec--; // -- I know it's wrong but it should supercharge the speed of bulk updates, which is the point.
                    continue;
                }

                // -- Singular mode
                if (mapPtr->bcQueue->TryDequeue(i)) {
                    unsigned char currentMat = mapPtr->GetBlockType(i.Location.X, i.Location.Y, i.Location.Z);
                    NetworkFunctions::NetworkOutBlockSet2Map(mapId, i.Location.X, i.Location.Y, i.Location.Z,
                                                             currentMat);
                }
                maxChangedSec--;
            }
        }

        watchdog::Watch("Map_Blockchanging", "End thread-slope", 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void D3PP::world::MapMain::MapBlockPhysics() const {
    while (System::IsRunning) {
        watchdog::Watch("Map_Physic", "Begin Thread-Slope", 0);

        for(const auto &map: _maps | std::views::values) {
            if (map->PhysicsStopped)
                continue;

            int counter = 0;
            TimeQueueItem physItem;

            while (counter < 5000) { // -- TODO: May need adjustment.
                if (map->pQueue == nullptr) { // -- Avoid edge cases while the map is still loading
                    continue;
                }
                if (map->pQueue != nullptr && map->pQueue->TryDequeue(physItem)) {
                    if (physItem.Time < std::chrono::steady_clock::now()) {
                        map->ProcessPhysics(physItem.Location.X, physItem.Location.Y, physItem.Location.Z);
                        counter++;
                    } else {
                        // -- The queue is a min-heap by time, so if the soonest item
                        // -- isn't due yet then nothing else is either. Put it back and
                        // -- stop draining this map so we don't burn the budget spinning.
                        map->pQueue->TryQueue(physItem);
                        break;
                    }
                }
                counter++;
            }

        }
        watchdog::Watch("Map_Physic", "End Thread-Slope", 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
}

std::shared_ptr<D3PP::world::Map> D3PP::world::MapMain::GetPointer(const int id) {
    if (_maps.contains(id))
        return _maps[id];

    return nullptr;
}

D3PP::world::MapMain::~MapMain()
{
    Shutdown();
}

std::shared_ptr<D3PP::world::Map> D3PP::world::MapMain::GetPointer(const std::string &name) const
{
    std::shared_ptr<Map> result = nullptr;
    for (const auto &map: _maps | std::views::values) {
        if (Utils::InsensitiveCompare(map->m_mapProvider->MapName, name)) {
            result = map;
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

void D3PP::world::MapMain::MapListSave() {
    const std::string fName = Files::GetFile(MAP_LIST_FILE);
    PreferenceLoader pl(fName, "");

    for(const auto &[mapId, mapPtr] : _maps) {
        pl.SelectGroup(stringulate(mapId));
        if (!mapPtr->m_mapProvider)
            continue;
        pl.Write("Name", mapPtr->m_mapProvider->MapName);
        pl.Write("Directory", mapPtr->filePath);
        pl.Write("Delete", 0);
        pl.Write("Reload", 0);
    }
    pl.SaveFile();

    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File saved. [" + fName + "]", NORMAL, GLF);
}

void D3PP::world::MapMain::MapListLoad() {
    const std::string fName = Files::GetFile(MAP_LIST_FILE);
    PreferenceLoader pl(fName, "");
    pl.LoadFile();

    for(const auto &key: pl.SettingsDictionary | std::views::keys) {
        if (key.empty())
            continue;
        const int mapId = stoi(key);

        pl.SelectGroup(key);
        std::string mapName = pl.Read("Name", key);
        std::string directory = pl.Read("Directory", Files::GetFolder("Maps") + key + "/");
        const bool mapDelete = (pl.Read("Delete", 0) == 1);
        bool mapReload = (pl.Read("Reload", 0) == 1);
        if (mapDelete) {
            AddDeleteAction(0, mapId);
        } else {
            if (std::shared_ptr<Map> mapPtr = GetPointer(mapId); mapPtr == nullptr) {
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


    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File loaded. [" + fName + "]", NORMAL, GLF);
}

int D3PP::world::MapMain::Add(int id, const short x, const short y, const short z, std::string name) {
    bool createNew = false;
    if (id == -1) {
        id = GetMapId();
        createNew = true;
    }

    if (name.empty())
        return -1;

    if (GetPointer(id) != nullptr)
        return -1;


    auto newMap = std::make_shared<Map>();
    newMap->ID = id;
    newMap->SaveTime = std::chrono::system_clock::now();
    newMap->BlockchangeStopped = false;
    newMap->Clients = 0;
    newMap->LastClient = std::chrono::system_clock::now();
    Vector3S sizeVector {x, y, z};
    if (createNew) {
        name = GetUniqueName(newMap->filePath);
    }

    if (name.ends_with("D3")) {
        newMap->m_mapProvider = std::make_unique<D3MapProvider>();
        newMap->filePath = Files::GetFolder("Maps") + name + "/";
    } else {

        newMap->m_mapProvider = std::make_unique<ClassicWorldMapProvider>();
        if (!name.ends_with(".cw"))
            newMap->filePath = Files::GetFolder("CWMaps") + name + ".cw";
        else {
            newMap->filePath = Files::GetFolder("CWMaps") + name;
            name = name.substr(0, name.size() - 3);
        }

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

void D3PP::world::MapMain::Delete(const int id) {
    const std::shared_ptr<Map> mp = GetPointer(id);

    if (mp == nullptr)
        return;

    if (mp->Clients > 0) {
        for(auto const &nc : network::Server::roClients) {
            if (nc->GetLoggedIn() && nc->GetPlayerInstance()->GetEntity() != nullptr&& nc->GetPlayerInstance()->GetEntity()->MapID == id) {
                const MinecraftLocation somewhere {0, 0, Vector3S{static_cast<short>(0), static_cast<short>(0), static_cast<short>(0)}};
                nc->GetPlayerInstance()->GetEntity()->PositionSet(0, somewhere, 4, true);
            }
        }
    }

    mp->Unload();
    _maps.erase(mp->ID);
    SaveFile = true;
}

void D3PP::world::MapMain::MapSettingsLoad() {
    const std::string mapSettingsFile = Files::GetFile("Map_Settings");
    json j;
    std::ifstream iStream(mapSettingsFile);
    if (!iStream.is_open()) {
        Logger::LogAdd(MODULE_NAME, "Failed to load Map settings, generating...", WARNING, GLF);
        MapSettingsSave();
        return;
    }

    try {
        iStream >> j;
    } catch (int exception) {
        Logger::LogAdd(MODULE_NAME, "Failed to load Map settings, [E: " + stringulate(exception) + "]", WARNING, GLF);
        return;
    }
    iStream.close();

    mapSettingsMaxChangesSec = j["Max_Changes_s"];
    mapSettingsLastWriteTime = Utils::FileModTime(mapSettingsFile);

    Logger::LogAdd(MODULE_NAME, "File Loaded [" + mapSettingsFile + "]", NORMAL, GLF);
}

void D3PP::world::MapMain::MapSettingsSave() {
    const std::string hbSettingsFile = Files::GetFile("Map_Settings");
    json j;
    j["Max_Changes_s"] = mapSettingsMaxChangesSec;

    std::ofstream ofstream(hbSettingsFile);

    ofstream << std::setw(4) << j;
    ofstream.flush();
    ofstream.close();

    mapSettingsLastWriteTime = Utils::FileModTime(hbSettingsFile);
    Logger::LogAdd(MODULE_NAME, "File Saved [" + hbSettingsFile + "]", NORMAL, GLF);
}

Vector3S D3PP::world::MapMain::GetMapExportSize(const std::string& filename) {
    std::vector<unsigned char> tempData(10);

    if (const int outputLen = GZIP::GZip_DecompressFromFile(tempData.data(), 10, filename); outputLen != 10) {
        Logger::LogAdd(MODULE_NAME, "Map not imported: Error unzipping.", L_ERROR, GLF);
        return Vector3S{};
    }
    // -- Read version and size info
    int versionNumber = 0;
    versionNumber = tempData[0];
    versionNumber |= tempData[1] << 8;
    versionNumber |= tempData[2] << 16;
    versionNumber |= tempData[3] << 24;
    if (versionNumber != 1000) {
        Logger::LogAdd(MODULE_NAME, "Map not imported, unknown version [" + filename + "]", L_ERROR, GLF);
        return Vector3S{};
    }
    Vector3S result{};
    result.X = tempData[4];
    result.X |= tempData[5] << 8;
    result.Y = tempData[6];
    result.Y |= tempData[7] << 8;
    result.Z = tempData[8];
    result.Z |= tempData[9] << 8;
    return result;
}

std::string D3PP::world::MapMain::GetUniqueName(const std::string &desired, int ignoreId) const {
    std::string result = desired;
    int antiCollideNumber = 1;
    // -- Collision.
    while (true) {
        std::shared_ptr<Map> collideMap = GetPointer(result);
        if (collideMap == nullptr || collideMap->ID == ignoreId) {
            if (result != desired)
                Logger::LogAdd(MODULE_NAME, "Map name taken, temporary renaming in place [" + desired +
                    "] -> [" + result + "]", WARNING, GLF);
            return result;
        }
        result = desired + "_" + stringulate(antiCollideNumber++);

    }
}

void D3PP::world::MapMain::LoadD3Maps() {
    const std::string mapDir = Files::GetFolder("Maps");
    if ( !std::filesystem::is_directory(mapDir)) return;

    for (const auto &entry : std::filesystem::directory_iterator(mapDir)) {
        std::string fileName = entry.path().filename().string();

        if (fileName.length() < 3) // -- exclude . and ..
            continue;

        if (entry.is_directory()) {
            const int dataSize = Utils::FileSize(mapDir + fileName + "/Data-Layer.gz");

            if (const int configSize = Utils::FileSize(mapDir + fileName + "/Config.txt"); dataSize != -1 && configSize != -1) {
                const int newMapId = GetMapId();
                Add(newMapId, 64, 64 ,64, mapDir + fileName);
                LoadImmediately(newMapId, mapDir + fileName);
            }
        }
    }
}

void D3PP::world::MapMain::LoadMaps() {
    std::string mapDir = Files::GetFolder("CWMaps");

    if (mapDir.empty()) {
        mapDir = "CWMaps";
    }

    if (Utils::FileSize(mapDir + Configuration::GenSettings.defaultMap) == -1 && Utils::FileSize(mapDir + Configuration::GenSettings.defaultMap + "u") == -1) {
        files::ClassicWorld cwMap(Vector3S{static_cast<short>(64), 64, 64});
        cwMap.MapName = "default";
        cwMap.Save(mapDir + Configuration::GenSettings.defaultMap);
        Logger::LogAdd(MODULE_NAME, "Default map created", NORMAL, GLF);
    }
    const int defaultId = GetMapId();
    Add(defaultId, 64, 64, 64, mapDir + Configuration::GenSettings.defaultMap);
    LoadImmediately(defaultId, mapDir + Configuration::GenSettings.defaultMap);

    if (std::filesystem::is_directory(mapDir)) {
        for (const auto &entry : std::filesystem::directory_iterator(mapDir)) {
            std::string fileName = entry.path().filename().string();

            if (fileName.length() < 3) // -- exclude . and ..
                continue;

            if (entry.is_directory()) {
                continue;
            }

            if (fileName.ends_with(".cw") && fileName != Configuration::GenSettings.defaultMap) {
                const int newId = GetMapId();
                Add(newId, 64, 64, 64, mapDir + fileName);
                LoadImmediately(newId, mapDir + fileName);
            }
        }
    }
}



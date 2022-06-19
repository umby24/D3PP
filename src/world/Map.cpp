//
// Created by unknown on 4/2/21.
//

#include "world/Map.h"

#include <utility>
#include <world/D3MapProvider.h>

#include "common/Files.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "System.h"
#include "common/ByteBuffer.h"
#include "common/Logger.h"
#include "common/UndoItem.h"
#include "compression.h"
#include "Utils.h"
#include "Block.h"
#include "watchdog.h"
#include "network/Network_Functions.h"
#include "network/Packets.h"
#include "world/Entity.h"
#include "world/Player.h"
#include "common/Player_List.h"
#include "plugins/PluginManager.h"
#include "world/Physics.h"
#include "CPE.h"
#include "EventSystem.h"
#include "events/EventMapActionDelete.h"
#include "events/EventMapActionFill.h"
#include "events/EventMapActionSave.h"
#include "events/EventMapActionLoad.h"
#include "events/EventMapBlockChange.h"
#include "events/EventMapBlockChangeClient.h"
#include "events/EventMapAdd.h"
#include "world/Teleporter.h"

using namespace D3PP::world;
using namespace D3PP::Common;
using namespace D3PP::files;

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

    phStarted = false;
    mbcStarted = false;
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
    fileTime = Utils::FileModTime(f->GetFile(MAP_SETTINGS_FILE));
    if (fileTime != mapSettingsLastWriteTime) {
        MapSettingsLoad();
    }
}

MapMain* MapMain::GetInstance() {
    if (Instance == nullptr)
        Instance = new MapMain();

    return Instance;
}

void MapMain::AddSaveAction(int clientId, int mapId, const std::string& directory) {
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

void MapMain::AddLoadAction(int clientId, int mapId, const std::string& directory) {
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

void MapMain::AddResizeAction(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z) {
    std::shared_ptr<Map> thisMap = GetPointer(mapId);

    if (thisMap == nullptr)
        return;

    Vector3S newSize(X, Y, Z);

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

void MapMain::AddFillAction(int clientId, int mapId, std::string functionName, std::string argString) {
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

void MapMain::AddDeleteAction(int clientId, int mapId) {
    EventMapActionDelete delAct{};
    delAct.actionId = -1;
    delAct.mapId = mapId;
    Dispatcher::post(delAct);

    Delete(mapId);
    if (clientId > 0) {
        NetworkFunctions::SystemMessageNetworkSend(clientId, "&eMap Deleted.");
    }
}

void MapMain::MapBlockChange() {
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

void MapMain::MapBlockPhysics() {
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

std::shared_ptr<Map> MapMain::GetPointer(int id) {
    if (_maps.find(id) != _maps.end())
        return _maps[id];

    return nullptr;
}

std::shared_ptr<Map> MapMain::GetPointer(const std::string& name) {
    std::shared_ptr<Map> result = nullptr;
    for (auto const &mi : _maps) {
        if (Utils::InsensitiveCompare(mi.second->m_mapProvider->MapName, name)) {
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

std::string MapMain::GetMapMOTDOverride(int mapId) {
    MapMain* mmInstance = GetInstance();
    std::string result;
    std::shared_ptr<Map> mapPtr = mmInstance->GetPointer(mapId);

    if (mapPtr == nullptr)
        return result;
    
    //result = mapPtr->MotdOverride;
    return result;
}

void MapMain::MapListSave() {
    Files* f = Files::GetInstance();
    std::string fName = f->GetFile(MAP_LIST_FILE);
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
                mapPtr->filePath = directory;
            }
            if ((mapReload)) {
                AddLoadAction(0, mapId, directory);
            }
        }
    }
    SaveFile = false;

    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File loaded. [" + fName + "]", LogType::NORMAL, GLF);
}

int MapMain::Add(int id, short x, short y, short z, const std::string& name) {
    bool createNew = false;
    if (id == -1) {
        id = GetMapId();
        createNew = true;
    }

    if (name.empty())
        return -1;

    if (GetPointer(id) != nullptr)
        return -1;


    Files *f = Files::GetInstance();

    std::shared_ptr<Map> newMap = std::make_shared<Map>();
    int mapSize = GetMapSize(x, y, z, MAP_BLOCK_ELEMENT_SIZE);
    newMap->ID = id;
    newMap->SaveTime = time(nullptr);
    newMap->loading = false;
    newMap->BlockchangeStopped = false;
    newMap->Clients = 0;
    newMap->LastClient = time(nullptr);
    Vector3S sizeVector {x, y, z};
    newMap->m_mapProvider = std::make_unique<D3MapProvider>();
    newMap->filePath = f->GetFolder("Maps") + name + "/";
    
    if (createNew)
        newMap->m_mapProvider->CreateNew(sizeVector, newMap->filePath, name);

    newMap->bcQueue = std::make_unique<BlockChangeQueue>(sizeVector);
    newMap->pQueue = std::make_unique<PhysicsQueue>(sizeVector);


    _maps.insert(std::make_pair(id, newMap));
    SaveFile = true;
    newMap->loaded = true;

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

    if (mp->Clients > 0) {
        for(auto const &nc : nm->roClients) {
            if (nc->LoggedIn && nc->player->tEntity != nullptr&& nc->player->tEntity->MapID == id) {
                MinecraftLocation somewhere {0, 0, Vector3S{(short)0, (short)0, (short)0}};
                nc->player->tEntity->PositionSet(0, somewhere, 4, true);
            }
        }
    }

    mp->Unload();
    _maps.erase(mp->ID);
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
    if (!loaded) {
        Reload();
        if (!loaded) {
            return false;
        }
    }

    loading = true;
    m_mapProvider->SetSize(Vector3S{x, y, z});
    bcQueue.reset();
    pQueue.reset();
    pQueue = std::make_unique<PhysicsQueue>(GetSize());
    bcQueue = std::make_unique<BlockChangeQueue>(GetSize());
    loading = false;
    Resend();
    return true;
}

void Map::Fill(const std::string& functionName, const std::string& paramString) {
    if (!loaded) {
        Reload();
        if (!loaded) {
            return;
        }
    }

    //UniqueID = MapMain::GetUniqueId();
    //RankBoxes.clear();
    Portals.clear();
    bcQueue->Clear();
    pQueue->Clear();
    Vector3S mapSize = m_mapProvider->GetSize();
    int mapSizeInt = (mapSize.X * mapSize.Y * mapSize.Z)*4;
    clock_t start = clock();
    std::vector<unsigned char> blankMap;
    blankMap.resize(mapSizeInt);

    m_mapProvider->SetBlocks(blankMap);

    D3PP::plugins::PluginManager *pm = D3PP::plugins::PluginManager::GetInstance();
    pm->TriggerMapFill(ID, mapSize.X, mapSize.Y, mapSize.Z, "Mapfill_" + functionName, std::move(paramString));
    Resend();
    Logger::LogAdd(MODULE_NAME, "Map '" + m_mapProvider->MapName + "' filled.", LogType::NORMAL, GLF);
}

bool Map::Save(const std::string& directory) {
    if (!this->loaded)
        return true;

    return m_mapProvider->Save(directory);
}

void Map::Load(const std::string& directory) {
    loading = true;

    if (m_mapProvider == nullptr && !directory.empty()) {
        m_mapProvider = std::make_unique<D3MapProvider>();
    } else if (m_mapProvider == nullptr) {
        Logger::LogAdd(MODULE_NAME, "Map Reloaded [" + m_mapProvider->MapName + "]", LogType::NORMAL, GLF);
    }

    bool result = m_mapProvider->Load(directory);

    if (!result) {
        Logger::LogAdd(MODULE_NAME, "Error loading map! [" + m_mapProvider->MapName + "]", L_ERROR, GLF);
        loading = false;
        return;
    }

    if (pQueue != nullptr) {
        pQueue.reset();
    }
    if (bcQueue != nullptr) {
        bcQueue.reset();
    }

    pQueue = std::make_unique<PhysicsQueue>(GetSize());
    bcQueue = std::make_unique<BlockChangeQueue>(GetSize());

    loading = false;
}

void Map::Reload() {
    if (loaded)
        return;

    loading = true;
    bool result = m_mapProvider->Reload();
    
    if (!result) {
        Logger::LogAdd(MODULE_NAME, "Failed to reload map! [" + m_mapProvider->MapName + "]", LogType::L_ERROR, GLF);
        loading = false;
        return;
    }

    loading = false;
    loaded = true;
    BlockchangeStopped = false;
    PhysicsStopped = false;
    Logger::LogAdd(MODULE_NAME, "Map Reloaded [" + m_mapProvider->MapName + "]", LogType::NORMAL, GLF);
}

void Map::Unload() {
    if (!loaded)
        return;

    m_mapProvider->Unload();
    BlockchangeStopped = true;
    PhysicsStopped = true;
    loaded = false;
    Logger::LogAdd(MODULE_NAME, "Map unloaded (" + m_mapProvider->MapName + ")", LogType::NORMAL, GLF);
}

void Map::Send(int clientId) {
    Network* nMain = Network::GetInstance();
    Block* bMain = Block::GetInstance();
    std::shared_ptr<IMinecraftClient> nc = nMain->GetClient(clientId);

    if (nc == nullptr)
        return;

    if (!loaded) {
        Reload();
        if (!loaded) {
            Logger::LogAdd(MODULE_NAME, "Can't send the map: Reload error", LogType::L_ERROR, GLF);
            nc->Kick("Mapsend error", false);
            return;
        }
    }

    Vector3S mapSize = m_mapProvider->GetSize();
    int mapVolume = mapSize.X * mapSize.Y * mapSize.Z;
    
    std::vector<unsigned char> tempBuf(mapVolume + 10);
    int tempBufferOffset = 0;

    tempBuf.at(tempBufferOffset++) = static_cast<unsigned char>(mapVolume >> 24);
    tempBuf.at(tempBufferOffset++) = static_cast<unsigned char>(mapVolume >> 16);
    tempBuf.at(tempBufferOffset++) = static_cast<unsigned char>(mapVolume >> 8);
    tempBuf.at(tempBufferOffset++) = static_cast<unsigned char>(mapVolume & 0xFF);

    int cbl = nc->GetCustomBlocksLevel();
    int dbl = CPE::GetClientExtVersion(nc, BLOCK_DEFS_EXT_NAME);
    std::vector<unsigned char> mapBlocks = m_mapProvider->GetBlocks();
    
    if (mapBlocks.size() != (mapVolume * 4)) {
        Logger::LogAdd("Map", "Error during mapsend: Size mismatch!!", LogType::L_ERROR, GLF);
        nc->SendChat("Error during mapsend!!");
        return;
    }

    for (int i = 0; i < mapVolume; i++) {
        int index = i * MAP_BLOCK_ELEMENT_SIZE;
        unsigned char blockAt = mapBlocks[index];
        if (blockAt < 49) { // -- If its an original block, Dont bother checking. Just speed past.
            tempBuf[tempBufferOffset++] = blockAt;
            continue;
        }
        if (blockAt > 65 && dbl < 1) { // -- If the user doesn't support customblocks and this is a custom block, replace with stone.
            tempBuf[tempBufferOffset++] = 1;
            continue;
        }

        MapBlock mb = bMain->GetBlock(blockAt);

        if (mb.CpeLevel > cbl)
            tempBuf[tempBufferOffset++] = static_cast<char>(mb.CpeReplace);
        else     
            tempBuf[tempBufferOffset++] = static_cast<char>(mb.OnClient);
    }

    mapBlocks.clear();

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
    CPE::DuringMapActions(nc);
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
    Packets::SendMapFinalize(clientId, mapSize.X, mapSize.Y, mapSize.Z);
    CPE::AfterMapActions(nc);
}

void Map::Resend() {
    Network* nMain = Network::GetInstance();

    for(auto const &nc : nMain->roClients) {
        if (nc->player == nullptr)
            continue;

        if (nc->player->MapId == ID) {
            nc->player->SendMap();
        }
    }

    Vector3S mapSize= m_mapProvider->GetSize();

    for (auto const &me : Entity::AllEntities) {
        if (me.second->MapID == ID) {
            me.second->Resend(me.second->Id);
        }
    }

    bcQueue->Clear();
    pQueue->Clear();
}

void Map::BlockChange(const std::shared_ptr<IMinecraftClient>& client, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char type) {
    if (client == nullptr)
        return;
    Vector3S blockLocation {(short)X, (short)Y, (short)Z};

    Block* bm = Block::GetInstance();
    auto rawBlock = m_mapProvider->GetBlock(blockLocation);

    if (client->IsStopped()) {
        NetworkFunctions::SystemMessageNetworkSend(client->GetId(), "&eYou are not allowed to build (stopped).");
        NetworkFunctions::NetworkOutBlockSet(client->GetId(), X, Y, Z, rawBlock);
        return;
    }

    unsigned char rawNewType;
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(client->GetId(), true);
    MapBlock oldType = bm->GetBlock(rawBlock);
    clientEntity->lastMaterial = type;
    int blockRank = BlockGetRank(X, Y, Z);

    if (mode > 0)
        rawNewType = type;
    else
        rawNewType = oldType.AfterDelete;

    MapBlock newType = bm->GetBlock(rawNewType);

    if (client->GetRank() < oldType.RankDelete) { // -- TODO: Externalize this?
        NetworkFunctions::SystemMessageNetworkSend(client->GetId(), "&eYou are not allowed to delete this block type.");
        NetworkFunctions::NetworkOutBlockSet(client->GetId(), X, Y, Z, oldType.OnClient);
        return;
    } else if (client->GetRank()  < newType.RankPlace) {
        NetworkFunctions::SystemMessageNetworkSend(client->GetId(), "&eYou are not allowed to build this block type.");
        NetworkFunctions::NetworkOutBlockSet(client->GetId(), X, Y, Z, oldType.OnClient);
        return;
    } else if (client->GetRank() < blockRank) {
        NetworkFunctions::SystemMessageNetworkSend(client->GetId(), "&eYou are not allowed to build here.");
        NetworkFunctions::NetworkOutBlockSet(client->GetId(), X, Y, Z, oldType.OnClient);
        return;
    }
    EventMapBlockChangeClient mbc;
    mbc.clientId = client->GetId();
    mbc.mapId = ID;
    mbc.X = X;
    mbc.Y = Y;
    mbc.Z =  Z;
    mbc.bType = type;
    mbc.mode = mode;
    Dispatcher::post(mbc);

    BlockChange(clientEntity->playerList->Number, X, Y, Z, rawNewType, true, true, true, 250);
    //QueueBlockChange(Vector3S(X, Y, Z), 250, -1);
    // -- PluginEventBlockCreate (one for delete, one for create.)

}

void AddUndoByPlayerId(short playerId, const UndoItem& item) {
    if (playerId == -1) return;

    Network* nMain = Network::GetInstance();

    for(auto const& nc : nMain->roClients) {
        if (nc->LoggedIn && nc->player && nc->player->tEntity && nc->player->tEntity->playerList) {
            if (nc->player->tEntity->playerList->Number == playerId) {
                nc->AddUndoItem(item);
                break;
            }
        }
    }

}

void Map::QueuePhysicsAround(const Vector3S& loc) {
    for (int ix = -1; ix < 2; ix++) {
        for (int iy = -1; iy < 2; iy++) {
            for (int iz = -1; iz < 2; iz++) {
                QueueBlockPhysics(Vector3S((short)(loc.X + ix), loc.Y + iy, loc.Z + iz));
            }
        }
    }
}

void Map::BlockChange (short playerNumber, unsigned short X, unsigned short Y, unsigned short Z, unsigned char type, bool undo, bool physic, bool send, unsigned char priority) {
    Vector3S locationVector(X,Y,Z);

    Block* bm = Block::GetInstance();
    auto roData = m_mapProvider->GetBlock(locationVector);

    EventMapBlockChange event;
    event.playerNumber = playerNumber;
    event.mapId = ID;
    event.X = X;
    event.Y = Y;
    event.Z = Z;
    event.bType = type;
    event.undo = undo;
    event.send = send;
    Dispatcher::post(event); // -- Post this event out!

    MapBlock oldType = bm->GetBlock(roData);
    short roLastPlayer = m_mapProvider->GetLastPlayer(locationVector);
    MapBlock newType = bm->GetBlock(type);


    if (type != roData && undo) {
        Common::UndoItem newUndoItem {locationVector, roData, type};
        AddUndoByPlayerId(playerNumber, newUndoItem);
    }

    if (type != roData && send) {
        QueueBlockChange(Vector3S(X, Y, Z), priority, roData);
    }

    m_mapProvider->SetBlock(locationVector, type);
    m_mapProvider->SetLastPlayer(locationVector, playerNumber);

    if (physic) {
        QueuePhysicsAround(locationVector);
    }
}

unsigned char Map::GetBlockType(unsigned short X, unsigned short Y, unsigned short Z) {
     if (!loaded && !loading) {
         LastClient = time(nullptr);
         Reload();
         if (!loaded) {
             return 255; // -- error reloading :( 
         }
     }

     if (loading) {
         while (loading) {
             std::this_thread::sleep_for(std::chrono::milliseconds(100));
         }
     }
     auto mapSize = m_mapProvider->GetSize();

     if (X > mapSize.X || Y > mapSize.Y || Z > mapSize.Z) {
         return 255;
     }

     return m_mapProvider->GetBlock(Vector3S(X, Y, Z));
}

void Map::QueueBlockChange(Common::Vector3S location, unsigned char priority, unsigned char oldType) const {
    while (loading) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ChangeQueueItem newQueueItem { location,
                                    oldType,
                                     priority
    };

    bcQueue->TryQueue(newQueueItem);
}

Map::Map() : IActions() {
    //ID = -1;
    loaded = false;
    loading = false;
    BlockchangeStopped = false;
    PhysicsStopped = false;
  //  SaveTime = 0;
   // LastClient = 0;
  //  Clients = 0;
}

void Map::BlockMove(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1,
                    unsigned short Z1, bool undo, bool physic, unsigned char priority) {

    Vector3S location0(X0, Y0, Z0);
    Vector3S location1(X1, Y1, Z1);

    auto oldBlockType0 = m_mapProvider->GetBlock(location0);
    auto oldBlockHistory0 = m_mapProvider->GetLastPlayer(location0);

    auto oldBlockType1 = m_mapProvider->GetBlock(location1);
    auto oldBlockHistory1 = m_mapProvider->GetLastPlayer(location1);

    if (undo) {
        if (oldBlockType0 != 0) {
            Common::UndoItem newUndoItem {location0, oldBlockType0, 0};
            AddUndoByPlayerId(oldBlockHistory0, newUndoItem);
        }

        if (oldBlockType0 != oldBlockType1) {
            Common::UndoItem newUndoItem {location1, oldBlockType1, oldBlockType0};
            AddUndoByPlayerId(oldBlockHistory0, newUndoItem);
        }
    }

    if (oldBlockType0 != 0) {
        QueueBlockChange(Vector3S(X0, Y0, Z0), priority, oldBlockType0);
    }

    if (oldBlockType1 != oldBlockType0) {
        QueueBlockChange(Vector3S(X1, Y1, Z1), priority, oldBlockType1);
    }

    oldBlockType1 = oldBlockType0;
    oldBlockType0 = 0;

    m_mapProvider->SetBlock(location0, oldBlockType0);
    m_mapProvider->SetBlock(location1, oldBlockType1);
    m_mapProvider->SetLastPlayer(location0, -1);
    m_mapProvider->SetLastPlayer(location1, oldBlockHistory0);

    if (physic) {
        QueuePhysicsAround(location0);
        QueuePhysicsAround(location1);
    }

}

unsigned short Map::GetBlockPlayer(unsigned short X, unsigned short Y, unsigned short Z) {
    return m_mapProvider->GetLastPlayer(Vector3S(X, Y, Z));
}

void Map::QueueBlockPhysics(Common::Vector3S location) {
    Block* bm = Block::GetInstance();
    MapBlock blockEntry = bm->GetBlock(m_mapProvider->GetBlock(location));
    unsigned char blockPhysics = blockEntry.Physics;
    std::string physPlugin = blockEntry.PhysicsPlugin;

    if (blockPhysics > 0 || !physPlugin.empty()) {
        auto physTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(blockEntry.PhysicsTime + Utils::RandomNumber(blockEntry.PhysicsRandom));

        TimeQueueItem physicItem { location, physTime };
        pQueue->TryQueue(physicItem);
    }
}

void Map::ProcessPhysics(unsigned short X, unsigned short Y, unsigned short Z) {
    Block* bm = Block::GetInstance();
    MapMain* mapMain= MapMain::GetInstance();

        MapBlock blockEntry = bm->GetBlock(m_mapProvider->GetBlock(Vector3S(X, Y, Z)));
        switch (blockEntry.Physics) {
            case 10:
                Physics::BlockPhysics10(mapMain->GetPointer(ID), X, Y, Z);
                break;
            case 11:
                Physics::BlockPhysics11(mapMain->GetPointer(ID), X, Y, Z);
                break;
            case 20:
                Physics::BlockPhysics20(mapMain->GetPointer(ID), X, Y, Z);
                break;
            case 21:
                Physics::BlockPhysics21(mapMain->GetPointer(ID), X, Y, Z);
                break;
        }

        if (!blockEntry.PhysicsPlugin.empty()) {
            D3PP::plugins::PluginManager *pm = D3PP::plugins::PluginManager::GetInstance();
            std::string pluginName = blockEntry.PhysicsPlugin;
            Utils::replaceAll(pluginName, "Lua:", "");
            pm->TriggerPhysics(ID, X, Y, Z, pluginName);
        }

        if (blockEntry.PhysicsRepeat) {
            QueueBlockPhysics(Vector3S(X, Y, Z));
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
//    int result = RankBuild;
//
//    for(auto const &r : RankBoxes) {
//        bool matches = (X >= r.X0 && X < r.X1 && Y >= r.Y0 && Y < r.Y1 && Z >= r.Z0 && Z < r.Z1);
//
//        if (r.Rank > result && matches) {
//            result = r.Rank;
//        }
//    }
//
//    return result;
    return -1;
}

void Map::SetRankBox(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1,
                     unsigned short Z1, short rank) {
//
//    for(auto i = 0; i < RankBoxes.size(); i++) {
//        auto item = RankBoxes.at(i);
//        if (item.X0 >= X0 && item.X1 <= X1 && item.Y0 >= Y0 && item.Y1 <= Y1 && item.Z0 >= Z0 && item.Z1 <= Z1) {
//            RankBoxes.erase(RankBoxes.begin() + i);
//            i--;
//        }
//    }
//
//    MapRankElement mre{};
//    mre.Rank = rank;
//    mre.X0 = X0;
//    mre.Y0 = Y0;
//    mre.Z0 = Z0;
//
//    mre.Z1 = Z1;
//    mre.Y1 = Y1;
//    mre.X1 = X1;
//    RankBoxes.push_back(mre);
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

    Teleporter newTp(start, end, destination, id, destMapUniqueId);
    Portals.push_back(newTp);
}

void Map::DeleteTeleporter(std::string id) {
    int index = -1;

    for(int i = 0; i < Portals.size(); i++) {
        if (Portals.at(i).Name == id) {
            index = i;
            break;
        }
    }
    if (index != -1) {
        Portals.erase(Portals.begin() + index);
    }
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
    // -- now block 
    for (int iz = startVec.Z; iz <= endVec.Z; iz++) {
        for (int iy = startVec.Y; iy <= endVec.Y; iy++) {
            for (int ix = startVec.X; ix <= endVec.X; ix++) {
                unsigned char currentBlock = GetBlockType(ix, iy, iz);
                tempData[offset++] = currentBlock;
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


std::vector<int> Map::GetEntities() {
    std::vector<int> result;

    for (auto const &e : Entity::AllEntities) {
        if (e.second->MapID == ID)
            result.push_back(e.second->Id);
    }

    return result;
}

void Map::RemoveEntity(std::shared_ptr<Entity> e) {
    Clients -= 1;
}

void Map::AddEntity(std::shared_ptr<Entity> e) {
    Clients += 1;
}

void Map::SetSpawn(MinecraftLocation location) {
    m_mapProvider->SetSpawn(location);
}

MapPermissions Map::GetMapPermissions() {
    return m_mapProvider->GetPermissions();
}

void Map::SetMapPermissions(const MapPermissions& perms) {
    m_mapProvider->SetPermissions(perms);
}

Teleporter Map::GetTeleporter(std::string id) {
    int index = -1;

    for(auto & Portal : Portals) {
        if (Portal.Name == id) {
            return Portal;
        }
    }

    return Teleporter();
}

void Map::SetMapEnvironment(const MapEnvironment &env) {
    m_mapProvider->SetEnvironment(env);
    Network* nm = Network::GetInstance();

    for(auto const &nc : nm->roClients) {
        CPE::AfterMapActions(nc);
    }
}


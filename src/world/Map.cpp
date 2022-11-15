//
// Created by unknown on 4/2/21.
//

#include "world/Map.h"

#include <utility>
#include <world/D3MapProvider.h>

#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/Server.h"
#include "System.h"
#include "common/ByteBuffer.h"
#include "common/Logger.h"
#include "common/UndoItem.h"
#include "compression.h"
#include "Utils.h"
#include "Block.h"
#include "network/Network_Functions.h"
#include "network/Packets.h"
#include "world/Entity.h"
#include "world/Player.h"
#include "common/Player_List.h"
#include "plugins/PluginManager.h"
#include "world/Physics.h"
#include "CPE.h"
#include "EventSystem.h"
#include "events/EventMapBlockChange.h"
#include "events/EventMapBlockChangeClient.h"
#include "world/Teleporter.h"
#include "world/MapMain.h"

using namespace D3PP::world;
using namespace D3PP::Common;
using namespace D3PP::files;

const std::string MODULE_NAME = "Map";

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
    RankBoxes.clear();
    Portals.clear();
    bcQueue->Clear();
    pQueue->Clear();
    Vector3S mapSize = m_mapProvider->GetSize();
    int mapSizeInt = (mapSize.X * mapSize.Y * mapSize.Z)*1;
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
    Particles = m_mapProvider->getParticles();
    Portals = m_mapProvider->getPortals();
    loading = false;
    loaded = true;

    Logger::LogAdd(MODULE_NAME, "Loaded map " + m_mapProvider->MapName, NORMAL, GLF);
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

    BlockchangeStopped = true;
    PhysicsStopped = true;
    loaded = false;
    m_mapProvider->Unload();
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

    for (int i = 0; i < mapVolume; i++) {
        unsigned char blockAt = mapBlocks[i];
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
    std::shared_lock lock(D3PP::network::Server::roMutex);
    for(auto const &nc : D3PP::network::Server::roClients) {
        if (nc->GetPlayerInstance() == nullptr)
            continue;
        auto concret = std::static_pointer_cast<D3PP::world::Player>(nc->GetPlayerInstance());

        if (nc->GetMapId() == ID) {
            concret->SendMap();
        }
    }

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
    
    if (mbc.isCancelled()) {
        NetworkFunctions::NetworkOutBlockSet(client->GetId(), X, Y, Z, oldType.OnClient);
        return;
    }

    BlockChange(clientEntity->playerList->Number, X, Y, Z, rawNewType, true, true, true, 250);
    D3PP::plugins::PluginManager* pm = D3PP::plugins::PluginManager::GetInstance();
    //QueueBlockChange(Vector3S(X, Y, Z), 250, -1);
    if (!oldType.DeletePlugin.empty() && oldType.DeletePlugin.starts_with("Lua:")) {
        std::string myPlug(oldType.DeletePlugin);
        Utils::replaceAll(myPlug, "Lua:", "");
        pm->TriggerBlockDelete(myPlug, ID, X, Y, Z);
    }
    if (!newType.CreatePlugin.empty() && newType.CreatePlugin.starts_with("Lua:")) {
        std::string myPlug(newType.CreatePlugin);
        Utils::replaceAll(myPlug, "Lua:", "");
        pm->TriggerBlockDelete(myPlug, ID, X, Y, Z);
    }

}

void AddUndoByPlayerId(short playerId, const UndoItem& item) {
    if (playerId == -1) return;

    std::shared_lock lock(D3PP::network::Server::roMutex);
    for(auto const& nc : D3PP::network::Server::roClients) {
        if (nc->GetLoggedIn() && nc->GetPlayerInstance() && nc->GetPlayerInstance()->GetEntity() && nc->GetPlayerInstance()->GetEntity()->playerList) {
            if (nc->GetPlayerInstance()->GetEntity()->playerList->Number == playerId) {
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

    if (event.isCancelled()) {
        return;
    }
    
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

Map::Map() : IActions(), RankBoxes(), Particles() {
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

int Map::BlockGetRank(unsigned short X, unsigned short Y, unsigned short Z) {
    MapPermissions perms = m_mapProvider->GetPermissions();

    int result = perms.RankBuild;

    for(auto const &r : RankBoxes) {
        bool matches = (X >= r.X0 && X < r.X1 && Y >= r.Y0 && Y < r.Y1 && Z >= r.Z0 && Z < r.Z1);

        if (r.Rank > result && matches) {
            result = r.Rank;
        }
    }

    return result;
}

void Map::SetRankBox(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1,
                     unsigned short Z1, short rank) {

    for(auto i = 0; i < RankBoxes.size(); i++) {
        auto item = RankBoxes.at(i);
        if (item.X0 >= X0 && item.X1 <= X1 && item.Y0 >= Y0 && item.Y1 <= Y1 && item.Z0 >= Z0 && item.Z1 <= Z1) {
            RankBoxes.erase(RankBoxes.begin() + i);
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
    RankBoxes.push_back(mre);

}

void Map::AddTeleporter(std::string id, MinecraftLocation start, MinecraftLocation end, MinecraftLocation destination, std::string destMapName) {
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
    start.SetAsBlockCoords(startVec);
    end.SetAsBlockCoords(endVec);
    
    Teleporter newTp(start, end, destination, id, destMapName);
    Portals.push_back(newTp);

    m_mapProvider->SetPortals(Portals);
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

    m_mapProvider->SetPortals(Portals);
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
    std::shared_lock lock(D3PP::network::Server::roMutex);
    for(auto const &nc : D3PP::network::Server::roClients) {
        CPE::AfterMapActions(nc);
    }
}

void Map::AddParticle(CustomParticle p) {
    Particles.push_back(p);
    m_mapProvider->SetParticles(Particles);
}

void Map::DeleteParticle(int effectId) {
    // std::erase_if(cm->Commands, [&commandName](const Command &c){ return c.Id == commandName; });
    erase_if(Particles, [&effectId](const CustomParticle &p){ return p.effectId == effectId; });
    m_mapProvider->SetParticles(Particles);
}


//
// Created by unknown on 4/2/21.
//
// Time for the biggest module in the server boys. 2242 lines of PB code.
//

#ifndef D3PP_MAP_H
#define D3PP_MAP_H
#define GLF __FILE__, __LINE__, __FUNCTION__
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <memory>
#include <filesystem>

#include "common/TaskScheduler.h"
#include "common/MinecraftLocation.h"
#include "common/Vectors.h"

#include "world/IMapProvider.h"
#include "world/MapActions.h"
#include "world/FillState.h"

#include "BlockChangeQueue.h"
#include "PhysicsQueue.h"

class IMinecraftClient;
class Entity;

namespace D3PP::world {
    class Teleporter;

    enum MapAction {
        SAVE = 0,
        LOAD,
        RESIZE = 5,
        FILL,
        DELETE = 10,
    };

    struct MapActionItem {
        int ID;
        int ClientID;
        int MapID;
        MapAction Action;
        std::string FunctionName;
        std::string Directory;
        Common::Vector3S Location;
        std::string ArgumentString;
    };

    struct MapBlockChanged {

    };

    struct UndoStep {
        short PlayerNumber;
        int MapId;
        short X;
        short Y;
        short Z;
        time_t Time;
        char TypeBefore;
        short PlayerNumberBefore;
    };

    const std::string MAP_LIST_FILE = "Map_List";
    const std::string MAP_SETTINGS_FILE = "Map_Settings";
    const std::string MAP_HTML_FILE = "Map_HTML";

    const int MAP_BLOCK_ELEMENT_SIZE = 4;

    class Map {
        friend class MapMain;

    public:
        int ID;
        time_t SaveTime;

        std::unique_ptr<PhysicsQueue> pQueue;
        std::unique_ptr<BlockChangeQueue> bcQueue;

        std::vector<UndoStep> UndoCache;
        std::vector<Teleporter> Portals;

        bool BlockchangeStopped, PhysicsStopped, loading, loaded;
        std::string filePath;
        time_t LastClient;
        int Clients;

        Map();
        std::string Name() { return m_mapProvider->MapName; }
        Common::Vector3S GetSize() { return Common::Vector3S{m_mapProvider->GetSize()}; }
        MinecraftLocation GetSpawn() { return m_mapProvider->GetSpawn(); }

        void SetSpawn(MinecraftLocation location);
        bool Resize(short x, short y, short z);
        void Fill(const std::string &functionName, const std::string& paramString);
        void BlockMove(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1,
                       unsigned short Z1, bool undo, bool physic, unsigned char priority);

        void BlockChange(const std::shared_ptr<IMinecraftClient> &client, unsigned short X, unsigned short Y,
                         unsigned short Z, unsigned char mode, unsigned char type);

        void BlockChange(short playerNumber, unsigned short X, unsigned short Y, unsigned short Z, unsigned char type,
                         bool undo, bool physic, bool send, unsigned char priority);

        void ProcessPhysics(unsigned short X, unsigned short Y, unsigned short Z);
        bool Save(const std::string& directory);
        void Load(const std::string& directory);
        unsigned char GetBlockType(unsigned short X, unsigned short Y, unsigned short Z);
        unsigned short GetBlockPlayer(unsigned short X, unsigned short Y, unsigned short Z);
        int BlockGetRank(unsigned short X, unsigned short Y, unsigned short Z);

        void SetRankBox(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1,
                        unsigned short Z1, short rank);

        void Reload();
        void Unload();
        void Send(int clientId);
        void Resend();

        void
        AddTeleporter(std::string id, MinecraftLocation start, MinecraftLocation end, MinecraftLocation destination,
                      std::string destMapUniqueId, int destMapId);

        void DeleteTeleporter(std::string id);
        Teleporter GetTeleporter(std::string id);
        void MapExport(MinecraftLocation start, MinecraftLocation end, std::string filename);
        void MapImport(std::string filename, MinecraftLocation location, short scaleX, short scaleY, short scaleZ);

        MapPermissions GetMapPermissions();
        void SetMapPermissions(const MapPermissions& perms);
        MapEnvironment GetMapEnvironment() { return m_mapProvider->GetEnvironment(); }
        void SetMapEnvironment(const MapEnvironment& env);

        std::vector<int> GetEntities();
        void RemoveEntity(std::shared_ptr<Entity> e);
        void AddEntity(std::shared_ptr<Entity> e);
        void SetBlocks(const std::vector<unsigned char>& blocks) { m_mapProvider->SetBlocks(blocks); }
        std::mutex BlockChangeMutex;
        std::unique_ptr<FillState> CurrentFillState;
    protected:
        std::unique_ptr<IMapProvider> m_mapProvider;
    private:
        MapActions m_actions;
        void QueueBlockPhysics(Common::Vector3S location);

        void QueueBlockChange(Common::Vector3S location, unsigned char priority,
                              unsigned char oldType) const;

        void  QueuePhysicsAround(const Common::Vector3S& loc);
    };

    class MapMain : TaskItem {
    public:
        MapMain();

        std::shared_ptr<Map> GetPointer(int id);
        std::shared_ptr<Map> GetPointer(const std::string& name);
        
        int Add(int id, short x, short y, short z, const std::string &name);
        void Delete(int id);
        static MapMain *GetInstance();
        static std::string GetMapMOTDOverride(int mapId);
        static int GetMapSize(int x, int y, int z, int blockSize) { return (x * y * z) * blockSize; }
        static int GetMapOffset(int x, int y, int z, int sizeX, int sizeY, int sizeZ, int blockSize) {
            return (x + y * sizeX + z * sizeX * sizeY) * blockSize;
        }
        static Common::Vector3S GetMapExportSize(const std::string &filename);
        void MainFunc();
        void AddSaveAction(int clientId, int mapId, const std::string &directory);
        void AddLoadAction(int clientId, int mapId, const std::string &directory);
        void AddResizeAction(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z);
        void AddFillAction(int clientId, int mapId, std::string functionName, std::string argString);
        void AddDeleteAction(int clientId, int mapId);
        bool SaveFile;
        std::map<int, std::shared_ptr<Map>> _maps;
    private:
        static MapMain *Instance;
        std::thread BlockchangeThread;
        std::thread PhysicsThread;
        bool mbcStarted;
        bool phStarted;

        time_t SaveFileTimer;
        std::string TempFilename;
        int TempId;
        std::string TempOverviewFilename;
        long LastWriteTime;

        std::vector<MapActionItem> _mapActions;

        // --
        long mapSettingsLastWriteTime;
        int mapSettingsTimerFileCheck;
        int mapSettingsMaxChangesSec;
        
        int GetMapId();
        void MapListSave();
        void MapListLoad();
        void MapSettingsSave();
        void MapSettingsLoad();
        void MapBlockChange();
        void MapBlockPhysics();
    };
}

#endif //D3PP_MAP_H

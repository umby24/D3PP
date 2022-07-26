#ifndef D3PP_WORLD_MAPMAIN_H
#define D3PP_WORLD_MAPMAIN_H

#include <memory>
#include <thread>

#include "common/TaskScheduler.h"
#include "common/MinecraftLocation.h"
#include "common/Vectors.h"

namespace D3PP::world {
    class Map;

    enum MapAction {
        SAVE = 0,
        LOAD = 2,
        RESIZE = 5,
        FILL = 7,
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
        void Init();
        void MainFunc();
        void AddSaveAction(int clientId, int mapId, const std::string &directory);
        void AddLoadAction(int clientId, int mapId, const std::string &directory);
        void LoadImmediately(int mapId, const std::string &directory);
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

#endif
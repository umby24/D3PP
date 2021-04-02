//
// Created by unknown on 4/2/21.
//
// Time for the biggest module in the server boys. 2242 lines of PB code.
//

#ifndef D3PP_MAP_H
#define D3PP_MAP_H
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <memory>

#include "TaskScheduler.h"
#include "Logger.h"
#include "ZLib.h"
#include "Utils.h"

struct MapBlockDo { // -- Physics Queue Item
    int time;
    unsigned short X;
    unsigned short Y;
    unsigned short Z;
};

struct MapBlockChanged {
    unsigned short X;
    unsigned short Y;
    unsigned short Z;
    char Priority;
    short OldMaterial;
};

struct UndoStep {
    short PlayerNumber;
    int MapId;
    short X;
    short Y;
    short Z;
    int Time;
    char TypeBefore;
    short PlayerNumberBefore;
};

struct MapRankElement {
    unsigned short X0;
    unsigned short Y0;
    unsigned short Z0;
    unsigned short X1;
    unsigned short Y1;
    unsigned short Z1;
    short Rank;
};

struct MapTeleporterElement {
    std::string Id;
    short X0;
    short Y0;
    short Z0;
    short X1;
    short Y1;
    short Z1;
    std::string DestMapUniqueId;
    int DestMapId;
    float DestX;
    float DestY;
    float DestZ;
    float DestRot;
    float DestLook;
};
enum OverviewType {
    None,
    TwoDee,
    Isomap
};
struct MapData {
    int ID;
    std::string UniqueID;
    std::string Name;
    int SaveInterval;
    int SaveTime;
    std::string Directory;
    OverviewType overviewType;
    unsigned short SizeX;
    unsigned short SizeY;
    unsigned short SizeZ;
    int blockCounter[256];
    float SpawnX;
    float SpawnY;
    float SpawnZ;
    float SpawnRot;
    float SpawnLook;
    char* Data; // -- Map data
    char* PhysicData; // -- Physics state, (1 Byte -> 8 blocks)
    char* BlockchangeData; // -- Blockchange state (1 byte -> 8 blocks)
    std::vector<MapBlockDo> PhysicsQueue;
    std::vector<MapBlockChanged> ChangeQueue;
    std::vector<UndoStep> UndoCache;
    std::vector<MapRankElement> RankBoxes;
    std::map<std::string, MapTeleporterElement> Teleporter;
    bool PhysicsStopped;
    bool BlockchangeStopped;
    short RankBuild;
    short RankShow;
    short RankJoin;
    std::string MotdOverride;
    bool loading;
    bool loaded;
    int LastClient;
    int Clients;
    // -- CPE..
    int SkyColor;
    int CloudColor;
    int FogColor;
    int alight;
    int dlight;
    bool ColorsSet;

    // -- EnvMapAppearance
    bool CustomAppearance;
    std::string CustomURL;
    char SideBlock;
    char EdgeBlock;
    short SideLevel;
    // -- Hack Control
    bool Flying;
    bool NoClip;
    bool Speeding;
    bool SpawnControl;
    bool ThirdPerson;
    bool Weather;
    short JumpHeight;
};
const std::string MAP_LIST_FILE = "Map_List";
const std::string MAP_SETTINGS_FILE = "Map_Settings";
// -- Names of the files within a map directory
const std::string MAP_FILENAME_DATA = "Data-Layer.gz";
const std::string MAP_FILENAME_RANK = "Rank-Layer.txt";
const std::string MAP_FILENAME_Overview = "Overview.png";
const std::string MAP_FILENAME_CONFIG = "config.txt";
const std::string MAP_FILENAME_TELEPORTER = "Teleporter.txt";
const int MAP_FILE_VERSION = 1050;
const int MAP_BLOCK_ELEMENT_SIZE = 4;

class Map {
public:
    MapData data;
};

class MapMain : TaskItem {
public:
    MapMain();
    shared_ptr<Map> GetPointer(int id);
    int GetMapId();
    std::string GetUniqueId();
    static MapMain* GetInstance();
    static int GetMapSize(int x, int y, int z, int blockSize) { return (x * y * z) * blockSize; }
    static int GetMapOffset(int x, int y, int z, int sizeX, int sizeY, int sizeZ, int blockSize) { return (x + y * sizeX + z * sizeX * sizeY) * blockSize;}
    void MainFunc();
private:
    static MapMain* Instance;
    std::thread BlockchangeThread;
    std::thread PhysicsThread;
    std::thread ActionThread;
    bool SaveFile;
    int SaveFileTimer;
    std::string TempFilename;
    int TempId;
    std::string TempOverviewFilename;
    int LastWriteTime;
    int StatsTimer;
    std::map<int, shared_ptr<Map>> _maps;
    // --
    int mapSettingsLastWriteTime;
    int mapSettingsTimerFileCheck;
    int mapSettingsMaxChangesSec;


    void MapListSave();
};
#endif //D3PP_MAP_H

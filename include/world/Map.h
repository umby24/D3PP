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
#include <queue>
#include <map>
#include <thread>
#include <memory>
#include <filesystem>

#include "common/TaskScheduler.h"
#include "common/MinecraftLocation.h"

class IMinecraftClient;
class Entity;

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
    unsigned short X;
    unsigned short Y;
    unsigned short Z;
    std::string ArgumentString;
};

struct MapBlockData {
    unsigned char type;
    unsigned char metadata;
    short lastPlayer;
};

struct MapBlockDo { // -- Physics Queue Item
    std::chrono::time_point<std::chrono::steady_clock> time;
    unsigned short X;
    unsigned short Y;
    unsigned short Z;
};

struct MapBlockChanged {
    unsigned short X;
    unsigned short Y;
    unsigned short Z;
    unsigned char Priority;
    short OldMaterial;

    bool operator()(const MapBlockChanged &a, const MapBlockChanged &b)
    {
        return a.Priority < b.Priority;
    }
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
    time_t SaveTime;
    std::string Directory;
    OverviewType overviewType;
    unsigned short SizeX;
    unsigned short SizeY;
    unsigned short SizeZ;
    std::vector<int> blockCounter;
    float SpawnX;
    float SpawnY;
    float SpawnZ;
    float SpawnRot;
    float SpawnLook;
    std::vector<unsigned char> Data;
    std::vector<unsigned char> PhysicData;
    std::vector<unsigned char> BlockchangeData;

    std::mutex physicsQueueMutex;
    std::mutex bcMutex;

    std::vector<MapBlockDo> PhysicsQueue;
    std::priority_queue<MapBlockChanged, std::vector<MapBlockChanged>, MapBlockChanged> ChangeQueue;
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
    time_t LastClient;
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
const std::string MAP_HTML_FILE = "Map_HTML";

// -- Names of the files within a map directory
const std::string MAP_FILENAME_DATA = "Data-Layer.gz";
const std::string MAP_FILENAME_RANK = "Rank-Layer.txt";
const std::string MAP_FILENAME_Overview = "Overview.png";
const std::string MAP_FILENAME_CONFIG = "Config.txt";
const std::string MAP_FILENAME_TELEPORTER = "Teleporter.txt";
const int MAP_FILE_VERSION = 1050;
const int MAP_BLOCK_ELEMENT_SIZE = 4;

class Map {
public:
    Map();
    MapData data;
    bool Resize(short x, short y, short z);
    void Fill(const std::string& functionName, std::string paramString);
    void BlockMove(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1, unsigned short Z1, bool undo, bool physic, unsigned char priority);
    void BlockChange(const std::shared_ptr<IMinecraftClient>& client, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char type);
    void BlockChange (short playerNumber, unsigned short X, unsigned short Y, unsigned short Z, unsigned char type, bool undo, bool physic, bool send, unsigned char priority);
    void ProcessPhysics(unsigned short X, unsigned short Y, unsigned short Z);
    bool Save(std::string directory);
    void Load(std::string directory);
    void LoadTeleporterFile(std::string directory);
    void LoadRankBoxFile(std::string directory);
    void LoadConfigFile(std::string directory);
    void SaveTeleporterFile(const std::string& directory);
    void SaveRankBoxFile(const std::string& directory);
    void SaveConfigFile(const std::string& directory);
    unsigned char GetBlockType(unsigned short X, unsigned short Y, unsigned short Z);
    unsigned short GetBlockPlayer(unsigned short X, unsigned short Y, unsigned short Z);
    MapBlockData GetBlockData(Vector3S location);
    void SetBlockData(Vector3S location, MapBlockData mbData);
    int BlockGetRank(unsigned short X, unsigned short Y, unsigned short Z);
    void SetRankBox(unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1, unsigned short Y1, unsigned short Z1, short rank);
    void Reload();
    void Unload();
    void Send(int clientId);
    void Resend();
    void AddTeleporter(std::string id, MinecraftLocation start, MinecraftLocation end, MinecraftLocation destination, std::string destMapUniqueId, int destMapId);
    void DeleteTeleporter(std::string id);
    void MapExport(MinecraftLocation start, MinecraftLocation end, std::string filename);
    void MapImport(std::string filename, MinecraftLocation location, short scaleX, short scaleY, short scaleZ);
    bool BlockInBounds(unsigned short X, unsigned short Y, unsigned short Z);
    void SetEnvColors(int red, int green, int blue, int type);
    void SetMapAppearance(std::string url, int sideblock, int edgeblock, int sidelevel);
    void SetHackControl(bool canFly, bool noclip, bool speeding, bool spawnControl, bool thirdperson, int jumpHeight);

    std::vector<int> GetEntities();
    void RemoveEntity(std::shared_ptr<Entity> e);
    void AddEntity(std::shared_ptr<Entity> e);
    std::mutex BlockChangeMutex;
protected:

private:


    void QueueBlockPhysics(unsigned short X, unsigned short Y, unsigned short Z);
    void QueueBlockChange(unsigned short X, unsigned short Y, unsigned short Z, unsigned char priority, unsigned char oldType);
};

class MapMain : TaskItem {
public:
    MapMain();
    std::shared_ptr<Map> GetPointer(int id);
    std::shared_ptr<Map> GetPointer(std::string name);
    std::shared_ptr<Map> GetPointerUniqueId(std::string uniqueId);
    int GetMapId();
    static std::string GetUniqueId();
    
    int Add(int id, short x, short y, short z, const std::string& name);
    void Delete(int id);
    static MapMain* GetInstance();
    static std::string GetMapMOTDOverride(int mapId);
    static int GetMapSize(int x, int y, int z, int blockSize) { return (x * y * z) * blockSize; }
    static int GetMapOffset(int x, int y, int z, int sizeX, int sizeY, int sizeZ, int blockSize) { return (x + y * sizeX + z * sizeX * sizeY) * blockSize;}
    static Vector3S GetMapExportSize(const std::string& filename);
    void MainFunc();

    void AddSaveAction(int clientId, int mapId, const std::string& directory);
    void AddLoadAction(int clientId, int mapId, const std::string& directory);
    void AddResizeAction(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z);
    void AddFillAction(int clientId, int mapId, std::string functionName, std::string argString);
    void AddDeleteAction(int clientId, int mapId);

    void ActionProcessor();
    bool SaveFile;
    std::map<int, std::shared_ptr<Map>> _maps;
private:
    static MapMain* Instance;
    std::thread BlockchangeThread;
    std::thread PhysicsThread;
    std::thread ActionThread;
    bool mbcStarted;
    bool maStarted;
    bool phStarted;

    time_t SaveFileTimer;
    std::string TempFilename;
    int TempId;
    std::string TempOverviewFilename;
    long LastWriteTime;
    time_t StatsTimer;

    std::vector<MapActionItem> _mapActions;

    // --
    long mapSettingsLastWriteTime;
    int mapSettingsTimerFileCheck;
    int mapSettingsMaxChangesSec;

    int GetMaxActionId();
    void AddAction(int clientId, int mapId, const MapActionItem &action);
    void HtmlStats(time_t time_);

    void MapListSave();
    void MapListLoad();

    void MapSettingsSave();
    void MapSettingsLoad();

    void MapBlockChange();
    void MapBlockPhysics();
};

const std::string MAP_HTML_TEMPLATE = R"(<html>
  <head>
    <title>Minecraft-Server Map</title>
  </head>
  <body>
      <b><u>Maps:</u></b><br>
      <br>
      <table border=1>        <tr>
          <th><b>ID</b></th>
          <th><b>U-ID</b></th>
          <th><b>Name</b></th>
          <th><b>Directory</b></th>
          <th><b>Save Intervall</b></th>
          <th><b>Ranks (B,J,S)</b></th>
          <th><b>Size (X,Y,Z)</b></th>
          <th><b>Memory</b></th>
          <th><b>Physic_Queue</b></th>
          <th><b>Send_Queue</b></th>
          <th><b>Physics</b></th>
          <th><b>Blockchange</b></th>
        </tr>
        [MAP_TABLE]
      </table>      <br>
      <br>
      <br>
    Site generated in [GEN_TIME] ms. [GEN_TIMESTAMP]<br>
  </body>
</html>)";

#endif //D3PP_MAP_H

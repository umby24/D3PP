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
#include <filesystem>

#include "TaskScheduler.h"

class NetworkClient;
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
    short lastPlayer;
    unsigned char metadata;
};

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
    void Fill(std::string functionName, std::string paramString);
    void BlockChange(std::shared_ptr<NetworkClient> client, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char type);
    void BlockChange (short playerNumber, unsigned short X, unsigned short Y, unsigned short Z, unsigned char type, bool undo, bool physic, bool send, unsigned char priority);
    bool Save(std::string directory);
    void Load(std::string directory);
    unsigned char GetBlockType(unsigned short X, unsigned short Y, unsigned short Z);
    void Reload();
    void Unload();
    void Send(int clientId);
    void Resend();
private:
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
    
    int Add(int id, short x, short y, short z, std::string name);
    void Delete(int id);
    static MapMain* GetInstance();
    std::string GetMapMOTDOverride(int mapId);
    static int GetMapSize(int x, int y, int z, int blockSize) { return (x * y * z) * blockSize; }
    static int GetMapOffset(int x, int y, int z, int sizeX, int sizeY, int sizeZ, int blockSize) { return (x + y * sizeX + z * sizeX * sizeY) * blockSize;}
    void MainFunc();
    
    void AddSaveAction(int clientId, int mapId, std::string directory);
    void AddLoadAction(int clientId, int mapId, std::string directory);
    void AddResizeAction(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z);
    void AddFillAction(int clientId, int mapId, std::string functionName, std::string argString);
    void AddDeleteAction(int clientId, int mapId);

    void ActionProcessor();
    bool SaveFile;
private:
    static MapMain* Instance;
    std::thread BlockchangeThread;
    std::thread PhysicsThread;
    std::thread ActionThread;
    bool mbcStarted;
    bool maStarted;
    
    int SaveFileTimer;
    std::string TempFilename;
    int TempId;
    std::string TempOverviewFilename;
    int LastWriteTime;
    int StatsTimer;
    time_t LastMapSettingsTime;
    std::vector<MapActionItem> _mapActions;
    std::map<int, std::shared_ptr<Map>> _maps;
    // --
    int mapSettingsLastWriteTime;
    int mapSettingsTimerFileCheck;
    int mapSettingsMaxChangesSec;

    int GetMaxActionId();
    void HtmlStats(time_t time_);
    void MapListSave();
    void MapListLoad();
    void MapSettingsSave(); // -- Where are these called from? 
    void MapSettingsLoad();
    void MapBlockchange();
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

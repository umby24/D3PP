//
// Created by unknown on 4/2/21.
//

#include "Map.h"

const std::string MODULE_NAME = "Map";

MapMain::MapMain() {
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);
    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

void MapMain::MainFunc() {
    Files* f = Files::GetInstance();
    std::string mapListFile = f->GetFile(MAP_LIST_FILE);
    long fileTime = Utils::FileModTime(mapListFile);
    if (LastWriteTime != fileTime) {
        // -- MapListLoad
    }
    if (SaveFile && SaveFileTimer < time(nullptr)) {
        SaveFileTimer = time(nullptr) + 5000;
        SaveFile = false;
        // -- MapListSave
    }
    for(auto const &m : _maps) {

        if (m.second->data.SaveInterval > 0 && m.second->data.SaveTime + m.second->data.SaveInterval*60000 < time(nullptr) && m.second->data.loaded) {
            m.second->data.SaveTime = time(nullptr);
            // -- MapActionAddSave
        }
        if (m.second->data.Clients > 0) {
            m.second->data.LastClient = time(nullptr);
            if (!m.second->data.loaded) {
                // -- reload
            }
        }
        if (m.second->data.loaded && (time(nullptr) - m.second->data.LastClient) > 20000) {
            // -- mapunload
        }
    }
    fileTime = Utils::FileModTime(f->GetFile(MAP_SETTINGS_FILE));
    if (fileTime != mapSettingsLastWriteTime) {
        // -- mapSettingsLoad
    }
    if (StatsTimer < time(nullptr)) {
        StatsTimer = time(nullptr) + 5000;
        // -- mapHtmlStats()
    }
}

MapMain* MapMain::GetInstance() {
    if (Instance == nullptr)
        Instance = new MapMain();

    return Instance;
}

shared_ptr<Map> MapMain::GetPointer(int id) {
    if (_maps.find(id) != _maps.end())
        return _maps[id];

    return nullptr;
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

void MapMain::MapListSave() {
    Files* f = Files::GetInstance();
    std::string fName = f->GetFile(MAP_LIST_FILE);
    ofstream oStream(fName);

    for(auto const &m : _maps) {
        oStream << "[" << m.first << "]" << endl;
        oStream << "Name = " << m.second->data.Name << endl;
        oStream << "Directory = " << m.second->data.Directory << endl;
        oStream << "Delete = 0" << endl;
        oStream << "Reload = 0" << endl;
    }
    oStream.close();
    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File saved. [" + fName + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

int MapMain::Add(int id, short x, short y, short z, std::string name) {
    if (id == -1)
        id = GetMapId();

    if (name.empty())
        return -1;

    if (GetPointer(id) != nullptr)
        return -1;
    Files *f = Files::GetInstance();

    shared_ptr<Map> newMap = std::make_shared<Map>();
    int mapSize = GetMapSize(x, y, z, MAP_BLOCK_ELEMENT_SIZE);
    newMap->data.Data = Mem::Allocate(mapSize, __FILE__, __LINE__, "Map_ID = " + stringulate(id));
    newMap->data.BlockchangeData = Mem::Allocate(1+mapSize/8, __FILE__, __LINE__, "Map_ID(Blockchange) = " + stringulate(id));
    newMap->data.PhysicData = Mem::Allocate(1+mapSize/8, __FILE__, __LINE__, "Map_ID(Physics) = " + stringulate(id));
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
    newMap->data.Clients = 0;
    newMap->data.LastClient = time(nullptr);

    for(auto i = 1; i < 255; i++)
        newMap->data.blockCounter[i] = 0;

    newMap->data.blockCounter[0] = x*y*z;
    _maps.insert(std::make_pair(id, newMap));
    SaveFile = true;
    return id;
}

void MapMain::Delete(int id) {
    shared_ptr<Map> mp = GetPointer(id);
    if (mp == nullptr)
        return;

    Mem::Free(mp->data.Data);
    Mem::Free(mp->data.BlockchangeData);
    Mem::Free(mp->data.PhysicData);
    _maps.erase(mp->data.ID);
    SaveFile = true;
}

bool Map::Resize(short x, short y, short z) {
    if (!data.loaded) {
        // -- MapReload
    }
    bool result = false;
    if (x >= 16 && y >= 16 && z >= 16 && x <= 32767 && y <= 32767 && z <= 32767) {
        int newMapSize = MapMain::GetMapSize(x, y, z, MAP_BLOCK_ELEMENT_SIZE);
        char* newMapData = Mem::Allocate(newMapSize, __FILE__, __LINE__, "Map_ID = " + stringulate(data.ID));
        char* newBCData = Mem::Allocate(1+newMapSize/8, __FILE__, __LINE__, "Map_ID(Blockchange) = " + stringulate(data.ID));
        char* newPhysData = Mem::Allocate(1+newMapSize/8, __FILE__, __LINE__, "Map_ID(Physics) = " + stringulate(data.ID));

        int copyAreaX = data.SizeX;
        int copyAreaY = data.SizeY;
        int copyAreaZ = data.SizeZ;
        if (copyAreaX > x)
            copyAreaX = x;
        if (copyAreaY > y)
            copyAreaY = y;
        if (copyAreaZ > z)
            copyAreaZ = z;

        for(auto i = 1; i < 255; i++)
            data.blockCounter[i] = 0;

        data.blockCounter[0] = x*y*z - (copyAreaX * copyAreaY * copyAreaZ);
        for (auto ix = 0; ix < copyAreaX; ix++) {
            for(auto iy = 0; iy < copyAreaY; iy++) {
                for (auto iz = 0; iz < copyAreaZ; iz++) {
                    char* pointerOld = data.Data + MapMain::GetMapOffset(ix, iy, iz, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
                    char* pointerNew = newMapData + MapMain::GetMapOffset(ix, iy, iz, x, y, z, MAP_BLOCK_ELEMENT_SIZE);
                    data.blockCounter[(int)*pointerOld] ++;
                    memcpy(pointerNew, pointerOld, MAP_BLOCK_ELEMENT_SIZE);
                }
            }
        }
        // -- Copy physics queue
        for(auto const &pi : data.PhysicsQueue) {
            if (pi.X < copyAreaX && pi.Y < copyAreaY && pi.Z < copyAreaZ) {
                int boolOffset = MapMain::GetMapOffset(pi.X, pi.Y, pi.Z, x, y, z, 1);
                char* boolPointer = newPhysData + (boolOffset / 8);
                newPhysData[(boolOffset / 8)] = newPhysData[(boolOffset / 8)] | (1 << (boolOffset % 8));
            }
        }
        data.PhysicsQueue.clear();
        for (auto const bc : data.ChangeQueue) {
            if (bc.X < copyAreaX && bc.Y < copyAreaY && bc.Z < copyAreaZ) {
                int boolOffset = MapMain::GetMapOffset(bc.X, bc.Y, bc.Z, x, y, z, 1);
                char* boolPointer = newPhysData + (boolOffset / 8);
                newBCData[(boolOffset / 8)] = newPhysData[(boolOffset / 8)] | (1 << (boolOffset % 8));
            }
        }
        data.ChangeQueue.clear();
        // -- Spawn limiting..
        if (data.SpawnX > x)
            data.SpawnX = x-1;
        if (data.SpawnY > y)
            data.SpawnY = y - 1;
        result = true;
        data.SizeX = x;
        data.SizeY = y;
        data.SizeZ = z;
        Mem::Free(data.Data);
        Mem::Free(data.PhysicData);
        Mem::Free(data.BlockchangeData);
        data.Data = newMapData;
        data.BlockchangeData = newBCData;
        data.PhysicData = newPhysData;
        // -- TODO: mapResend()
        MapMain* mm = MapMain::GetInstance();
        mm->SaveFile = true;
    }
    return result;
}

bool Map::Save(std::string directory) {
    int mapDataSize = MapMain::GetMapSize(data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
    if (directory.empty())
        directory = data.Directory;

    std::filesystem::create_directory(directory);
    std::string configName = directory + MAP_FILENAME_CONFIG;
    ofstream oStream(configName);
    if (oStream.is_open()) {
        oStream << "; Overview_Types: 0=Nothing, 1=2D, 2=Iso(fast)" << endl;
        oStream << "; Save_Intervall: in minutes (0 = Disabled)" << endl;
        oStream << "; Jumpheight: -1 = Default" << endl;
        oStream << ";" << endl;
        oStream << "Server_Version = 1018" << endl;
        oStream << "Unique_ID = " << data.UniqueID << endl;
        oStream << "Name = " + data.Name << endl;
        oStream << "Rank_Build = " << data.RankBuild << endl;
        oStream << "Rank_Join = " << data.RankJoin << endl;
        oStream << "Rank_Show = " << data.RankShow << endl;
        oStream << "Physic_Stopped = " << data.PhysicsStopped << endl;
        oStream << "MOTD_Override = " << data.MotdOverride << endl;
        oStream << "Save_Intervall = " << data.SaveInterval << endl;
        oStream << "Overview_Type = " << data.overviewType << endl;
        oStream << "Size_X = " << data.SizeX << endl;
        oStream << "Size_Y = " << data.SizeY << endl;
        oStream << "Size_Z = " << data.SizeZ << endl;
        oStream << "Spawn_X = " << data.SpawnX << endl;
        oStream << "Spawn_Y = " << data.SpawnY << endl;
        oStream << "Spawn_Z = " << data.SpawnZ << endl;
        oStream << "Spawn_Rot = " << data.SpawnRot << endl;
        oStream << "Spawn_Look = " << data.SpawnLook << endl;
        oStream << "Colors_Set = " << data.ColorsSet << endl;
        oStream << "Sky_Color = " << data.SkyColor << endl;
        oStream << "Cloud_Color = " << data.CloudColor << endl;
        oStream << "Fog_Color = " << data.FogColor << endl;
        oStream << "A_Light = " << data.alight << endl;
        oStream << "D_Light = " << data.dlight << endl;
        oStream << "Custom_Appearance = " << data.CustomAppearance << endl;
        oStream << "Custom_Texture_Url = " << data.CustomURL << endl;
        oStream << "Custom_Side_Block = " << data.SideBlock << endl;
        oStream << "Custom_Edge_Block = " << data.EdgeBlock << endl;
        oStream << "Custom_Side_Level = " << data.SideLevel << endl;
        oStream << "Allow_Flying = " << data.Flying << endl;
        oStream << "Allow_Noclip = " << data.NoClip << endl;
        oStream << "Allow_Fastwalk = " << data.Speeding << endl;
        oStream << "Allow_Respawn = " << data.SpawnControl << endl;
        oStream << "Allow_Thirdperson = " << data.ThirdPerson << endl;
        oStream << "Allow_Weatherchange = " << data.Weather << endl;
        oStream << "Jumpheight = " << data.JumpHeight << endl;
        oStream.close();
    }
    configName = directory + MAP_FILENAME_RANK;
    configName = directory + MAP_FILENAME_TELEPORTER;
    GZIP::GZip_CompressToFile((unsigned char*)data.Data, mapDataSize, "temp.gz");
    std::filesystem::copy_file("temp.gz", directory + MAP_FILENAME_DATA);
    Logger::LogAdd(MODULE_NAME, "File saved [" + directory + MAP_FILENAME_DATA + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);

    return true;
}

void Map::Load(std::string directory) {
    PreferenceLoader pLoader(MAP_FILENAME_CONFIG, directory);
    std::swap(_configFile, pLoader);
    pLoader.LoadFile();
    data.SizeX = pLoader.Read("Size_X", 0);
    data.SizeY = pLoader.Read("Size_Y", 0);
    data.SizeZ = pLoader.Read("Size_Z", 0);

    data.UniqueID = pLoader.Read("Unique_ID", MapMain::GetUniqueId());
    data.RankBuild = pLoader.Read("Rank_Build", 0);
    data.RankBuild = pLoader.Read("Rank_Join", 0);
    data.RankBuild = pLoader.Read("Rank_Show", 0);
    data.RankBuild = pLoader.Read("Physic_Stopped", 0);
    data.MotdOverride = pLoader.Read("MOTD_Override", "");
    data.SaveInterval = pLoader.Read("Save_Intervall", 10);
    data.overviewType = static_cast<OverviewType>(pLoader.Read("Overview_Type", 2));
    data.SpawnX = stof(pLoader.Read("Spawn_X", "1"));
    data.SpawnY = stof(pLoader.Read("Spawn_Y", "1"));
    data.SpawnZ = stof(pLoader.Read("Spawn_Z", "0"));
    data.SpawnRot = stof(pLoader.Read("Spawn_Rot", "0"));
    data.SpawnLook = stof(pLoader.Read("Spawn_Look", "0"));
}

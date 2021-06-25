//
// Created by unknown on 4/2/21.
//

#include "Map.h"

using namespace std;

const std::string MODULE_NAME = "Map";
MapMain* MapMain::Instance = nullptr;

MapMain::MapMain() {
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);
    TaskScheduler::RegisterTask(MODULE_NAME, *this);
    mbcStarted = false;
}

void MapMain::MainFunc() {
    Files* f = Files::GetInstance();
    std::string mapListFile = f->GetFile(MAP_LIST_FILE);
    long fileTime = Utils::FileModTime(mapListFile);
    
    if (System::IsRunning && mbcStarted == false) {
        std::thread mbcThread(MapMain::MapBlockchange, this);
        std::swap(BlockchangeThread, mbcThread);
        mbcStarted = true;
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

        if (m.second->data.SaveInterval > 0 && m.second->data.SaveTime + m.second->data.SaveInterval*600 < time(nullptr) && m.second->data.loaded) {
            m.second->data.SaveTime = time(nullptr);
            // -- MapActionAddSave
        }
        if (m.second->data.Clients > 0) {
            m.second->data.LastClient = time(nullptr);
            if (!m.second->data.loaded) {
                m.second->Reload();
            }
        }
        if (m.second->data.loaded && (time(nullptr) - m.second->data.LastClient) > 200) { // -- 3 minutes
            m.second->Unload();
        }
    }
    fileTime = Utils::FileModTime(f->GetFile(MAP_SETTINGS_FILE));
    if (fileTime != mapSettingsLastWriteTime) {
        MapSettingsLoad();
    }
    if (StatsTimer < time(nullptr)) {
        StatsTimer = time(nullptr) + 5;
        HtmlStats(time(nullptr));
    }
}

MapMain* MapMain::GetInstance() {
    if (Instance == nullptr)
        Instance = new MapMain();

    return Instance;
}

void MapMain::MapBlockchange() {
    clock_t blockChangeTimer = clock();

    while (System::IsRunning) {
        watchdog::Watch("Map_Blockchanging", "Begin thread-slope", 0);
        while (blockChangeTimer < clock()) {
            blockChangeTimer += 100;
            for(auto const &m : _maps) {
                if (m.second->data.BlockchangeStopped)
                    continue;
                
                int maxChangedSec = 1100 / 10;
                int toRemove = 0;
                for(auto const &chg : m.second->data.ChangeQueue) {
                    unsigned short x = chg.X;
                    unsigned short y = chg.Y;
                    unsigned short z = chg.Z;
                    short oldMat = chg.OldMaterial;
                    char priority = chg.Priority;
                    char currentMat = m.second->GetBlockType(x, y, z);
                    toRemove++;
                    int blockChangeOffset = GetMapOffset(x, y, z, m.second->data.SizeX, m.second->data.SizeY, m.second->data.SizeZ, 1);
                    int bitmaskSize = GetMapSize(m.second->data.SizeX, m.second->data.SizeY, m.second->data.SizeZ, 1);
                    
                    if (blockChangeOffset < bitmaskSize) { // -- remove from bitmask
                        m.second->data.BlockchangeData[blockChangeOffset/8] = m.second->data.BlockchangeData[blockChangeOffset/8] & ~(1 << (blockChangeOffset % 8));
                    }
                    if (currentMat != oldMat) {
                        NetworkFunctions::NetworkOutBlockSet2Map(m.first, x, y, z, currentMat);
                        maxChangedSec--;
                        if (maxChangedSec <= 0) {
                            break;
                        }
                    }
                }

                m.second->data.ChangeQueue.erase(m.second->data.ChangeQueue.begin(), m.second->data.ChangeQueue.begin() + toRemove);
            }
        }
        watchdog::Watch("Map_Blockchanging", "Begin thread-slope", 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

shared_ptr<Map> MapMain::GetPointer(int id) {
    if (_maps.find(id) != _maps.end())
        return _maps[id];

    return nullptr;
}

shared_ptr<Map> MapMain::GetPointer(std::string name) {
    shared_ptr<Map> result = nullptr;
    for (auto const &mi : _maps) {
        if (Utils::InsensitiveCompare(mi.second->data.Name, name)) {
            result = mi.second;
            break;
        }
    }
    
    return result;
}

std::shared_ptr<Map> MapMain::GetPointerUniqueId(std::string uniqueId) {
    shared_ptr<Map> result = nullptr;
    for (auto const &mi : _maps) {
        if (Utils::InsensitiveCompare(mi.second->data.UniqueID, uniqueId)) {
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

std::string MapMain::GetUniqueId() {
    std::string result;

    for(auto i = 1; i < 16; i++)
        result += char(65 + Utils::RandomNumber(25));

    return result;
}

std::string MapMain::GetMapMOTDOverride(int mapId) {
    std::string result = "";
    shared_ptr<Map> mapPtr = GetPointer(mapId);
    if (mapPtr == nullptr)
        return result;
    
    result = mapPtr->data.MotdOverride;
    return result;
}

void MapMain::HtmlStats(time_t time_) {
    time_t startTime = time(nullptr);
    std::string result = MAP_HTML_TEMPLATE;

    // -- Map Table Generation
    std::string mapTable;
    for (auto const &m : _maps) {
        mapTable += "<tr>\n";
        mapTable += "<td>" + stringulate(m.first) + "</td>\n";
        mapTable += "<td>" + m.second->data.UniqueID + "</td>\n";
        mapTable += "<td>" + m.second->data.Name + "</td>\n";
        mapTable += "<td>" + m.second->data.Directory + "</td>\n";
        mapTable += "<td>" + stringulate(m.second->data.SaveInterval) + "min</td>\n";
        mapTable += "<td>" + stringulate(m.second->data.RankBuild) + "," + stringulate(m.second->data.RankJoin) + "," + stringulate(m.second->data.RankShow) + "</td>\n";
        mapTable += "<td>" + stringulate(m.second->data.SizeX) + "x" + stringulate(m.second->data.SizeY) + "x" + stringulate(m.second->data.SizeZ) + "</td>\n";
        if (m.second->data.loaded)
            mapTable += "<td><i>See Memory.html</i></td>\n";
        else
            mapTable += "<td>0 MB (unloaded)</td>\n";

        mapTable += "<td>" + stringulate(m.second->data.PhysicsQueue.size()) + "</td>\n";
        mapTable += "<td>" + stringulate(m.second->data.ChangeQueue.size()) + "</td>\n";
        if (m.second->data.PhysicsStopped)
            mapTable += "<td><font color=\"#FF0000\">Stopped</font></td>\n";
        else
            mapTable += "<td><font color=\"#00FF00\">Started</font></td>\n";
        
        if (m.second->data.BlockchangeStopped)
            mapTable += "<td><font color=\"#FF0000\">Stopped</font></td>\n";
        else
            mapTable += "<td><font color=\"#00FF00\">Started</font></td>\n";

        mapTable += "</tr>\n";
    }
    Utils::replaceAll(result, "[MAP_TABLE]", mapTable);

    time_t finishTime = time(nullptr);
    long duration = finishTime - startTime;
    char buffer[255];
    strftime(buffer, sizeof(buffer), "%H:%M:%S  %m-%d-%Y", localtime(reinterpret_cast<const time_t *>(&finishTime)));
    std::string meh(buffer);
    Utils::replaceAll(result, "[GEN_TIME]", stringulate(duration));
    Utils::replaceAll(result, "[GEN_TIMESTAMP]", meh);

    Files* files = Files::GetInstance();
    std::string memFile = files->GetFile(MAP_HTML_FILE);

    ofstream oStream(memFile, std::ios::out | std::ios::trunc);
    if (oStream.is_open()) {
        oStream << result;
        oStream.close();
    } else {
        Logger::LogAdd(MODULE_NAME, "Couldn't open file :<" + memFile, LogType::WARNING, __FILE__, __LINE__, __FUNCTION__ );
    }
}

void MapMain::MapListSave() {
    Files* f = Files::GetInstance();
    std::string fName = f->GetFile(MAP_LIST_FILE);
    PreferenceLoader pl(fName, "");

    for(auto const &m : _maps) {
        pl.SelectGroup(stringulate(m.first));
        pl.Write("Name", m.second->data.Name);
        pl.Write("Directory", m.second->data.Directory);
        pl.Write("Delete", 0);
        pl.Write("Reload", 0);
    }
    pl.SaveFile();

    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File saved. [" + fName + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
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
            // -- MapActionAddDelete
        } else {
            shared_ptr<Map> mapPtr = GetPointer(mapId);
            if (mapPtr == nullptr) {
                Add(mapId, 64, 64, 64, mapName);
                mapReload = true;
                mapPtr = GetPointer(mapId);
            }
            mapPtr->data.Directory = directory;
            //mapPtr->Load("");
            if ((mapReload)) {
                mapPtr->Load("");
                // -- MapActionAddLoad
            }
        }
    }
    SaveFile = true;

    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File loaded. [" + fName + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
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
    newMap->data.BlockchangeStopped = false;
    newMap->data.PhysicsStopped = false;
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

void MapMain::MapSettingsLoad() {

}

bool Map::Resize(short x, short y, short z) {
    if (!data.loaded) {
        Reload();
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
        Resend();
        MapMain* mm = MapMain::GetInstance();
        mm->SaveFile = true;
    }
    return result;
}

bool Map::Save(std::string directory) {
    if (!this->data.loaded)
        return true;

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
    configName = directory + MAP_FILENAME_RANK; // -- TODO: Rank boxes
    configName = directory + MAP_FILENAME_TELEPORTER; // -- TODO: Teleporters
    GZIP::GZip_CompressToFile((unsigned char*)data.Data, mapDataSize, "temp.gz");
    try {
        if (std::filesystem::exists(directory + MAP_FILENAME_DATA))
            std::filesystem::remove(directory + MAP_FILENAME_DATA);

        std::filesystem::copy("temp.gz", directory + MAP_FILENAME_DATA, std::filesystem::copy_options::overwrite_existing);
    } catch (std::filesystem::filesystem_error& e) {
        Logger::LogAdd(MODULE_NAME, "Could not copy mapfile: " + stringulate(e.what()), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
    }
    Logger::LogAdd(MODULE_NAME, "File saved [" + directory + MAP_FILENAME_DATA + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);

    return true;
}

void Map::Load(std::string directory) {
    Block* blockMain = Block::GetInstance();

    if (directory.empty()) {
        directory = data.Directory;
    }

    PreferenceLoader pLoader(MAP_FILENAME_CONFIG, directory);
    pLoader.LoadFile();
    int sizeX = pLoader.Read("Size_X", 0);
    int sizeY = pLoader.Read("Size_Y", 0);
    int sizeZ = pLoader.Read("Size_Z", 0);
    int mapSize = sizeX * sizeY * sizeZ;

    Resize(sizeX, sizeY, sizeZ);
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

    int dSize = GZIP::GZip_DecompressFromFile(reinterpret_cast<unsigned char*>(data.Data), mapSize * MAP_BLOCK_ELEMENT_SIZE, directory + MAP_FILENAME_DATA);
    if (dSize == (mapSize * MAP_BLOCK_ELEMENT_SIZE)) {
        // -- TODO: Clear undo map
        for(int i = 0; i < 255; i++) {
            data.blockCounter[i] = 0;
        }
        for (int iz = 0; iz < data.SizeZ; iz++) {
            for (int iy = 0; iy < data.SizeY; iy++) {
                for (int ix = 0; ix < data.SizeX; ix++) {
                    int index = MapMain::GetMapOffset(ix, iy, iz, sizeX, sizeY, sizeZ, MAP_BLOCK_ELEMENT_SIZE);
                    unsigned char point = data.Data[index];
                    MapBlock b = blockMain->GetBlock(point);

                    if (b.ReplaceOnLoad >= 0)
                        data.Data[index] = static_cast<char>(b.ReplaceOnLoad);

                    data.blockCounter[point] += 1;
                    
                    if (b.PhysicsOnLoad) {
                        // -- map_block_do_add
                    }
                }
            }
        }

        Logger::LogAdd(MODULE_NAME, "Map Loaded [" + directory + MAP_FILENAME_DATA + "] (" + stringulate(sizeX) + "x" + stringulate(sizeY) + "x" + stringulate(sizeZ) + ")", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }

    // -- Load RankBox file
    PreferenceLoader rBoxLoader(MAP_FILENAME_RANK, directory);
    rBoxLoader.LoadFile();
    for (auto const &fi : rBoxLoader.SettingsDictionary) {
        rBoxLoader.SelectGroup(fi.first);
        MapRankElement mre;
        mre.X0 = rBoxLoader.Read("X_0", 0);
        mre.Y0 = rBoxLoader.Read("Y_0", 0);
        mre.Z0 = rBoxLoader.Read("Z_0", 0);
        mre.X1 = rBoxLoader.Read("X_1", 0);
        mre.Y1 = rBoxLoader.Read("Y_1", 0);
        mre.Z1 = rBoxLoader.Read("Z_1", 0);
        mre.Rank = rBoxLoader.Read("Rank", 0);
        data.RankBoxes.push_back(mre);
    }
    // -- Load Teleporter file
    PreferenceLoader tpLoader(MAP_FILENAME_TELEPORTER, directory);
    tpLoader.LoadFile();
    for (auto const &tp : tpLoader.SettingsDictionary) {
        tpLoader.SelectGroup(tp.first);
        MapTeleporterElement tpe;
        tpe.Id = tp.first;
        tpe.X0 = tpLoader.Read("X_0", 0);
        tpe.Y0 = tpLoader.Read("Y_0", 0);
        tpe.Z0 = tpLoader.Read("Z_0", 0);
        tpe.X1 = tpLoader.Read("X_1", 0);
        tpe.Y1 = tpLoader.Read("Y_1", 0);
        tpe.Z1 = tpLoader.Read("Z_1", 0);

        tpe.DestMapUniqueId = tpLoader.Read("Dest_Map_Unique_ID", "");
        tpe.DestMapId = tpLoader.Read("Dest_Map_ID", -1);
        tpe.DestX = tpLoader.Read("Dest_X", -1);
        tpe.DestY = tpLoader.Read("Dest_Y", -1);
        tpe.DestZ = tpLoader.Read("Dest_Z", -1);
        tpe.DestRot = tpLoader.Read("Dest_Rot", 0);
        tpe.DestLook = tpLoader.Read("Dest_Look", 0);
        data.Teleporter.insert(std::make_pair(tpe.Id, tpe));
    }
}

void Map::Reload() {
    if (data.loaded)
        return;
    Block* blockMain = Block::GetInstance();

    data.loading = true;
    int mapSize = data.SizeX * data.SizeY * data.SizeZ;
    data.Data = Mem::Allocate(mapSize * MAP_BLOCK_ELEMENT_SIZE, __FILE__, __LINE__, "Map_ID = " + stringulate(data.ID));
    data.BlockchangeData = Mem::Allocate(1+mapSize/8, __FILE__, __LINE__, "Map_ID(Blockchange) = " + stringulate(data.ID));
    data.PhysicData = Mem::Allocate(1+mapSize/8, __FILE__, __LINE__, "Map_ID(Physics) = " + stringulate(data.ID));

    std::string filenameData = data.Directory + MAP_FILENAME_DATA;
    int unzipResult = GZIP::GZip_DecompressFromFile(reinterpret_cast<unsigned char*>(data.Data), mapSize * MAP_BLOCK_ELEMENT_SIZE, data.Directory + MAP_FILENAME_DATA);
    if (unzipResult > 0) {
    // -- UNDO Clear Map
        for(int i = 0; i < 255; i++) {
            data.blockCounter[i] = 0;
        }
        for (int iz = 0; iz < data.SizeZ; iz++) {
            for (int iy = 0; iy < data.SizeY; iy++) {
                for (int ix = 0; ix < data.SizeX; ix++) {
                    int offset = MapMain::GetMapOffset(ix, iy, iz, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
                    unsigned char rawBlock = data.Data[offset];
                    MapBlock b = blockMain->GetBlock(rawBlock);

                    if (b.ReplaceOnLoad >= 0)
                        data.Data[offset] = static_cast<char>(b.ReplaceOnLoad);

                    data.blockCounter[rawBlock] += 1;

                    if (b.PhysicsOnLoad) {
                        
                        // -- map_block_do_add
                    }
                }
            }
        }

        data.loaded = true;
        data.loading = false;
        data.PhysicsStopped = false;
        data.BlockchangeStopped = false;
        Logger::LogAdd(MODULE_NAME, "Map Reloaded [" + filenameData + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
}

void Map::Unload() {
    if (!data.loaded)
        return;

    Save("");
    data.PhysicsStopped = true;
    data.BlockchangeStopped = true;
    Mem::Free(data.Data);
    Mem::Free(data.BlockchangeData);
    Mem::Free(data.PhysicData);
    data.loaded = false;
    Logger::LogAdd(MODULE_NAME, "Map unloaded (" + data.Name + ")", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Map::Send(int clientId) {
    Network* nMain = Network::GetInstance();
    Block* bMain = Block::GetInstance();
    shared_ptr<NetworkClient> nc = nMain->GetClient(clientId);

    if (nc == nullptr)
        return;

    if (!data.loaded)
        Reload();

    int mapSize = data.SizeX * data.SizeY * data.SizeZ;
    char* tempBuf = Mem::Allocate(mapSize + 10, __FILE__, __LINE__, "Temp");
    int tempBufferOffset = 0;

    tempBuf[tempBufferOffset++] = mapSize / 16777216;
    tempBuf[tempBufferOffset++] = mapSize / 65536;
    tempBuf[tempBufferOffset++] = mapSize / 256;
    tempBuf[tempBufferOffset++] = mapSize;
    std::vector<unsigned char> seenIds;
    std::vector<int> returnedIds;

    for (int i = 0; i < mapSize-1; i++) {
        int index = i * MAP_BLOCK_ELEMENT_SIZE;

        unsigned char rawBlock = static_cast<unsigned char>(data.Data[index]);
        MapBlock mb = bMain->GetBlock(rawBlock);

        if (mb.CpeLevel > nc->CustomBlocksLevel)
            tempBuf[tempBufferOffset++] = mb.CpeReplace;
        else     
            tempBuf[tempBufferOffset++] = mb.OnClient;
    }
    int tempBuffer2Size = GZIP::GZip_CompressBound(tempBufferOffset) + 1024 + 512;
    char* tempBuf2 = Mem::Allocate(tempBuffer2Size, __FILE__, __LINE__, "Temp");

    int compressedSize = GZIP::GZip_Compress(reinterpret_cast<unsigned char*>(tempBuf2), tempBuffer2Size, reinterpret_cast<unsigned char*>(tempBuf), tempBufferOffset);
    Mem::Free(tempBuf);
    if (compressedSize == -1) {
        Logger::LogAdd(MODULE_NAME, "Can't send the map: GZip Error", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        nc->Kick("Mapsend error", false);
        Mem::Free(tempBuf2);
        return;
    }

    compressedSize += (1024 - (compressedSize % 1024));
    Packets::SendMapInit(clientId);
    int bytes2Send = compressedSize;
    int bytesSent = 0;

    while (bytes2Send > 0) {
        int bytesInBlock = bytes2Send;
        if (bytesInBlock > 1024)
            bytesInBlock = 1024;
        Packets::SendMapData(clientId, bytesInBlock, tempBuf2 + bytesSent, bytesSent*100/compressedSize);
        bytesSent += bytesInBlock;
        bytes2Send -= bytesInBlock;
    }

    Packets::SendMapFinalize(clientId, data.SizeX, data.SizeY, data.SizeZ);
    // -- TODO: CPE::AfterMapActions();
    Mem::Free(tempBuf2);
}

void Map::Resend() {
    Network* nMain = Network::GetInstance();
    for(auto const &nc : nMain->_clients) {
        if (nc.second->player->MapId == data.ID)
            nc.second->player->MapId = -1;
    }
    for (auto const &me : Entity::_entities) {
        if (me.second->MapID == data.ID) {
            if (me.second->X > data.SizeX-0.5)
                me.second->X = data.SizeX-0.5;
            
            if (me.second->X < 0.5)
                me.second->X = 0.5;

            if (me.second->Y > data.SizeY-0.5)
                me.second->Y = data.SizeY-0.5;
            
            if (me.second->Y < 0.5)
                me.second->Y = 0.5;
        }
    }

    for(auto const &bc : data.ChangeQueue) {
        int blockCHangeOffset = MapMain::GetMapOffset(bc.X, bc.Y, bc.Z, data.SizeX, data.SizeY, data.SizeZ, 1);
        if (blockCHangeOffset < MapMain::GetMapSize(data.SizeX, data.SizeY, data.SizeZ, 1)) {
            data.BlockchangeData[blockCHangeOffset/8] = (data.BlockchangeData[blockCHangeOffset/8]&255) & ~(1 << (blockCHangeOffset % 8));
        }
    }
    data.ChangeQueue.clear();
}

void Map::BlockChange(std::shared_ptr<NetworkClient> client, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char type) {
    if (client == nullptr || client->player->tEntity == nullptr || client->player->tEntity->playerList == nullptr)
        return;

    if (client->player->tEntity->playerList->Stopped) {
        NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to build (stopped).");
        return;
    }
    
    Block* bm = Block::GetInstance();

    if (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ) {
        unsigned char rawBlock = static_cast<unsigned char>(data.Data[MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE)]);
        unsigned char rawNewType = 0;
        MapBlock oldType = bm->GetBlock(rawBlock);
        client->player->tEntity->lastMaterial = type;
        // -- Map_Block_Get_Rank (RBOX!!)
        // -- TODO:

        if (mode > 0)
            rawNewType = type;
        else
            rawNewType = oldType.AfterDelete;

        QueueBlockChange(X, Y, Z, 250, -1);

        MapBlock newType = bm->GetBlock(rawNewType);
        if (client->player->tEntity->playerList->PRank < oldType.RankDelete) {
            NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to delete this block type.");
            return;
        } else if (client->player->tEntity->playerList->PRank < newType.RankPlace) {
            NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to build this block type.");
            return;
        }
        BlockChange(client->player->tEntity->playerList->Number, X, Y, Z, rawNewType, true, true, true, 250);
        // -- PluginEventBlockCreate (one for delete, one for create.)

    }
}

void Map::BlockChange (short playerNumber, unsigned short X, unsigned short Y, unsigned short Z, unsigned char type, bool undo, bool physic, bool send, unsigned char priority) {
    if (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ) {
        Block* bm = Block::GetInstance();
        int blockOffset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
        MapBlockData* atLoc = reinterpret_cast<MapBlockData*>(data.Data + blockOffset);

        // -- Plugin Event: Block change
        MapBlock oldType = bm->GetBlock(atLoc->type);
        MapBlock newType = bm->GetBlock(type);
        
        if (type != atLoc->type && undo) {
            // -- TODO: Undo_Add()
        }

        data.blockCounter[atLoc->type]--;
        data.blockCounter[type]++;
        atLoc->type = type;
        atLoc->lastPlayer = playerNumber;

        if (physic) {
            // -- QueuePhysics
        }
        if (oldType.Id != newType.Id && send) {
            QueueBlockChange(X, Y, Z, priority, oldType.Id);
        }
    }
}

unsigned char Map::GetBlockType(unsigned short X, unsigned short Y, unsigned short Z) {
    // if (data.loaded = false && data.loading == false) {
    //     data.LastClient = time(nullptr);
    //     Reload();
    // }
    // if (data.loading) {
    //     while (data.loaded == false) {
    //         std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //     }
    // }
    if (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ) {
        int index = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, MAP_BLOCK_ELEMENT_SIZE);
        return data.Data[index];
    }

    return -1;
}

void Map::QueueBlockChange(unsigned short X, unsigned short Y, unsigned short Z, unsigned char priority, unsigned char oldType) {
    if (X >= 0 && X < data.SizeX && Y >= 0 && Y < data.SizeY && Z >= 0 && Z < data.SizeZ) {
        int offset = MapMain::GetMapOffset(X, Y, Z, data.SizeX, data.SizeY, data.SizeZ, 1);
        bool blockChangeFound = data.BlockchangeData[offset/8] & (1 << (offset % 8));
        if (!blockChangeFound) {
            data.BlockchangeData[offset/8] |= (1 << (offset % 8)); // -- Set bitmask
            MapBlockChanged changeItem { X, Y, Z, priority, oldType};
            int insertIndex = 0;
            
            for(int i = 0; i < data.ChangeQueue.size(); i++) {
                if (data.ChangeQueue[i].Priority >= priority) {
                    insertIndex++;
                    break;
                }
                insertIndex++;
            }
            // -- This is going to break. Probs need to back it up by 1.
            data.ChangeQueue.insert(data.ChangeQueue.begin() + insertIndex, changeItem);
        }
    }
}

Map::Map() = default;

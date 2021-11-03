//
// Created by unknown on 2/18/2021.
//

#include "Block.h"

#include "common/PreferenceLoader.h"
#include "Utils.h"
#include "common/Files.h"
#include "common/Logger.h"


Block* Block::Instance = nullptr;
const std::string Block::MODULE_NAME = "Block";

Block::Block() {
    SaveFile = false;
    hasLoaded = false;
    LastFileDate = 0;
    for(auto i = 0; i < 255; i++) { // -- Pre-pop..
        struct MapBlock shell{i};
        Blocks.push_back(shell);
    }

    this->Interval = std::chrono::seconds(2);
    this->Setup = [this] { Load(); };
    this->Main = [this] { MainFunc(); };
    this->Teardown = [this] {Save(); };

    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}


MapBlock Block::GetBlock(int id) {
    if (!hasLoaded)
        Load();

    if (id >= 0 && id < 255)
        return Blocks[id];

    MapBlock result {-1};
    return result;
}

MapBlock Block::GetBlock(std::string name) {
    if (!hasLoaded)
        Load();

    MapBlock result {-1};

    for(auto i = 0; i < 255; i++) {
        if (Utils::InsensitiveCompare(Blocks[i].Name, name)) {
            result = Blocks[i];
            break;
        }
    }
    
    return result;
}

void Block::LoadOld() {
    if (Utils::FileSize("Data/Block.txt") == -1)
        return;
    
    Logger::LogAdd(MODULE_NAME, "Importing old block file, Blocks.json will be overwritten.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    Blocks.clear();
    for (auto i = 0; i < 255; i++) { // -- Pre-pop..
        struct MapBlock shell { i };
        Blocks.push_back(shell);
    }
    PreferenceLoader pl("Block.txt", "Data/");
    pl.LoadFile();
    
    for (auto const &item : pl.SettingsDictionary) {
        if (item.first.empty())
            continue;

        pl.SelectGroup(item.first);
        struct MapBlock newItem;
        newItem.Id = stoi(item.first);
        Logger::LogAdd(MODULE_NAME, "Loading " + stringulate(item.first), LogType::DEBUG, GLF);
        newItem.Name = pl.Read("Name", "Unknown");
        newItem.OnClient = pl.Read("On_Client", 4);
        newItem.Physics = pl.Read("Physic", 0);
        newItem.PhysicsPlugin = pl.Read("Physics_Plugin", "");
        if (newItem.PhysicsPlugin.empty()) {
            newItem.PhysicsPlugin = pl.Read("Physic_Plugin", "");
        }
        newItem.PhysicsTime = pl.Read("Do_Time", 0);
        newItem.PhysicsRandom = pl.Read("Do_Time_Random", 0);
        newItem.PhysicsRepeat = (pl.Read("Do_Repeat", 0) == 1);
        newItem.PhysicsOnLoad = (pl.Read("Do_By_Load", 0) == 1);
        newItem.CreatePlugin = pl.Read("Create_Plugin", "");
        newItem.DeletePlugin = pl.Read("Delete_Plugin", "");
        newItem.RankPlace = pl.Read("Rank_Place", 0);
        newItem.RankDelete = pl.Read("Rank_Delete", 0);
        newItem.AfterDelete = pl.Read("After_Delete", 0);
        newItem.ReplaceOnLoad = pl.Read("Replace_By_Load", -1);
        newItem.Kills = (pl.Read("Killer", 0) == 1);
        newItem.Special = (pl.Read("Special", 0) == 1);
        newItem.OverviewColor = pl.Read("Color_Overview", 0);
        newItem.CpeLevel = pl.Read("CPE_Level", 0);
        newItem.CpeReplace = pl.Read("CPE_Replace", 0);
        if (newItem.Id <= 254)
            Blocks[newItem.Id] = newItem;
    }

    Logger::LogAdd(MODULE_NAME, stringulate(Blocks.size()) + " blocks imported.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    Save();
    std::filesystem::remove("Data/Block.txt");
}

bool compareBlockIds(const MapBlock &i1, const MapBlock &i2) {
    return (i1.Id < i2.Id);
}

void Block::Load() {
    if (Utils::FileSize("Data/Block.txt") != -1) {
        LoadOld();
        return;
    }
    
    Files* f = Files::GetInstance();
    std::string filePath = f->GetFile(BLOCK_FILE_NAME);
    json j;

    std::ifstream iStream(filePath);

    if (!iStream.is_open()) {
        Logger::LogAdd(MODULE_NAME, "Failed to load blocks!!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }
    try {
        iStream >> j;
    } catch (int exception) {
        Logger::LogAdd(MODULE_NAME, "Failed to load blocks! [" + stringulate(exception) + "]", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }
    iStream.close();

    for(auto &item : j) {
        struct MapBlock loadedItem {-1, };
        if (item["Id"].is_number()) {
            loadedItem.Id = item["Id"];
            loadedItem.Name = item["Name"];
            loadedItem.OnClient = item["OnClient"];
            loadedItem.Physics = item["Physics"];
            loadedItem.PhysicsPlugin = item["PhysicsPlugin"];
            loadedItem.PhysicsTime = item["PhysicsTime"];
            loadedItem.PhysicsRandom = item["PhysicsRandom"];
            loadedItem.PhysicsRepeat = item["PhysicsRepeat"];

            if (item["PhysicsOnLoad"].is_boolean())
                loadedItem.PhysicsOnLoad = item["PhysicsOnLoad"];
            
            if (!item["CreatePlugin"].is_null())
                loadedItem.CreatePlugin = item["CreatePlugin"];

            if (!item["DeletePlugin"].is_null())
                loadedItem.DeletePlugin = item["DeletePlugin"];

            if (!item["ReplaceOnLoad"].is_null())
                loadedItem.ReplaceOnLoad = item["ReplaceOnLoad"];
            else
                loadedItem.ReplaceOnLoad = -1;

            if (!item["RankPlace"].is_null())
                loadedItem.RankPlace = item["RankPlace"];

            if (!item["RankDelete"].is_null())
                loadedItem.RankDelete = item["RankDelete"];

            if (!item["AfterDelete"].is_null())
                loadedItem.AfterDelete = item["AfterDelete"];

            if (!item["Kills"].is_null())
                loadedItem.Kills = item["Kills"];

            if (!item["Special"].is_null())
                loadedItem.Special = item["Special"];

            if (!item["OverviewColor"].is_null())
                loadedItem.OverviewColor = item["OverviewColor"];

            if (item["CpeLevel"].is_number())
                loadedItem.CpeLevel = item["CpeLevel"];

            if (!item["CpeReplace"].is_null())
                loadedItem.CpeReplace = item["CpeReplace"];

            Blocks[loadedItem.Id] = loadedItem;
        }
    }

    Logger::LogAdd(MODULE_NAME, "File loaded.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__ );
    hasLoaded = true;
    time_t modTime = Utils::FileModTime(filePath);
    LastFileDate = modTime;
}

void Block::Save() {
    json j;
    Files* f = Files::GetInstance();
    std::string blockFile = f->GetFile(BLOCK_FILE_NAME);
    
    std::sort(Blocks.begin(), Blocks.end(), compareBlockIds);

    for(auto i = 0; i < 255; i++) {
        j[i] = nullptr;
        j[i] = {
                {"Id", i},
                {"Name", Blocks[i].Name},
                {"OnClient", Blocks[i].OnClient},
                {"Physics", Blocks[i].Physics},
                {"PhysicsPlugin", Blocks[i].PhysicsPlugin},
                {"PhysicsTime", Blocks[i].PhysicsTime},
                {"PhysicsRandom", Blocks[i].PhysicsRandom},
                {"PhysicsRepeat", Blocks[i].PhysicsRepeat},
                {"PhysicsOnLoad", Blocks[i].PhysicsOnLoad},
                {"CreatePlugin", Blocks[i].CreatePlugin},
                {"DeletePlugin", Blocks[i].DeletePlugin},
                {"ReplaceOnLoad", Blocks[i].ReplaceOnLoad},
                {"RankPlace", Blocks[i].RankPlace},
                {"RankDelete", Blocks[i].RankDelete},
                {"AfterDelete", Blocks[i].AfterDelete},
                {"Kills", Blocks[i].Kills},
                {"Special", Blocks[i].Special},
                {"OverviewColor", Blocks[i].OverviewColor},
                {"CpeLevel", Blocks[i].CpeLevel},
                {"CpeReplace", Blocks[i].CpeReplace},
        };
    }

    std::ofstream oStream(blockFile, std::ios::trunc);
    oStream << std::setw(4) << j;
    oStream.flush();
    oStream.close();

    time_t modTime = Utils::FileModTime(blockFile);
    LastFileDate = modTime;

    Logger::LogAdd(MODULE_NAME, "File saved [" + blockFile + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Block::MainFunc() {
    if (SaveFile) {
        SaveFile = false;
        Save();
    }

    Files* f = Files::GetInstance();
    std::string blockFile = f->GetFile(BLOCK_FILE_NAME);
    time_t modTime = Utils::FileModTime(blockFile);

    if (modTime != LastFileDate) {
        Load();
        LastFileDate = modTime;
    }
}

Block *Block::GetInstance() {
    if (Instance == nullptr)
        Instance = new Block();

    return Instance;
}

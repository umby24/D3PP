//
// Created by unknown on 2/18/2021.
//

#include "Block.h"
const std::string MODULE_NAME = "Block";
Block* Block::Instance = nullptr;

Block::Block() {
    for(auto i = 0; i < 255; i++) { // -- Pre-pop..
        struct MapBlock shell;
        shell.Id = i;
        Blocks.push_back(shell);
    }

    this->Interval = std::chrono::seconds(2);
    this->Setup = [this] { Load(); };
    this->Main = [this] { MainFunc(); };
    this->Teardown = [this] {Save(); };

    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}


MapBlock Block::GetBlock(int id) {
    if (id >= 0 && id <= 255)
        return Blocks[id];

    MapBlock result {-1};
    return result;
}

void Block::Load() {
    Files* f = Files::GetInstance();
    std::string filePath = f->GetFile(BLOCK_FILE_NAME);
    json j;

    ifstream iStream(filePath);

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
        struct MapBlock loadedItem;
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

    time_t modTime = Utils::FileModTime(filePath);
    LastFileDate = modTime;
}

void Block::Save() {
    json j;
    Files* f = Files::GetInstance();
    std::string blockFile = f->GetFile(BLOCK_FILE_NAME);

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

    ofstream oStream(blockFile, std::ios::trunc);
    oStream << std::setw(4) << j;
    oStream.flush();
    oStream.close();

    time_t modTime = Utils::FileModTime(blockFile);
    LastFileDate = modTime;

    Logger::LogAdd(MODULE_NAME, "File saved.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
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

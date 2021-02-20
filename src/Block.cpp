//
// Created by unknown on 2/18/2021.
//

#include "Block.h"

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

    TaskScheduler::RegisterTask("Block", *this);
}


MapBlock Block::GetBlock() {

}

void Block::Load() {
    Files* f = Files::GetInstance();
    std::string filePath = f->GetFile(BLOCK_FILE_NAME);
    json j;

    ifstream iStream(filePath);

    if (!iStream.is_open()) {
        Logger::LogAdd("Block", "Failed to load blocks!!", static_cast<LogType>(10), __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    iStream >> j;
    iStream.close();

    for(auto &item : j) {
        struct MapBlock loadedItem;
        loadedItem.Id = item["id"];
    }

    Logger::LogAdd("Block", "File loaded.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__ );

}

void Block::Save() {
    json j;
    Files* f = Files::GetInstance();
    std::string blockFile = f->GetFile(BLOCK_FILE_NAME);
    for(auto i = 0; i < 255; i++) {
        j[i] = nullptr;
        j[i] = {
                {"id", i},
                {"Name", Blocks[i].Name},
                {"OnClient", Blocks[i].OnClient},
                {"Physics", Blocks[i].Physics},
                {"PhysicsPlugin", Blocks[i].PhysicsPlugin},
                {"PhysicsTime", Blocks[i].PhysicsTime},
        };
    }
    ofstream oStream(blockFile);
    oStream << j;
    oStream.flush();
    oStream.close();
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
    }
}

void Block::Shutdown() {
    Save();
}


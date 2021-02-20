//
// Created by unknown on 2/18/2021.
//

#ifndef D3PP_BLOCK_H
#define D3PP_BLOCK_H
#include <string>
#include <vector>
#include "TaskScheduler.h"
#include "json.hpp"
#include "Utils.h"
#include "Files.h"

using json = nlohmann::json;

const std::string BLOCK_FILE_NAME = "Block";
enum PhysicsType {
    OFF = 0,
    ORIGINAL_SAND = 10,
    NEW_SAND = 11,
    INFINITE_WATER = 20,
    FINITE_WATER = 21
};

struct MapBlock {
    int Id;
    std::string Name;
    char OnClient;
    char Physics;
    std::string PhysicsPlugin;
    int PhysicsTime;
    int PhysicsRandom;
    bool PhysicsRepeat;
    bool PhysicsOnLoad;
    std::string CreatePlugin;
    std::string DeletePlugin;
    int ReplaceOnLoad;
    int RankPlace;
    int RankDelete;
    char AfterDelete;
    bool Kills;
    bool Special;
    int OverviewColor;
    char CpeLevel;
    char CpeReplace;
};

class Block : TaskItem {
public:
    Block();
    MapBlock GetBlock();
    void Save();
protected:
private:
    bool SaveFile;
    time_t LastFileDate;
    time_t FileCheckTimer;
    std::vector<MapBlock> Blocks;

    void Load();

    void MainFunc();
    void Shutdown();
};


#endif //D3PP_BLOCK_H

//
// Created by unknown on 2/18/2021.
//

#ifndef D3PP_BLOCK_H
#define D3PP_BLOCK_H
#include <string>
#include <vector>
#include <time.h>

#include "TaskScheduler.h"
#include "json.hpp"


using json = nlohmann::json;
class TaskItem;
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
    int OnClient;
    int Physics;
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
    int AfterDelete;
    bool Kills;
    bool Special;
    int OverviewColor;
    int CpeLevel;
    int CpeReplace;
};

class Block : TaskItem {
public:
    Block();
    MapBlock GetBlock(int id);
    MapBlock GetBlock(std::string name);
    void Save();
    static Block* GetInstance();
protected:
    static Block* Instance;
private:
    bool SaveFile;
    bool hasLoaded;
    time_t LastFileDate;
    std::vector<MapBlock> Blocks;
    
    void LoadOld();
    void Load();
    void MainFunc();
};


#endif //D3PP_BLOCK_H

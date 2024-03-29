//
// Created by Wande on 10/1/2021.
//

#ifndef D3PP_CUSTOMBLOCKS_H
#define D3PP_CUSTOMBLOCKS_H
#define CUSTOM_BLOCK_FILE_NAME "BlockDefs"
#include <string>
#include <map>
#include <vector>
#include "common/TaskScheduler.h"

enum BlockSolidity {
    Walkthrough = 0,
    Swimthrough,
    Solid,
    PartSlip,
    FullSlip,
    Water,
    Lava,
    Rope
};

struct BlockDefinition {
public:
    unsigned char blockId;
    std::string name;
    BlockSolidity solidity;
    char movementSpeed;
    short topTexture;
    short leftTexture;
    short rightTexture;
    short frontTexture;
    short backTexture;
    short bottomTexture;
    bool transmitsLight;
    char walkSound;
    bool fullBright;
    char minX;
    char minY;
    char minZ;
    char maxX;
    char maxY;
    char maxZ;
    char shape;
    char drawType;
    char fogDensity;
    char fogR;
    char fogG;
    char fogB;
};

class CustomBlocks : public TaskItem {
public:
    CustomBlocks();

    static CustomBlocks* GetInstance();

    void MainFunc();
    void Load();
    void Save();
    void Add(BlockDefinition blockDef);
    std::vector<BlockDefinition> GetBlocks();
    void Remove(unsigned char blockId);
    bool HasDef(int blockId);
    BlockDefinition GetDef(int blockId);
private:
    static CustomBlocks* instance;
    bool isModified;
    time_t lastModified;

    std::map<unsigned char, BlockDefinition> _blockDefintiions;
};

#endif //D3PP_CUSTOMBLOCKS_H

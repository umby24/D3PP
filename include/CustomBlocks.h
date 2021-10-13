//
// Created by Wande on 10/1/2021.
//

#ifndef D3PP_CUSTOMBLOCKS_H
#define D3PP_CUSTOMBLOCKS_H
#include <string>
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
    char topTexture;
    char sideTexture;
    char bottomTexture;
    bool transmitsLight;
    char walkSound;
    bool fullBright;
    char shape;
    char drawType;
    char fogDensity;
    char fogR;
    char fogG;
    char fogB;
};

class CustomBlocks {
    void Load();
    void Save();
    void Add();
    void Remove();
    
};
#endif //D3PP_CUSTOMBLOCKS_H

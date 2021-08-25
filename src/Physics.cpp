//
// Created by unknown on 7/2/21.
//

#include "Physics.h"

#include "Map.h"
std::vector<BlockFillPhysics> Physics::_blockFill;

/* Block falls straight down */
void Physics::BlockPhysics10(std::shared_ptr<Map> physMap, int x, int y, int z) {
    int currentBlock = physMap->GetBlockType(x, y, z);
    if (physMap->GetBlockType(x, y, z-1) == 0) {
        physMap->BlockMove(x, y, z, x, y, z-1, true, true, 1);
    }
}

/* Block falls in 45 degree bevels (builds a pyramid) */
void Physics::BlockPhysics11(std::shared_ptr<Map> physMap, int x, int y, int z) {
    int currentBlock = physMap->GetBlockType(x, y, z);
    int blockBelow = physMap->GetBlockType(x, y, z-1);

    int blockBelowRight = physMap->GetBlockType(x+1,y,z-1);
    int blockRight = physMap->GetBlockType(x+1, y, z);

    int blockBelowLeft = physMap->GetBlockType(x-1, y, z-1);
    int blockLeft = physMap->GetBlockType(x-1,y,z);

    int blockBelowForward =  physMap->GetBlockType(x, y+1, z-1);
    int blockForward =  physMap->GetBlockType(x, y+1, z);

    int blockBelowBehind =  physMap->GetBlockType(x, y-1, z-1);
    int blockBehind =  physMap->GetBlockType(x, y-1, z);

    if (blockBelow == 0) {
        physMap->BlockMove(x, y, z, x, y, z-1, true, true, 1);
    } else if (blockBelowRight == 0 && blockRight == 0) {
        physMap->BlockMove(x, y, z, x+1, y, z-1, true, true, 1);
    } else if (blockBelowLeft == 0 && blockLeft == 0) {
        physMap->BlockMove(x, y, z, x-1, y, z-1, true, true, 1);
    } else if (blockBelowForward == 0 && blockForward == 0) {
        physMap->BlockMove(x, y, z, x, y+1,z-1, true, true, 1);
    } else if (blockBelowBehind == 0 && blockBehind == 0) {
        physMap->BlockMove(x, y, z, x, y-1, z-1, true, true, 1);
    }
}

/* Minecraft original fluid physics (Block duplicates laterally and downwardly) */
void Physics::BlockPhysics20(std::shared_ptr<Map> physMap, int x, int y, int z) {
    int currentBlock = physMap->GetBlockType(x, y, z);
    unsigned short blockPlayer = physMap->GetBlockPlayer(x, y, z);
    int blockBelow = physMap->GetBlockType(x, y, z-1);
    int blockRight = physMap->GetBlockType(x+1, y, z);
    int blockLeft = physMap->GetBlockType(x-1,y,z);
    int blockForward =  physMap->GetBlockType(x, y+1, z);
    int blockBehind =  physMap->GetBlockType(x, y-1, z);

    if (blockBelow == 0) {
        physMap->BlockChange(blockPlayer, x, y, z-1, currentBlock, true, true, true, 1);
    } else if (blockRight == 0) {
        physMap->BlockChange(blockPlayer, x+1, y, z, currentBlock, true, true, true, 1);
    } else if (blockLeft == 0) {
        physMap->BlockChange(blockPlayer, x-1, y, z, currentBlock, true, true, true, 1);
    } else if (blockForward == 0) {
        physMap->BlockChange(blockPlayer, x, y+1, z, currentBlock, true, true, true, 1);
    } else if (blockBehind == 0) {
        physMap->BlockChange(blockPlayer, x, y-1, z, currentBlock, true, true, true, 1);
    }
}

/* More realistic fluid (D3 Fluid) */
void Physics::BlockPhysics21(std::shared_ptr<Map> physMap, int x, int y, int z) {
    int currentBlock = physMap->GetBlockType(x, y, z);
    int blockBelow = physMap->GetBlockType(x, y, z-1);

    if (blockBelow == 0) {
        physMap->BlockMove(x, y, z, x, y, z-1, true, true, 1);
        return;
    }
    // -- This is a flood-search algorithm..
    char BlockFillArray[Physics_Fill_X][Physics_Fill_Y];
    for(auto px = 0; px < Physics_Fill_X; px++) {
        for (int py = 0; py < Physics_Fill_Y; ++py) {
            BlockFillArray[px][py] = 0;
        }
    }

    _blockFill.clear();
    BlockFillPhysics item {static_cast<short>(x), static_cast<short>(y), static_cast<short>(z)};
    _blockFill.push_back(item);

    bool found = false;
    BlockFillPhysics pointed;
    while (!_blockFill.empty()) {
        pointed = _blockFill.at(0);
        _blockFill.erase(_blockFill.begin());

        if (physMap->GetBlockType(pointed.X, pointed.Y, pointed.Z-1) == 0) {
            found = true;
            physMap->BlockMove(x, y, z, pointed.X, pointed.Y, pointed.Z-1, true, true, 1);
        } else {
            if (physMap->GetBlockType(pointed.X+1, pointed.Y, pointed.Z) == 0 && BlockFillArray[pointed.X + 1][pointed.Y] == 0) {
                BlockFillArray[pointed.X + 1][pointed.Y] = 1;
                BlockFillPhysics newItem;
                newItem.X = pointed.X + 1;
                newItem.Y = pointed.Y;
                newItem.Z = pointed.Z;
                _blockFill.push_back(newItem);
            }

            if (physMap->GetBlockType(pointed.X-1, pointed.Y, pointed.Z) == 0 && BlockFillArray[pointed.X -1][pointed.Y] == 0) {
                BlockFillArray[pointed.X - 1][pointed.Y] = 1;
                BlockFillPhysics newItem;
                newItem.X = pointed.X - 1;
                newItem.Y = pointed.Y;
                newItem.Z = pointed.Z;
                _blockFill.push_back(newItem);
            }

            if (physMap->GetBlockType(pointed.X, pointed.Y+1, pointed.Z) == 0 && BlockFillArray[pointed.X][pointed.Y+1] == 0) {
                BlockFillArray[pointed.X][pointed.Y+1] = 1;
                BlockFillPhysics newItem;
                newItem.X = pointed.X ;
                newItem.Y = pointed.Y+1;
                newItem.Z = pointed.Z;
                _blockFill.push_back(newItem);
            }

            if (physMap->GetBlockType(pointed.X, pointed.Y-1, pointed.Z) == 0 && BlockFillArray[pointed.X][pointed.Y-1] == 0) {
                BlockFillArray[pointed.X][pointed.Y-1] = 1;
                BlockFillPhysics newItem;
                newItem.X = pointed.X;
                newItem.Y = pointed.Y -1;
                newItem.Z = pointed.Z;
                _blockFill.push_back(newItem);
            }
        }

        if (_blockFill.size() > Max_Water_Search || found) {
            _blockFill.clear();
        }
    }
}

//
// Created by unknown on 7/2/21.
//

#ifndef D3PP_PHYSICS_H
#define D3PP_PHYSICS_H
#define Physics_Fill_X  1024
#define Physics_Fill_Y  1024
#define Max_Water_Search  50000 // Maximale größe beim Suchen nach einer freien Stelle auf einer Fläche

#include <vector>
#include <memory>

struct BlockFillPhysics {
    short X;
    short Y;
    short Z;
};

class Map;

class Physics {
public:
    static void BlockPhysics10(std::shared_ptr<Map> physMap, int x, int y, int z);
    static void BlockPhysics11(std::shared_ptr<Map> physMap, int x, int y, int z);
    static void BlockPhysics20(std::shared_ptr<Map> physMap, int x, int y, int z);
    static void BlockPhysics21(std::shared_ptr<Map> physMap, int x, int y, int z);
private:
    static std::vector<BlockFillPhysics> _blockFill;
    static char BlockFillArray[Physics_Fill_X][Physics_Fill_Y];
    static char blockFillArrayEmpty[Physics_Fill_X][Physics_Fill_Y];
};
#endif //D3PP_PHYSICS_H

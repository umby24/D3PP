#ifndef D3PP_FLATGRASS_GENERATOR
#define D3PP_FLATGRASS_GENERATOR

#include "world/Map.h"
#include "common/Vectors.h"
#include "world/MapMain.h"

class GenTools {
    public:
    static int GetIndex(int x, int y, int z, int mapX, int mapY) {
        return (x + y * mapX + z * mapX * mapY) * 4;
    }

static void FlatgrassGen(int mapId) {
    D3PP::world::MapMain* mm = D3PP::world::MapMain::GetInstance();
    std::shared_ptr<D3PP::world::Map> thisMap = mm->GetPointer(mapId);

    if (thisMap == nullptr) {
        return;
    }

    D3PP::Common::Vector3S mapSize = thisMap->GetSize();
     std::vector<unsigned char> newBlocks;
     newBlocks.resize((mapSize.X * mapSize.Y * mapSize.Z) * 4);

    for(int x = 0; x < mapSize.X; x++) {
        for (int y = 0; y < mapSize.Y; y++) {
            for (int z = 0; z < mapSize.Z/2; z++) {
                int blockType = 3;
                
                if (z == (mapSize.Z/2)-1)
                    blockType = 2;

//                thisMap->BlockChange(-1, x, y, z, blockType, false, false, false, 1);
                newBlocks[GetIndex(x, y, z, mapSize.X, mapSize.Y)] = blockType;
            }
        }
    }

    thisMap->SetBlocks(newBlocks);
}
};



#endif
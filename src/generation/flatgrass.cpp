#include "world/Map.h"
#include "common/Vectors.h"

int GetIndex(int x, int y, int z, int mapX, int mapY) {
    return (x + y * mapX + z * mapX * mapY) * 4;
}

void FlatgrassGen(int mapId) {
    D3PP::world::MapMain* mm = D3PP::world::MapMain::GetInstance();
    std::shared_ptr<D3PP::world::Map> thisMap = mm->GetPointer(mapId);

    if (thisMap == nullptr) {
        return;
    }

    D3PP::Common::Vector3S mapSize = thisMap->GetSize();
    std::vector<unsigned char> newBlocks;
    newBlocks.reserve((mapSize.X * mapSize.Y * mapSize.Z) * 4);

    for(int x = 0; x < mapSize.X; x++) {
        for (int y = 0; y < mapSize.Y; y++) {
            for (int z = 0; z < mapSize.Z/2; z++) {
                int blockType = 3;
                
                if (z == mapSize.Z/2)
                    blockType = 4;

                newBlocks[GetIndex(x, y, z, mapSize.X, mapSize.Y)] = blockType;
            }
        }
    }

    thisMap->SetBlocks(newBlocks);
}
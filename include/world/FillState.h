//
// Created by Wande on 2/3/2022.
//

#ifndef D3PP_FILLSTATE_H
#define D3PP_FILLSTATE_H
#include <vector>
#include "common/Vectors.h"

namespace D3PP::world {
    struct FillState {
        std::vector<unsigned char> fillData;

        FillState(Common::Vector3S mapSize) { MapSize = mapSize; fillData.resize((mapSize.X * mapSize.Y * mapSize.Z) * 4);}
        void SetBlock(Common::Vector3S location, unsigned char type) { int index = GetIndex(location.X, location.Y, location.Z); fillData.at(index) = type; }
    private:
        Common::Vector3S MapSize;
        int GetIndex(int x, int y, int z) {
            return (x + y * MapSize.X + z * MapSize.X * MapSize.Y) * 4;
        }
    };
}
#endif //D3PP_FILLSTATE_H

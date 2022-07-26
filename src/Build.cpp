//
// Created by Wande on 8/4/2021.
//

#include "Build.h"
#include <cmath>
#include <Utils.h>
#include <common/Logger.h>
#include "world/Map.h"
#include "world/MapMain.h"

using namespace D3PP::world;

void Build::BuildLinePlayer(short playerNumber, int mapId, unsigned short X0, unsigned short Y0, unsigned short Z0,
                            unsigned short X1, unsigned short Y1, unsigned short Z1, unsigned char material,
                            unsigned char priority, bool undo, bool physics) {

    auto dx = (float)(X1 - X0);
    auto absDx = (float)std::abs(dx);
    auto dy = (float)(Y1 - Y0);
    auto absDy = (float)std::abs(dy);
    auto dz = (float)(Z1 - Z0);
    auto absDz = (float)std::abs(dz);

    float blocks = 1.0;

    if (blocks < absDx)
        blocks = absDx;

    if (blocks < absDy)
        blocks = absDy;

    if (blocks < absDz)
        blocks = absDz;

    float mx = dx / (float)blocks;
    float my = dy / blocks;
    float mz = dz / blocks;

    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Map> pMap = mapMain->GetPointer(mapId);

    if (pMap != nullptr) {
        for(int i = 0; i < (int)(blocks+1); i++) {
            auto x = static_cast<unsigned short>(X0+mx *i);
            pMap->BlockChange(playerNumber, x, Y0+my*i, Z0+mz*i, material, undo, physics, true, priority);
        }
    }
}

void Build::BuildBoxPlayer(short playerNumber, int mapId, unsigned short X0, unsigned short Y0, unsigned short Z0,
                           unsigned short X1, unsigned short Y1, unsigned short Z1, unsigned char material,
                           char replaceMaterial, bool hollow, unsigned char priority, bool undo,
                           bool physics) {
    auto startX = std::min(X0, X1);
    auto startY = std::min(Y0, Y1);
    auto startZ = std::min(Z0, Z1);
    auto endX = std::max(X0, X1);
    auto endY = std::max(Y0, Y1);
    auto endZ = std::max(Z0, Z1);

    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Map> pMap = mapMain->GetPointer(mapId);

    if (pMap != nullptr) {
        for (int ix = startX; ix < endX+1; ix++) {
            for(int iy = startY; iy < endY+1; iy++) {
                for(int iz = startZ; iz< endZ + 1; iz++) {
                    if (replaceMaterial == -1 || replaceMaterial == pMap->GetBlockType(ix, iy, iz)) {
                        if (!hollow || (ix == startX || ix == endX || iy == startY || iy == endY || iz == startZ || iz== endZ)) {
                            pMap->BlockChange(playerNumber, ix, iy, iz, material, undo, physics, true, priority);
                        }
                    }
                }
            }
        }
    }

}

void Build::BuildSpherePlayer(short playerNumber, int mapId, unsigned short X0, unsigned short Y0, unsigned short Z0,
                              float radius, unsigned char material, char replaceMaterial, bool hollow,
                              unsigned char priority, bool undo, bool physics) {
    int rounded = (int)std::round(radius) + 1;
    auto power = std::pow(radius, 2);

    for(auto ix = -rounded; ix < rounded; ix++) {
        for(auto iy = -rounded; iy < rounded; iy++) {
            for(auto iz = -rounded; iz< rounded; iz++) {
                auto squareDistance = std::pow(ix, 2) + std::pow(iy, 2) + std::pow(iz, 2);
                if (squareDistance > power) {
                    continue;
                }
                bool allowed = false;
                if (hollow) {

                } else {
                    allowed = true;
                }

                if (!allowed)
                    continue;

                MapMain* mapMain = MapMain::GetInstance();
                std::shared_ptr<Map> pMap = mapMain->GetPointer(mapId);

                if (replaceMaterial == -1 || replaceMaterial == pMap->GetBlockType(X0+ix, Y0+iy, Z0+iz)) {
                    pMap->BlockChange(playerNumber, X0+ix, Y0+iy, Z0+iz, material, undo, physics, true, priority);
                }
            }
        }
    }
}

void Build::BuildRankBox(int mapId, unsigned short X0, unsigned short Y0, unsigned short Z0, unsigned short X1,
                         unsigned short Y1, unsigned short Z1, short rank, short maxRank) {
    auto startX = std::min(X0, X1);
    auto startY = std::min(Y0, Y1);
    auto startZ = std::min(Z0, Z1);
    auto endX = std::max(X0, X1);
    auto endY = std::max(Y0, Y1);
    auto endZ = std::max(Z0, Z1);
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Map> pMap = mapMain->GetPointer(mapId);

    if (pMap == nullptr)
        return;

 //   if (pMap->data.RankBuild <= maxRank && rank<= maxRank) {
        // -- Map add rank box.
        pMap->SetRankBox(startX, startY, startZ, endX, endY, endZ, rank);
  //  }
}

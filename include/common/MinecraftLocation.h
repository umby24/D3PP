//
// Created by Wande on 8/25/2021.
//

#ifndef D3PP_MINECRAFTLOCATION_H
#define D3PP_MINECRAFTLOCATION_H
#include "common/Vectors.h"

class MinecraftLocation {
public:
    const short X() { return Location.X; };
    const short Y() { return Location.Y; };
    const short Z() { return Location.Z; };
    float Rotation;
    float Look;
    D3PP::Common::Vector3S Location;

    void SetAsBlockCoords(D3PP::Common::Vector3S blockCoords);
    void SetAsPlayerCoords(D3PP::Common::Vector3S playerCoords);
    void SetAsPlayerCoords(D3PP::Common::Vector3F playerCoords);
    D3PP::Common::Vector3S GetAsBlockCoords() const;

    bool operator==(const MinecraftLocation &other) const {
        return Location.isEqual(other.Location) && other.Rotation == Rotation && other.Look == Look;
    }
};

#endif
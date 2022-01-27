//
// Created by Wande on 8/25/2021.
//

#ifndef D3PP_MINECRAFTLOCATION_H
#define D3PP_MINECRAFTLOCATION_H
#include "common/Vectors.h"

class MinecraftLocation {
public:
    [[nodiscard]] short X() const { return Location.X; };
    [[nodiscard]] short Y() const { return Location.Y; };
    [[nodiscard]] short Z() const { return Location.Z; };
    float Rotation;
    float Look;
    D3PP::Common::Vector3S Location;

    void SetAsBlockCoords(D3PP::Common::Vector3S blockCoords);
    void SetAsPlayerCoords(D3PP::Common::Vector3S playerCoords);
    void SetAsPlayerCoords(D3PP::Common::Vector3F playerCoords);
    [[nodiscard]] D3PP::Common::Vector3S GetAsBlockCoords() const;

    bool operator==(const MinecraftLocation &other) const {
        return Location.isEqual(other.Location) && other.Rotation == Rotation && other.Look == Look;
    }
};

#endif
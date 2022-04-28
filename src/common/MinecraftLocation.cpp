
#include "common/MinecraftLocation.h"

using namespace D3PP::Common;

void MinecraftLocation::SetAsBlockCoords(Vector3S blockCoords) {
    Vector3S newLoc { static_cast<short>(( blockCoords.X * 32)), static_cast<short>((blockCoords.Y * 32)), static_cast<short>(((blockCoords.Z * 32) + 51))};
    Location = newLoc;
}

void MinecraftLocation::SetAsPlayerCoords(Vector3S playerCoords) {
    Location = playerCoords;
}

Vector3S MinecraftLocation::GetAsBlockCoords() const {
    Vector3S result {
        static_cast<short>(Location.X / 32),
        static_cast<short>(Location.Y / 32),
        static_cast<short>((Location.Z - 51) / 32)
    };
    
    return result;
}

void MinecraftLocation::SetAsPlayerCoords(Vector3F inCoords) {
    Location.X = static_cast<short>(inCoords.X * 32);
    Location.Y = static_cast<short>(inCoords.Y * 32);
    Location.Z = static_cast<short>((inCoords.Z * 32) + 51);
}

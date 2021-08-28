
#include "MinecraftLocation.h"

void MinecraftLocation::SetAsBlockCoords(Vector3S blockCoords) {
    Vector3S newLoc { ( blockCoords.X * 32), (blockCoords.Y * 32), ((blockCoords.Z * 32) + 51)};
    Location = newLoc;
}

void MinecraftLocation::SetAsPlayerCoords(Vector3S playerCoords) {
    Location = playerCoords;
}

Vector3S MinecraftLocation::GetAsBlockCoords() {
    Vector3S result {
        Location.X / 32,
        Location.Y / 32,
        (Location.Z - 51) / 32
    };
    
    return result;
}
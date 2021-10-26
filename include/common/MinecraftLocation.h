//
// Created by Wande on 8/25/2021.
//

#ifndef D3PP_MINECRAFTLOCATION_H
#define D3PP_MINECRAFTLOCATION_H

struct Vector3F {
    float X;
    float Y;
    float Z;
};

struct Vector3S {
    short X;
    short Y;
    short Z;

    bool isEqual(const Vector3S &other) {
        return (X == other.X && Y == other.Y && Z == other.Z);
    }
};

class MinecraftLocation {
public:
    const short X() { return Location.X; };
    const short Y() { return Location.Y; };
    const short Z() { return Location.Z; };
    unsigned char Rotation;
    unsigned char Look;
    Vector3S Location;

    void SetAsBlockCoords(Vector3S blockCoords);
    void SetAsPlayerCoords(Vector3S playerCoords);
    void SetAsPlayerCoords(Vector3F playerCoords);
    Vector3S GetAsBlockCoords();

    bool operator==(const MinecraftLocation &other) {
        return Location.isEqual(other.Location) && other.Rotation == Rotation && other.Look == Look;
    }
};

#endif
//
// Created by Wande on 8/25/2021.
//

#ifndef D3PP_MINECRAFTLOCATION_H
#define D3PP_MINECRAFTLOCATION_H

struct Vector3S {
    short X;
    short Y;
    short Z;
};

class MinecraftLocation {
public:
    short X() { return Location.X; };
    short Y() { return Location.Y; };
    short Z() { return Location.Z; };
    unsigned char Rotation;
    unsigned char Look;
    Vector3S Location;

    void SetAsBlockCoords(Vector3S blockCoords);
    void SetAsPlayerCoords(Vector3S playerCoords);
    Vector3S GetAsBlockCoords();
};

#endif
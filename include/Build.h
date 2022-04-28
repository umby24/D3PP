//
// Created by Wande on 8/4/2021.
//

#ifndef D3PP_BUILD_H
#define D3PP_BUILD_H

class Build {
public:
    static void BuildLinePlayer(short playerNumber, int mapId, unsigned short X0, unsigned short Y0, unsigned short Z0,unsigned short X1, unsigned short Y1, unsigned short Z1, unsigned char material, unsigned char priority, bool undo, bool physics);
    static void BuildBoxPlayer(short playerNumber, int mapId, unsigned short X0, unsigned short Y0, unsigned short Z0,unsigned short X1, unsigned short Y1, unsigned short Z1, unsigned char material, char replaceMaterial, bool hollow, unsigned char priority, bool undo, bool physics);
    static void BuildSpherePlayer(short playerNumber, int mapId, unsigned short X0, unsigned short Y0, unsigned short Z0, float radius, unsigned char material, char replaceMaterial, bool hollow, unsigned char priority, bool undo, bool physics);
    static void BuildRankBox(int mapId, unsigned short X0, unsigned short Y0, unsigned short Z0,unsigned short X1, unsigned short Y1, unsigned short Z1, short rank, short maxRank);
};

#endif //D3PP_BUILD_H

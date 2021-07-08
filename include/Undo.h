//
// Created by unknown on 7/2/21.
//

#ifndef D3PP_UNDO_H
#define D3PP_UNDO_H
#include <vector>
#include <time.h>

struct UndoEntry {
    short playerNumber;
    int mapId;
    unsigned short X;
    unsigned short Y;
    unsigned short Z;
    time_t time;
    unsigned char oldType;
    short playerNumberBefore;
};

class Undo {
public:
    Undo();
    static void Add(short playerNumber, int mapId, unsigned short X, unsigned short Y, unsigned short Z, unsigned char typeBefore, short playerBefore);
    static void UndoPlayer(int mapId, short playerNumber, time_t time);
    static void UndoTime(int mapId, time_t time);
    static void ClearMap(int mapId);
    static void Clear();
    static Undo* GetInstance();
    static Undo* Instance;
private:
    std::vector<UndoEntry> _undoSteps;
};
#endif //D3PP_UNDO_H

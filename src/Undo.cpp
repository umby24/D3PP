//
// Created by unknown on 7/2/21.
//

#include "Undo.h"
#include "world/Map.h"

Undo* Undo::Instance = nullptr;
using namespace D3PP::world;

Undo *Undo::GetInstance() {
    if (Instance == nullptr)
        Instance = new Undo();

    return Instance;
}

Undo::Undo() = default;

void
Undo::Add(short playerNumber, int mapId, unsigned short X, unsigned short Y, unsigned short Z, unsigned char typeBefore,
          short playerBefore) {
	const UndoEntry newEntry {playerNumber, mapId, X, Y, Z, time(nullptr), typeBefore, playerBefore};
    Undo* undoMain = GetInstance();
    undoMain->_undoSteps.push_back(newEntry);
}

void Undo::UndoPlayer(int mapId, short playerNumber, time_t time) {
    Undo* undoMain = GetInstance();
    MapMain* mm = MapMain::GetInstance();

    for(auto i = 0; i< undoMain->_undoSteps.size(); i++) {
        const auto &item = undoMain->_undoSteps.at(i);

        if (item.playerNumber == playerNumber && item.time >= time) {
            if (mapId == -1 || item.mapId == mapId) {
                std::shared_ptr<Map> chgMap = mm->GetPointer(mapId);
                if (chgMap != nullptr) {
                    chgMap->BlockChange(item.playerNumberBefore, item.X, item.Y, item.Z, item.oldType, false, false, true, 5);
                    undoMain->_undoSteps.erase(undoMain->_undoSteps.begin() + i);
                    if (i != 0) i--;
                }
            }
        }
    }

}

void Undo::UndoTime(int mapId, time_t time) {
    Undo* undoMain = GetInstance();
    MapMain* mm = MapMain::GetInstance();

    for(auto i = 0; i< undoMain->_undoSteps.size(); i++) {
        const auto &item = undoMain->_undoSteps.at(i);

        if (item.time >= time) {
            if (mapId == -1 || item.mapId == mapId) {
                std::shared_ptr<Map> chgMap = mm->GetPointer(mapId);
                if (chgMap != nullptr) {
                    chgMap->BlockChange(item.playerNumberBefore, item.X, item.Y, item.Z, item.oldType, false, false, true, 5);
                    undoMain->_undoSteps.erase(undoMain->_undoSteps.begin() + i);
                    if (i != 0) i--;
                }
            }
        }
    }
}

void Undo::ClearMap(int mapId) {
    Undo* undoMain = GetInstance();

    for(auto i = 0; i< undoMain->_undoSteps.size(); i++) {
        const auto &item = undoMain->_undoSteps.at(i);
        if (item.mapId == mapId) {
            undoMain->_undoSteps.erase(undoMain->_undoSteps.begin() + i);
            if (i != 0) i--;
        }
    }

}

void Undo::Clear() {
    Undo* undoMain = GetInstance();
    undoMain->_undoSteps.clear();
}

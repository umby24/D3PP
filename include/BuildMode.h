//
// Created by unknown on 6/15/2021.
//

#ifndef D3PP_BUILDMODE_H
#define D3PP_BUILDMODE_H

#include <string>
#include <vector>
#include <map>

#include "common/TaskScheduler.h"


const std::string BUILD_MODE_FILE_NAME = "Build_Mode";
const int BUILD_MODE_BLOCKS_TO_RESEND_SIZE_MAX = 1000;

struct BlockResend {
    int clientId;
    int mapId;
    unsigned short X;
    unsigned short Y;
    unsigned short Z;
};

class BuildMode {
    public:
    std::string ID;
    std::string Name;
    std::string Plugin;
};

class BuildModeMain : TaskItem {
public:
    BuildModeMain();
    static BuildModeMain* GetInstance();
    static BuildModeMain* Instance;
     void Distribute(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z, bool mode, unsigned char blockType);

     void SetState(int clientId, char state);
     char GetState(int clientId);
     void SetCoordinate(int clientId, int index, unsigned short X, unsigned short Y, unsigned short Z);
     unsigned short GetCoordinateX(int clientId, int index);
     unsigned short GetCoordinateY(int clientId, int index);
     unsigned short GetCoordinateZ(int clientId, int index);
     void SetInt(int clientId, int index, int val);
     int GetInt(int clientId, int index);
     void SetFloat(int clientId, int index, float val);
     float GetFloat(int clientId, int index);
     void SetString(int clientId, int index, std::string val);
     std::string GetString(int clientId, int index);

    void SetMode(int clientId, std::string mode);

private:
    std::vector<BlockResend> _resendBlocks;
    std::map<std::string, BuildMode> _buildmodes;

    void Load();
    void Save();
   
    void Resend(int clientId);

    void MainFunc();
    bool SaveFile;
    bool hasLoaded;
    time_t LastFileDate;
};
#endif
//
// Created by unknown on 6/15/2021.
//

#ifndef D3PP_BUILDMODE_H
#define D3PP_BUILDMODE_H

#include <string>
#include <vector>
#include <map>

#include "Files.h"
#include "TaskScheduler.h"
#include "common/PreferenceLoader.h"
#include "Network.h"
#include "Utils.h"

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
private:
    std::vector<BlockResend> _resendBlocks;
    std::map<std::string, BuildMode> _buildmodes;

    void Load();
    void Save();
   
    void Resend(int clientId);
    void SetMode(int clientId, std::string mode);
    void MainFunc();
    bool SaveFile;
    bool hasLoaded;
    time_t LastFileDate;
};
#endif
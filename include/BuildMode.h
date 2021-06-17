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
#include "Utils.h"

const std::string BUILD_MODE_FILE_NAME = "Build_Mode";

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
private:
    std::vector<BlockResend> _resendBlocks;
    std::map<std::string, BuildMode> _buildmodes;

    void Load();
    void Save();
    void Distribute(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z, bool mode, unsigned char blockType);
    void Resend(int clientId);
    void SetMode(int clientId, std::string mode);
    void MainFunc();
    bool SaveFile;
    bool hasLoaded;
    time_t LastFileDate;
};
#endif
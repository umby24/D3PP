//
// Created by unknown on 6/29/21.
//

#ifndef D3PP_HEARTBEAT_H
#define D3PP_HEARTBEAT_H
#include <string>
#include <chrono>

#include "TaskScheduler.h"
#include "json.hpp"

const char *const CLASSICUBE_NET_URL = "http://www.classicube.net";
const char *const CLASSICUBE_HEARTBEAT_PATH = "/server/heartbeat/";

const std::string HEARTBEAT_FILE_NAME = "Heartbeat_HTML";
using json = nlohmann::json;

class Heartbeat : TaskItem {
public:
    Heartbeat();
    static std::string CreateSalt();
    void Load();
    void Save();
    void Init();
    void MainFunc();
    void Beat();
    bool VerifyName(std::string name, std::string pass);
    static Heartbeat* GetInstance();
    static Heartbeat* Instance;
private:
    std::string salt;
    std::string serverUrl;
    bool isPublic;
    bool SaveFile;
    bool LoadFile;
    time_t lastBeat;
    time_t FileDateLast;
};

#endif //D3PP_HEARTBEAT_H

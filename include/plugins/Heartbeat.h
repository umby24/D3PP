//
// Created by unknown on 6/29/21.
//

#ifndef D3PP_HEARTBEAT_H
#define D3PP_HEARTBEAT_H
#include <string>
#include <chrono>

#include "common/TaskScheduler.h"


const char *const CLASSICUBE_NET_URL = "http://www.classicube.net";
const char *const CLASSICUBE_HEARTBEAT_PATH = "/server/heartbeat/";

const std::string HEARTBEAT_FILE_NAME = "Heartbeat_HTML";


class Heartbeat : TaskItem {
public:
    Heartbeat();
    static std::string CreateSalt();
    void Init();
    void MainFunc();
    void Beat();
    bool VerifyName(std::string name, std::string pass);
    static Heartbeat* GetInstance();
    static Heartbeat* Instance;
private:
    bool isFirstBeat;
    std::string salt;
    std::string serverUrl;
    bool isPublic;
    time_t lastBeat;

    static int GetUniqueOnlinePlayers();
    void TeardownFunc();
};

#endif //D3PP_HEARTBEAT_H

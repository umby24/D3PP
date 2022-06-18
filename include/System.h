//
// Created by Wande on 2/25/2021.
//

#ifndef D3PP_SYSTEM_H
#define D3PP_SYSTEM_H
#include <string>
#include <mutex>

const float SYSTEM_VERSION_NUMBER = 0.12f;

class System  {
public:
    static bool IsRunning;
    static time_t startTime;
    static System* GetInstance();
    static System* Instance_;
    static std::mutex mainMutex;
    static std::string ServerName;
};
#endif //D3PP_SYSTEM_H

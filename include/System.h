//
// Created by Wande on 2/25/2021.
//

#ifndef D3PP_SYSTEM_H
#define D3PP_SYSTEM_H
#include <string>
#include <ctime>

const float SYSTEM_VERSION_NUMBER = 0.16f;

class System  {
public:
    static bool IsRunning;
    static time_t startTime;
    static std::string ServerName;
};
#endif //D3PP_SYSTEM_H

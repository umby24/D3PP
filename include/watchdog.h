//
// Created by Wande on 2/25/2021.
//

#ifndef D3PP_WATCHDOG_H
#define D3PP_WATCHDOG_H
#include <string>
#include <vector>
#include <mutex>
#include <thread>

#include "common/TaskScheduler.h"

const std::string WATCHDOG_HTML_NAME = "Watchdog_HTML";

struct WatchdogModule {
    std::string Name;
    std::string LastMessage;
    std::string BiggestMessage;
    int WatchTime;
    int Timeout;
    int BiggestTimeout;
    int MaxTimeout;
    int CallsPerSecond;
    int CpuTime;
    int CpuTime0;
    int CpuTime4Percent;
};

class watchdog : TaskItem {
public:
    watchdog();
    static void Watch(const std::string &moadule, const std::string &message, int state);
    static watchdog* GetInstance();
protected:

    static watchdog* singleton_;
    std::mutex _lock;
    std::vector<WatchdogModule> _modules;
private:
    bool isRunning;
    void HtmlStats(time_t time_);
    void MainFunc();
    std::thread mainThread;
};

const std::string HTML_TEMPLATE = R"(<html>
<head>
    <title>Minecraft-Server Watchdog</title>
  </head>
  <body>
    <b><u>Modules:</u></b><br>
    <br>
    <table border=1>      <tr>
        <th><b>Name</b></th>
        <th><b>State</b></th>
        <th><b>Timeout</b></th>
        <th><b>Timeout_Message</b></th>
        <th><b>Last_Message</b></th>
        <th><b>Calls</b></th>
        <th><b>CPU</b></th>
        <th><b>Thread-Handle</b></th>
        <th><b>Kernel-Time</b></th>
        <th><b>Usermode-Time</b></th>
      </tr>
     [MODULE_TABLE]
    </table>      <br>
      <br>
      <br>
    Site generated in [GEN_TIME] ms. [GEN_TIMESTAMP]<br>
  </body>
</html>
)";

#endif //D3PP_WATCHDOG_H

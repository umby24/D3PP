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

    int GetVirtualRAMUsage();
    int GetPhysicalRAMUsage();
protected:

    static watchdog* singleton_;
    std::mutex _lock;
    std::vector<WatchdogModule> _modules;
private:
    bool isRunning;
    void HtmlStats(time_t time_);
    void MainFunc();
    std::thread mainThread;
    void DoTeardown();
};

const std::string HTML_TEMPLATE = R"(<html>
<head>
    <title>D3PP Watchdog</title>
    <style type=""text/css"">
            body {
                font-family: ""Microsoft PhagsPa"";
                color:#2f2f2f;
                background-color:#F7F7F7;
            }
            h1.header {
                background-color:darkblue;
                text-shadow:2px 1px 0px rgba(0,0,0,.2); 
                font-size:25px;
                font-weight:bold;
                text-decoration:none;
                text-align:center;
                color:white;
                margin:0;
                height:42px; 
                width:auto;
                border-bottom: 1px black solid;
                height: 42px;
                margin: -8px;
                line-height: 42px;
            }
            table {
                border: 1px solid #A0A0A0;
                table-layout: auto;
                empty-cells: hide;
                border-collapse: collapse;
            }
            tr {
                border: 1px solid #A0A0A0;
                background-color: #D0D0D0;
                color: #212121;
                opacity:1.0;
            }
            td {
                border-right: 1px solid #A0A0A0;
            }
            th {
                border-right: 1px solid #A0A0A0;
            }
            .wellText {
                font-weight: bold;
                color: #00AA00;
            }
            .laggingText {
                font-weight: bold;
                color: #AA0000;
            }
        </style>
  </head>
  <body>
    <h1 class="header">D3PP Watchdog</h1>
    <br>
    <h3>Watch Modules:</h3>
    <br>
    <table>      <tr>
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
    Virtual Ram Usage: [VIRT]<br />
    Physical Ram Usage: [PHYS]<br />
  </body>
</html>
)";

#endif //D3PP_WATCHDOG_H

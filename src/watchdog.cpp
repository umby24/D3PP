//
// Created by Wande on 2/25/2021.
//

#include "watchdog.h"
#include "common/Files.h"
#include "Utils.h"
#include "common/Logger.h"
#include <ctime>

watchdog* watchdog::singleton_ = nullptr;

void watchdog::Watch(const std::string &moadule, const std::string &message, int state) {
    watchdog* i = GetInstance();
    const std::scoped_lock pLock(i->_lock);
    int myI = 0;
    for(auto &item : i->_modules) {
        if (item.Name != moadule) {
            myI++;
            continue;
        }

        item.Timeout = clock() - item.WatchTime;
        if (item.BiggestTimeout < item.Timeout) {
            item.BiggestTimeout = item.Timeout;
            item.BiggestMessage = item.LastMessage;
        }
        item.LastMessage = message;
        const clock_t current = clock();

        if (state == 0) {
            item.CpuTime0 = current;
        } else if (state == 2) {
            item.CpuTime += (current - item.CpuTime0);
            item.CpuTime4Percent += (current- item.CpuTime0);
            item.CallsPerSecond++;
        }
        item.WatchTime = current;
        i->_modules[myI] = item;
        myI++;
    }
}

watchdog *watchdog::GetInstance() {
    if (singleton_ == nullptr)
        singleton_ = new watchdog();

    return singleton_;
}

void watchdog::MainFunc() {
    clock_t timer = 0;

    while (isRunning) {
	    const clock_t currentTime = clock(); // -- generationTIme and timer
	    const clock_t time_ = currentTime - timer;
        timer = currentTime;
	    {
		    int i = 0;
		    // -- Artificial block for scoped lock
            const std::scoped_lock<std::mutex> pLock(_lock);
            for (auto &item : _modules) {
                item.CpuTime = item.CpuTime4Percent * 100 / time_;
                item.CpuTime4Percent = 0;

                item.Timeout = currentTime - item.WatchTime;
                if (item.BiggestTimeout < item.Timeout) {
                    item.BiggestTimeout = item.Timeout;
                }
                _modules[i] = item;
                i++;
            }
        }
        HtmlStats(time_);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void watchdog::HtmlStats(time_t time_) {
    time_t start_time = time(nullptr);
    clock_t div_time = clock();
    std::string result = HTML_TEMPLATE;

    // -- Module Table Generation
    std::string modTable;
    int i = 0;
    for(auto &item : _modules) {
       modTable += "<tr>\n";
       modTable += "<td>" + item.Name + "</td>\n";
        if (item.BiggestTimeout >= item.MaxTimeout)
            modTable += "<td><font color=\"#AA0000\"><b>Lagging</b></font></td>\n";
        else
            modTable += "<td><font color=\"#00AA00\"><b>Well</b></font></td>\n";

        modTable += "<td>" + stringulate(item.BiggestTimeout) + "ms (Max. " + stringulate(item.MaxTimeout) + "ms)</td>\n";
        modTable += "<td>" + item.BiggestMessage + "</td>\n";
        modTable += "<td>" + item.LastMessage + "</td>\n";
        modTable += "<td>" + stringulate((item.CallsPerSecond*100/div_time)) + "/s</td>\n";
        modTable += "<td>" + stringulate(item.CpuTime) + "%</td>\n";
        modTable += "</tr>\n";

        item.CallsPerSecond = 0;
        item.BiggestTimeout = 0;
        _modules[i] = item;
        i++;
    }
    std::string modTableStr = "[MODULE_TABLE]";
    Utils::replaceAll(result, modTableStr, modTable);

    time_t finishTime = time(nullptr);
    time_t duration = finishTime - start_time;
    char buffer[255];
    struct tm *tme_info = new tm{};
    #if defined(__unix__)
        localtime_r(&finishTime, tme_info);
    #elif defined(_MSC_VER)
        localtime_s(tme_info, &finishTime);
    #else
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        tme_info = std::localtime((const time_t*)(&finishTime));
    #endif
    
    strftime(buffer, sizeof(buffer), "%H:%M:%S  %m-%d-%Y", tme_info);
    std::string meh(buffer);
    std::string genTimeStr = "[GEN_TIME]";
    std::string genTSStr = "[GEN_TIMESTAMP]";
    std::string durationStr = stringulate(duration);
    Utils::replaceAll(result, genTimeStr, durationStr);
    Utils::replaceAll(result, genTSStr, meh);

    std::string memFile = Files::GetFile(WATCHDOG_HTML_NAME);
    delete tme_info;
    if (std::ofstream oStream(memFile, std::ios::out | std::ios::trunc); oStream.is_open()) {
        oStream << result;
        oStream.close();
    } else {
        Logger::LogAdd("Watchdog", "Couldn't open file :<" + memFile, WARNING, GLF);
    }
}

watchdog::watchdog() {
    struct WatchdogModule mainModule { .Name = "Main", .MaxTimeout =  200 };
    struct WatchdogModule physMod { .Name = "Map_Physic", .MaxTimeout =  400 };
    struct WatchdogModule bcMod { .Name = "Map_Blockchanging", .MaxTimeout =  400 };
    struct WatchdogModule actionMod { .Name = "Map_Action", .MaxTimeout =  10000 };

    _modules.push_back(mainModule);
    _modules.push_back(physMod);
    _modules.push_back(bcMod);
    _modules.push_back(actionMod);
    isRunning = true;

    this->Teardown = [this] { isRunning = false; };
    this->Interval = std::chrono::seconds(100);
    this->LastRun = std::chrono::system_clock::now();
    std::thread myThread(&watchdog::MainFunc, this);
    std::swap(myThread, mainThread);
    TaskScheduler::RegisterTask("Watchdog", *this);
}

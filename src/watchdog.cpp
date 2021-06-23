//
// Created by Wande on 2/25/2021.
//

#include "watchdog.h"
watchdog* watchdog::singleton_ = nullptr;

void watchdog::Watch(std::string module, std::string message, int state) {
    watchdog* i = watchdog::GetInstance();
    i->_lock.lock();
    int myI = 0;
    for(auto item : i->_modules) {
        if (item.Name != module) {
            myI++;
            continue;
        }

        item.Timeout = clock() - item.WatchTime;
        if (item.BiggestTimeout < item.Timeout) {
            item.BiggestTimeout = item.Timeout;
            item.BiggestMessage = item.LastMessage;
        }
        item.LastMessage = message;
        clock_t current = clock();

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
    i->_lock.unlock();
}

watchdog *watchdog::GetInstance() {
    if (singleton_ == nullptr)
        singleton_ = new watchdog();

    return singleton_;
}

void watchdog::MainFunc() {
    clock_t timer;
    while (isRunning) {
        _lock.lock();
        clock_t currentTime = clock(); // -- generationTIme and timer
        clock_t time_ = currentTime - timer;
        timer = currentTime;
        int i = 0;
        for(auto item : _modules) {
            item.Timeout = currentTime - item.WatchTime;
            if (item.BiggestTimeout < item.Timeout) {
                item.BiggestTimeout = item.Timeout;
            }
            _modules[i] = item;
            i++;
        }

        HtmlStats(time_);
        _lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void watchdog::HtmlStats(time_t time_) {
    time_t startTime = time(nullptr);
    clock_t divTime = clock();
    std::string result = HTML_TEMPLATE;

    // -- Module Table Generation
    std::string modTable;
    int i = 0;
    for(auto item : _modules) {
       modTable += "<tr>\n";
       modTable += "<td>" + item.Name + "</td>\n";
        if (item.BiggestTimeout >= item.MaxTimeout)
            modTable += "<td><font color=\"#AA0000\"><b>Lagging</b></font></td>\n";
        else
            modTable += "<td><font color=\"#00AA00\"><b>Well</b></font></td>\n";

        modTable += "<td>" + stringulate(item.BiggestTimeout) + "ms (Max. " + stringulate(item.MaxTimeout) + "ms)</td>\n";
        modTable += "<td>" + item.BiggestMessage + "</td>\n";
        modTable += "<td>" + item.LastMessage + "</td>\n";
        modTable += "<td>" + stringulate((item.CallsPerSecond*1000/divTime)) + "/s</td>\n";
        modTable += "<td>" + stringulate(item.CpuTime) + "%</td>\n";
        modTable += "</tr>\n";

        item.CallsPerSecond = 0;
        item.BiggestTimeout = 0;
        _modules[i] = item;
        i++;
    }
    Utils::replaceAll(result, "[MODULE_TABLE]", modTable);

    time_t finishTime = time(nullptr);
    long duration = finishTime - startTime;
    char buffer[255];
    strftime(buffer, sizeof(buffer), "%H:%M:%S  %m-%d-%Y", localtime(reinterpret_cast<const time_t *>(&finishTime)));
    std::string meh(buffer);
    Utils::replaceAll(result, "[GEN_TIME]", stringulate(duration));
    Utils::replaceAll(result, "[GEN_TIMESTAMP]", meh);

    Files* files = Files::GetInstance();
    std::string memFile = files->GetFile(WATCHDOG_HTML_NAME);

    std::ofstream oStream(memFile, std::ios::out | std::ios::trunc);
    if (oStream.is_open()) {
        oStream << result;
        oStream.close();
    } else {
        Logger::LogAdd("Watchdog", "Couldn't open file :<" + memFile, LogType::WARNING, __FILE__, __LINE__, __FUNCTION__ );
    }
}

watchdog::watchdog() {
    struct WatchdogModule mainModule { .Name = "Main", .MaxTimeout =  200 };
    struct WatchdogModule netMod { .Name = "Network", .MaxTimeout =  200 };
    struct WatchdogModule physMod { .Name = "Map_Physic", .MaxTimeout =  400 };
    struct WatchdogModule bcMod { .Name = "Map_BlockChanging", .MaxTimeout =  400 };
    struct WatchdogModule actionMod { .Name = "Map_Action", .MaxTimeout =  10000 };
    struct WatchdogModule loginMod { .Name = "Client_login", .MaxTimeout =  2000 };

    _modules.push_back(mainModule);
    _modules.push_back(netMod);
    _modules.push_back(physMod);
    _modules.push_back(bcMod);
    _modules.push_back(actionMod);
    _modules.push_back(loginMod);
    isRunning = true;

    this->Teardown = [this] { isRunning = false; };

    std::thread myThread(&watchdog::MainFunc, this);
    swap(myThread, mainThread);
    TaskScheduler::RegisterTask("Watchdog", *this);
}

#include <Files.h>
#include <Logger.h>
#include <Block.h>
#include "Rank.h"
using namespace std;
bool isRunning = false;
void mainLoop();

int main()
{
    Logger::LogAdd("Main", "====== Welcome to D3PP =====", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    Block b;
    Rank r;

    TaskScheduler::RunSetupTasks();
    isRunning = true;
    mainLoop();
    isRunning = false;
    TaskScheduler::RunTeardownTasks();

    Logger::LogAdd("Module", "Server shutdown complete.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    return 0;
}

void mainLoop() {
    while (isRunning) {
        TaskScheduler::RunMainTasks();
       // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
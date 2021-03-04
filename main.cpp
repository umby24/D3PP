#include "network/WindowsServerSockets.h"

#include <Files.h>
#include <Logger.h>
#include <Block.h>
#include <thread>

#include "Rank.h"
#include "Mem.h"
#include "watchdog.h"
#include "System.h"
#include "Network.h"

using namespace std;
bool isRunning = false;
void mainLoop();
void MainConsole();

int main()
{
    Logger::LogAdd("Main", "====== Welcome to D3PP =====", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    Block b;
    Rank r;
    System s;
    Network n;

    TaskScheduler::RunSetupTasks();
    isRunning = true;
    n.Load();
    n.Save();

    char* someMemory = Mem::Allocate(20, "Main.cpp", 15, "Test");
    someMemory[0] = 5;
    someMemory[1] = 10;
    n.Start();

    std::thread mainThread(mainLoop);
    n.meh();
    MainConsole();

    Mem::Free(someMemory);
    TaskScheduler::RunTeardownTasks();

    Logger::LogAdd("Module", "Server shutdown complete.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    return 0;
}

void MainConsole() {
    std::string input;

    while (isRunning) {
        getline(cin, input);
        if (input == "q" || input == "quit") {
            isRunning = false;
        }
    }
}

void mainLoop() {
    while (isRunning) {
        watchdog::Watch("Main", "Begin Mainslope", 0);
        TaskScheduler::RunMainTasks();
        watchdog::Watch("Main", "End Mainslope", 2);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
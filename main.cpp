#include "network/WindowsServerSockets.h"
#include <string>
#include <vector>

#include <Files.h>
#include <Logger.h>
#include <Block.h>
#include <thread>

#include "Rank.h"
#include "Mem.h"
#include "watchdog.h"
#include "System.h"
#include "Network.h"
#include "Player_List.h"

using namespace std;
bool isRunning = false;
void mainLoop();
void MainConsole();
int MainVersion = 1018;

int main()
{
    srand(time(nullptr));
    Logger::LogAdd("Main", "====== Welcome to D3PP =====", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    Block *b = Block::GetInstance();
    Rank *r = Rank::GetInstance();
    System s;
    Player_List *l = Player_List::GetInstance();
    PlayerMain *pm = PlayerMain::GetInstance();
    Network *n = Network::GetInstance();

    TaskScheduler::RunSetupTasks();
    isRunning = true;
    n->Load();
    n->Save();
    n->Start();
    std::thread mainThread(mainLoop);
    MainConsole();

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
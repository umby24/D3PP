#include <string>
#include <thread>

#include "Network.h"
#include "Rank.h"
#include "Mem.h"
#include "watchdog.h"
#include "System.h"
#include "Logger.h"
#include "Block.h"
#include "Entity.h"
#include "BuildMode.h"

#include "Player_List.h"
#include "Command.h"

using namespace std;

void mainLoop();
void MainConsole();
int MainVersion = 1018;

int main()
{
    std::set_terminate([](){ 
        std::cout << "Unhandled exception" << std::endl; std::abort();
    }
    );

    srand(time(nullptr));
    Logger::LogAdd("Main", "====== Welcome to D3PP =====", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    Block *b = Block::GetInstance();
    Rank *r = Rank::GetInstance();
    System *s = System::GetInstance();
    Player_List *l = Player_List::GetInstance();
    PlayerMain *pm = PlayerMain::GetInstance();
    Network *n = Network::GetInstance();
    EntityMain em;
    CommandMain *cm = CommandMain::GetInstance();
    BuildModeMain *bmm = BuildModeMain::GetInstance();
    
    TaskScheduler::RunSetupTasks();
    System::IsRunning = true;
    n->Load();
    n->Save();
    n->Start();
    
    std::thread mainThread(mainLoop);
    std::thread clientLoginThread(Client::LoginThread);
    
    MainConsole();

    TaskScheduler::RunTeardownTasks();

    Logger::LogAdd("Module", "Server shutdown complete.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    return 0;
}

void MainConsole() {
    std::string input;

    while (System::IsRunning) {
        getline(cin, input);
        if (input == "q" || input == "quit") {
            System::IsRunning = false;
        }
    }
}

void mainLoop() {
    while (System::IsRunning) {
        watchdog::Watch("Main", "Begin Mainslope", 0);
        TaskScheduler::RunMainTasks();
        watchdog::Watch("Main", "End Mainslope", 2);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
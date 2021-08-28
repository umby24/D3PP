#include <string>
#include <thread>

#include "Network.h"
#include "Rank.h"
#include "Mem.h"
#include "System.h"
#include "Logger.h"
#include "Client.h"

#include "Block.h"
#include "Player.h"
#include "Entity.h"
#include "BuildMode.h"
#include "Heartbeat.h"
#include "watchdog.h"

#include "Player_List.h"
#include "Command.h"
#include "plugins/LuaPlugin.h"

using namespace std;

void mainLoop();
void MainConsole();
int MainVersion = 1018;

int main()
{
    std::set_terminate([](){ 
        std::exception_ptr eptr = std::current_exception();
        if (eptr)
        {
            try
            {
                std::rethrow_exception(eptr);
            }
            catch (const std::exception& e)
            {
                std::cout << "AHHH" << e.what() << std::endl;
                //DBG_FAIL(e.what());
            }
            catch (...)
            {
                //DBG_FAIL("Unknown exception.");
            }
        }
        std::cout << "Unhandled exception" << std::endl; std::abort();
    }
    );
 //   _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
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
    Heartbeat* hb = Heartbeat::GetInstance();
    LuaPlugin* llll = LuaPlugin::GetInstance();
    watchdog* wd = watchdog::GetInstance();
    TaskScheduler::RunSetupTasks();
    System::IsRunning = true;
    System::startTime = time(nullptr);
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
        TaskScheduler::RunMainTasks();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
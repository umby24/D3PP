#include <iostream>
#include <string>
#include <thread>
#include <CustomBlocks.h>

#include "network/Network.h"
#include "network/Server.h"
#include "Rank.h"
#include "System.h"
#include "common/Logger.h"
#include "world/MapMain.h"
#include "Block.h"
#include "world/Player.h"
#include "world/Entity.h"
#include "BuildMode.h"
#include "plugins/Heartbeat.h"
#include "plugins/PluginManager.h"
#include "watchdog.h"
#include "common/Files.h"
#include "common/Player_List.h"
#include "Command.h"
#include "plugins/LuaPlugin.h"
#include "plugins/RestApi.h"
#include "common/Configuration.h"
#include "ConsoleClient.h"
#include "network/Network_Functions.h"
#include "world/Map.h"

#if _WIN32
#include <windows.h>
#endif
using namespace std;

void mainLoop();
void MainConsole();
int MainVersion = 1018;

void fixWindowsTerminal() {
#if _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    if (!(dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#endif
}

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
                std::cout << "An Exception occurred: " << e.what() << std::endl;
                //DBG_FAIL(e.what());
            }
            catch (...)
            {
                std::cout << "Unknown exception occurred, exiting.";
                std::abort();
            }
        }
    }
    );

    fixWindowsTerminal();

    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
    srand(time(nullptr));

    Files::Load();
    Logger::LogAdd("Main", "====== Welcome to D3PP =====", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    Configuration* config = Configuration::GetInstance();

    Block *b = Block::GetInstance();
    Rank *r = Rank::GetInstance();
    
    Player_List *l = Player_List::GetInstance();
    PlayerMain *pm = PlayerMain::GetInstance();
    EntityMain em;
    CommandMain *cm = CommandMain::GetInstance();
    BuildModeMain *bmm = BuildModeMain::GetInstance();
    Heartbeat* hb = Heartbeat::GetInstance();
    RestApi rapi;
    D3PP::plugins::PluginManager *plugm = D3PP::plugins::PluginManager::GetInstance();
    watchdog* wd = watchdog::GetInstance();
    CustomBlocks* cb = CustomBlocks::GetInstance();
    D3PP::world::MapMain* mm = D3PP::world::MapMain::GetInstance();

    TaskScheduler::RunSetupTasks();

    System::IsRunning = true;
    System::startTime = time(nullptr);
    System::ServerName = "D3PP Beta v" + stringulate(SYSTEM_VERSION_NUMBER);
    
    D3PP::network::Server::Start();
    
    std::thread mainThread(mainLoop);
    plugm->LoadPlugins();
    MainConsole();

    D3PP::network::Server::Stop();
    TaskScheduler::RunTeardownTasks();

    Logger::LogAdd("Module", "Server shutdown complete.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    return 0;
}

void MainConsole() {
    auto cc = ConsoleClient::GetInstance();
    CommandMain* cm = CommandMain::GetInstance();
    std::string input;

    while (System::IsRunning) {
        getline(cin, input);
        if (input.empty()) {
            continue;
        }

        if (input.substr(0, 1) == "/") {
            cm->CommandDo(cc, input.substr(1));
        } else if (input == "q" || input == "quit") {
            System::IsRunning = false;
        } else {
            NetworkFunctions::SystemMessageNetworkSend2All(-1, "&c[&fCONSOLE&c]:&f " + input);
        }
    }
}

void mainLoop() {
    while (System::IsRunning) {
        TaskScheduler::RunMainTasks();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
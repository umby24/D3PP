#include <iostream>
#include <string>
#include <thread>

#include "network/Server.h"
#include "Rank.h"
#include "System.h"
#include "common/Logger.h"
#include "Block.h"
#include "plugins/PluginManager.h"
#include "common/Files.h"
#include "Command.h"
#include "plugins/LuaPlugin.h"
#include "ConsoleClient.h"
#include "network/Network_Functions.h"
#include "world/Map.h"

#if _WIN32
#include <windows.h>
#endif
using namespace std;

void main_loop();
void main_console();
int main_version = 1018;

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
		    if (const std::exception_ptr exception_pointer = std::current_exception())
        {
            try
            {
                std::rethrow_exception(exception_pointer);
            }
            catch (const std::exception& e)
            {
                std::cout << "An Exception occurred: " << e.what() << '\n';
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
    Logger::LogAdd("Main", "====== Welcome to D3PP =====", NORMAL, GLF);

    TaskScheduler::RunSetupTasks();

    System::IsRunning = true;
    System::startTime = time(nullptr);
    System::ServerName = "D3PP Beta v" + stringulate(SYSTEM_VERSION_NUMBER);
    
    D3PP::network::Server::Start();

    std::thread main_thread(main_loop);
    plug_manager->LoadPlugins();

    main_console();

    D3PP::network::Server::Stop();
    TaskScheduler::RunTeardownTasks();

    Logger::LogAdd("Module", "Server shutdown complete.", NORMAL, GLF);
    return 0;
}

void main_console() {
	const auto cc = ConsoleClient::GetInstance();
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

void main_loop() {
    while (System::IsRunning) {
        TaskScheduler::RunMainTasks();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "YO WTF WE EXITING WHY";
}

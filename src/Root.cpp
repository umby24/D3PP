#include "Root.h"

#include "System.h"
#include "Utils.h"
#include "common/Configuration.h"
#include "common/Logger.h"
#include "network/Server.h"

Root::Root() : m_restApi(), m_pluginManager(nullptr), m_block(nullptr), m_buildMode(nullptr), m_command(nullptr),
               m_customBlocks(nullptr),
               m_rank(nullptr),
               m_watchdog(nullptr),
               m_mapMain(nullptr),
               m_config(nullptr),
               m_playerList(nullptr),
               m_playerMain(nullptr),
               m_heartbeat(nullptr) {
}

Root::~Root() {
}

void Root::Start() {

    Logger::LogAdd("Root", "Initializing Modules...", DEBUG, GLF);

    m_config = Configuration::GetInstance();

    this->m_block = Block::GetInstance();
    this->m_command = CommandMain::GetInstance();
    this->m_rank = Rank::GetInstance();
    this->m_watchdog = watchdog::GetInstance();
    this->m_buildMode = BuildModeMain::GetInstance();
    this->m_customBlocks = CustomBlocks::GetInstance();
    this->m_mapMain = D3PP::world::MapMain::GetInstance();
    m_pluginManager = D3PP::plugins::PluginManager::GetInstance();

    Logger::LogAdd("Root", "Modules loaded.", DEBUG, GLF);

    Logger::LogAdd("Main", "Running Init Tasks...", NORMAL, GLF);
    TaskScheduler::RunSetupTasks();

    System::IsRunning = true;
    System::startTime = time(nullptr);
    System::ServerName = "D3PP Beta v" + stringulate(SYSTEM_VERSION_NUMBER);

    Logger::LogAdd("Root", "Starting networking.", DEBUG, GLF);
    D3PP::network::Server::Start();
    Logger::LogAdd("Root", "Done", DEBUG, GLF);

    m_pluginManager->LoadPlugins();
}

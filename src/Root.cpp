#include "Root.h"

#include "System.h"
#include "Utils.h"
#include "common/Configuration.h"
#include "common/Logger.h"
#include "network/Server.h"
#include "common/TaskScheduler.h"

Root::Root() = default;

Root::~Root() = default;

void Root::Start() {

    Logger::LogAdd("Root", "Initializing Modules...", DEBUG, GLF);

    m_config = std::make_unique<Configuration>();
    Configuration::Instance = m_config.get();

    m_block = std::make_unique<Block>();
    Block::Instance = m_block.get();

    m_command = std::make_unique<CommandMain>();
    CommandMain::Instance = m_command.get();

    m_rank = std::make_unique<Rank>();
    Rank::Instance = m_rank.get();

    m_watchdog = std::make_unique<watchdog>();
    watchdog::singleton_ = m_watchdog.get();

    m_buildMode = std::make_unique<BuildModeMain>();
    BuildModeMain::Instance = m_buildMode.get();

    m_customBlocks = std::make_unique<CustomBlocks>();
    CustomBlocks::Instance = m_customBlocks.get();

    m_mapMain = std::make_unique<D3PP::world::MapMain>();
    D3PP::world::MapMain::Instance = m_mapMain.get();

    m_playerMain = std::make_unique<PlayerMain>();
    PlayerMain::Instance = m_playerMain.get();

    m_heartbeat = std::make_unique<Heartbeat>();
    Heartbeat::Instance = m_heartbeat.get();

    m_playerList = std::make_unique<Player_List>();
    Player_List::Instance = m_playerList.get();

    m_pluginManager = std::make_unique<D3PP::plugins::PluginManager>();
    D3PP::plugins::PluginManager::Instance = m_pluginManager.get();

    Logger::LogAdd("Root", "Modules loaded.", DEBUG, GLF);

    Logger::LogAdd("Main", "Running Init Tasks...", NORMAL, GLF);
    TaskScheduler::InitializeMonitoringTask();
    TaskScheduler::RunSetupTasks();

    System::IsRunning = true;
    System::startTime = time(nullptr);
    System::ServerName = "D3PP Beta v" + stringulate(SYSTEM_VERSION_NUMBER);

    Logger::LogAdd("Root", "Starting networking.", DEBUG, GLF);
    D3PP::network::Server::Start();
    Logger::LogAdd("Root", "Done", DEBUG, GLF);

    if (m_heartbeat) {
        m_heartbeat->Start();
    }

    m_pluginManager->LoadPlugins();
}

void Root::Stop() {
    Logger::LogAdd("Root", "Running Teardown Tasks...", NORMAL, GLF);

    if (m_heartbeat) {
        m_heartbeat->Stop();
    }

    TaskScheduler::RunTeardownTasks();

    m_pluginManager.reset(); D3PP::plugins::PluginManager::Instance = nullptr;
    m_playerList.reset();    Player_List::Instance = nullptr;
    m_playerMain.reset();    PlayerMain::Instance = nullptr;
    m_heartbeat.reset();     Heartbeat::Instance = nullptr;
    m_mapMain.reset();       D3PP::world::MapMain::Instance = nullptr;
    m_customBlocks.reset();  CustomBlocks::Instance = nullptr;
    m_buildMode.reset();     BuildModeMain::Instance = nullptr;
    m_rank.reset();          Rank::Instance = nullptr;
    m_command.reset();       CommandMain::Instance = nullptr;
    m_block.reset();         Block::Instance = nullptr;
    m_watchdog.reset();      watchdog::singleton_ = nullptr;
    m_config.reset();        Configuration::Instance = nullptr;

    D3PP::network::Server::Stop();
    System::IsRunning = false;
    Logger::LogAdd("Root", "Done", DEBUG, GLF);
}
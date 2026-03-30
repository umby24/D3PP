#ifndef D3PP_ROOT
#define D3PP_ROOT

#include <memory>

#include "Block.h"
#include "BuildMode.h"
#include "Command.h"
#include "CustomBlocks.h"
#include "Rank.h"
#include "watchdog.h"
#include "common/Configuration.h"
#include "common/Player_List.h"
#include "plugins/Heartbeat.h"
#include "plugins/RestApi.h"
#include "plugins/PluginManager.h"
#include "world/MapMain.h"
#include "world/Player.h"

class Root {
public:
	Root();
	~Root();

	void Start();
	void Stop();
	static void Restart();

	/** Returns the amount of virtual RAM used, in KiB. Returns a negative number on error */
	static int GetVirtualRAMUsage(void);

	/** Returns the amount of virtual RAM used, in KiB. Returns a negative number on error */
	static int GetPhysicalRAMUsage(void);
private:
	RestApi m_restApi;
	std::unique_ptr<Configuration> m_config;
	std::unique_ptr<Block> m_block;
	std::unique_ptr<CommandMain> m_command;
	std::unique_ptr<Rank> m_rank;
	std::unique_ptr<watchdog> m_watchdog;
	std::unique_ptr<BuildModeMain> m_buildMode;
	std::unique_ptr<CustomBlocks> m_customBlocks;
	std::unique_ptr<D3PP::world::MapMain> m_mapMain;
	std::unique_ptr<PlayerMain> m_playerMain;
	std::unique_ptr<Heartbeat> m_heartbeat;
	std::unique_ptr<Player_List> m_playerList;
	std::unique_ptr<D3PP::plugins::PluginManager> m_pluginManager;

	void StartWorlds();
};

#endif // !D3PP_ROOT
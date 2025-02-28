#ifndef D3PP_ROOT
#define D3PP_ROOT

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
	static void Stop();
	static void Restart();

	/** Returns the amount of virtual RAM used, in KiB. Returns a negative number on error */
	static int GetVirtualRAMUsage(void);

	/** Returns the amount of virtual RAM used, in KiB. Returns a negative number on error */
	static int GetPhysicalRAMUsage(void);
private:
	RestApi m_restApi;
	D3PP::plugins::PluginManager* m_pluginManager;
	Block* m_block;
	BuildModeMain* m_buildMode;
	CommandMain* m_command;
	CustomBlocks* m_customBlocks;
	Rank* m_rank;
	watchdog* m_watchdog;
	D3PP::world::MapMain* m_mapMain;
	Configuration* m_config;
	Player_List* m_playerList;
	PlayerMain* m_playerMain;
	Heartbeat* m_heartbeat;

	void StartWorlds();
};

#endif // !D3PP_ROOT
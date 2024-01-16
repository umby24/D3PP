#ifndef D3PP_ROOT
#define D3PP_ROOT

#include "plugins/RestApi.h"
#include "plugins/PluginManager.h"

class Root {
public:
	Root(void);
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
	D3PP::plugins::PluginManager m_pluginManager;

	void StartWorlds();
};

#endif // !D3PP_ROOT
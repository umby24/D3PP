# D3PP Lua API

D3PP Runs Lua version 5.4, and supports all features of it. LuaJIT functions are not supported.

The server provided functions are broken into modules based on what they are controlling.

Server scripts are now broken into plugins. Each folder inside of the 'Plugins' directory will be considered its own plugin.
Function names can repeat between plugins as each one is separate from the other.

If you add a new plugin while the server is still running, you will need to run '/preload' from the console.

Once a plugin is loaded, files will automatically be reloaded whenever they are modified.
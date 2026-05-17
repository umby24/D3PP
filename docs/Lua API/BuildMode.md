# Lua BuildMode Module

A BuildMode is a state where a player is constructing or providing the details needed to perform a build operation on an area.

While in a non-normal build mode, block changes are local only (they do not actually alter the map).

Each player has 5 build variable slots, indexed 0–4. Each slot can hold one coordinate (X, Y, Z), one integer ("long"), one float, and one string independently.

`State` lets you track which step the player is at in a multi-step build process. Build variables retain their values when the state changes. All build variables are cleared when build mode is reset to normal.

## BuildMode.create(name, plugin)
Registers a new named build mode. `plugin` is the plugin that owns this mode (used for scoping).

## BuildMode.set(clientId, buildmode)
Sets the current build mode of the given client.

## BuildMode.get(clientId)
Returns the current build mode name of the given client, or `""` if none.

## BuildMode.setstate(clientId, value)
Sets the integer state of the client's current build mode.

## BuildMode.getstate(clientId)
Returns the state integer of the client's build mode. Returns -1 if no build mode is active.

## BuildMode.setcoordinate(clientId, index, x, y, z)
Sets the stored coordinate at `index` for the given client's build mode.

## BuildMode.getcoordinate(clientId, index)
Returns three values (x, y, z) for the stored coordinate at `index`. Returns -1, -1, -1 if not set.

## BuildMode.setlong(clientId, index, value)
Sets the integer value at `index` in the client's build mode storage.

## BuildMode.getlong(clientId, index)
Returns the integer value at `index` in the client's build mode storage. Returns -1 if not set.

## BuildMode.setfloat(clientId, index, value)
Sets the float value at `index` in the client's build mode storage.

## BuildMode.getfloat(clientId, index)
Returns the float value at `index` in the client's build mode storage. Returns -1 if not set.

## BuildMode.setstring(clientId, index, value)
Sets the string value at `index` in the client's build mode storage.

## BuildMode.getstring(clientId, index)
Returns the string value at `index` in the client's build mode storage.
# Lua Map Module

## Map.getall()
Returns a Lua table (array) of all loaded map IDs, plus the count as a second return value.

## Map.setblock(playerNumber, mapId, x, y, z, type, undo, physics, send, priority)
Places a block on the map, attributed to `playerNumber` (-1 for no player). `undo` and `physics` are booleans (0/1). `send` controls whether the change is sent to clients. `priority` is the block-change queue priority.

## Map.setblockclient(clientId, mapId, x, y, z, mode, type)
Processes a client-originated block change (as if the client placed/deleted the block). `mode` is 0 for delete, 1 for place.

## Map.setblockplayer(playerId, mapId, x, y, z, type, undo, physics, send, priority)
Places a block attributed to the given player number. Works like `Map.setblock` but validates against the player list.

## Map.moveblock(mapId, x0, y0, z0, x1, y1, z1, undo, physics, priority)
Copies the block at (x0, y0, z0) to (x1, y1, z1) and clears the source position.

## Map.getblock(mapId, x, y, z)
Returns the block type ID at the specified coordinates. Returns -1 if the map is not found.

## Map.getrank(mapId, x, y, z)
Returns the rank associated with the block at the given coordinates (from the per-block rank overlay). Returns -1 if not found.

## Map.getplayer(mapId, x, y, z)
Returns the player number of the last player who changed the block at the given coordinates. Returns -1 if unknown.

## Map.name(mapId)
Returns the name string of the map.

## Map.uuid(mapId)
**Deprecated.** Always returns the string `"Deprecated"`.

## Map.directory(mapId)
Returns the file path of the map's save file.

## Map.buildrank(mapId)
Returns the minimum rank required to build on this map.

## Map.showrank(mapId)
Returns the minimum rank required to see this map in the map list.

## Map.joinrank(mapId)
Returns the minimum rank required to join this map.

## Map.size(mapId)
Returns three values: the width (X), height (Y), and depth (Z) of the map.

## Map.spawn(mapId)
Returns five values: the spawn X, Y, Z coordinates, rotation, and look of the map.

## Map.saveinterval(mapId)
Returns the save interval for the map in seconds. (Currently always returns 10.)

## Map.setname(mapId, name)
Sets the name of the map. *(Currently a no-op in this version.)*

## Map.setdirectory(mapId, directory)
Sets the file path used when saving the map.

## Map.setbuildrank(mapId, rank)
Sets the minimum rank required to build on this map.

## Map.setjoinrank(mapId, rank)
Sets the minimum rank required to join this map.

## Map.setshowrank(mapId, rank)
Sets the minimum rank required for this map to appear in the map list.

## Map.setspawn(mapId, x, y, z, rotation, look)
Sets the spawn position and orientation of the map.

## Map.setsaveinterval(mapId, interval)
Sets the save interval for the map. *(Currently a no-op in this version.)*

## Map.add(mapId, sizeX, sizeY, sizeZ, name)
Creates a new map with the given ID, dimensions, and name. Returns the actual map ID assigned.

## Map.load(mapId, directory)
Loads map data for `mapId` from the given file path. The load is queued as a map action.

## Map.resize(mapId, sizeX, sizeY, sizeZ)
Queues a resize action that changes the dimensions of the map.

## Map.save(mapId, directory)
Queues a save action that writes the map to the given file path.

## Map.fill(mapId, functionName, argumentString)
Queues a fill action. `functionName` identifies the fill generator; `argumentString` is passed to it as configuration.

## Map.delete(mapId)
Queues a delete action that removes the map from the server.

## Map.resend(mapId)
Immediately resends the full map to all clients currently on it.

## Map.export(mapId, x0, y0, z0, x1, y1, z1, filename)
Exports the block region between the two corners to a file.

## Map.exportsize(filename)
Returns three values — the X, Y, Z dimensions — of a previously exported map file.

## Map.import(playerNumber, filename, mapId, x, y, z, scaleX, scaleY, scaleZ)
Imports a previously exported block region into `mapId` at the given position, scaled by the provided factors. Attributed to `playerNumber`.

## Map.beginfill(mapId)
Starts a bulk fill session for the map. Creates a temporary in-memory block array so that individual block assignments bypass the usual physics, block-change queues, and rank checks. Returns 1 on success, 0 if a fill session is already active.

## Map.getfillblock(mapId, x, y, z)
Returns the block type at (x, y, z) in the active fill buffer. Returns 0 if no fill session is active.

## Map.setfillblock(mapId, x, y, z, type)
Sets a block type in the active fill buffer. Returns 1 on success, 0 if no fill session is active.

## Map.endfill(mapId)
Ends the fill session and atomically replaces the map's block data with the fill buffer contents. Returns 1 on success, 0 if no fill session was active.

## Map.fillflat(mapId)
Applies the built-in flatgrass generator to the given map.

## Map.createParticle(mapId, effectId, U1, V1, U2, V2, redTint, blueTint, greenTint, frameCount, particleCount, size, sizeVariation, spread, speed, gravity, baseLifetime, lifetimeVariation, collideFlags, fullBright)
Defines a custom particle effect on the given map. See the [CustomParticles CPE spec](https://wiki.vg/Classic_Protocol_Extension#CustomParticles) for field descriptions. Returns `true` on success.

## Map.deleteParticle(mapId, particleId)
Removes a previously defined custom particle from the map.

## Map.spawnParticle(mapId, particleId, originX, originY, originZ, positionX, positionY, positionZ)
Spawns instances of a defined particle on the map. `origin` is where the effect appears; `position` is the point particles move away from.

## Map.setProperty(mapId, propertyId, value)
Sets a numeric map-environment property. Property IDs:
* 0 = Side block ID
* 1 = Edge block ID
* 2 = Side level
* 3 = Cloud height
* 4 = Max fog distance
* 5 = Cloud speed
* 6 = Weather speed
* 7 = Weather fade
* 8 = Exponential fog (0/1)
* 9 = Map side offset

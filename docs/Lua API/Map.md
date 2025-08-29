# Lua Map Module

## Map.getall()
Returns a table of all loaded maps.

## Map.setblock(mapId, x, y, z, type, undo, physic, priority, clientId, playerId)
Sets a block at the specified coordinates on the map.

## Map.setblockclient(mapId, x, y, z, type, priority, clientId)
Sets a block at the specified coordinates for a specific client.

## Map.setblockplayer(mapId, x, y, z, type, undo, physic, priority, playerId, clientId)
Sets a block at the specified coordinates for a specific player.

## Map.moveblock(mapId, x0, y0, z0, x1, y1, z1, undo, physic, priority)
Moves a block from one location to another on the map.

## Map.getblock(mapId, x, y, z, clientId)
Gets the block type at the specified coordinates for a client.

## Map.getrank(mapId, x, y, z, clientId)
Gets the rank required to modify a block at the specified coordinates.

## Map.getplayer(mapId, x, y, z, clientId)
Gets the player ID associated with a block at the specified coordinates.

## Map.name(mapId)
Returns the name of the map.

## Map.uuid(mapId)
Returns the unique identifier (UUID) of the map.

## Map.directory(mapId)
Returns the directory path of the map.

## Map.buildrank(mapId)
Gets the rank required to build on the map.

## Map.showrank(mapId)
Gets the rank required to see the map in /maps.

## Map.joinrank(mapId)
Gets the rank required to join the map.

## Map.size(mapId)
Returns the dimensions (width, height, depth) of the map.

## Map.spawn(mapId)
Returns the spawn coordinates of the map.

## Map.saveinterval(mapId)
Gets the save interval (in seconds) for the map.

## Map.setname(mapId, name)
Sets the name of the map.

## Map.setdirectory(mapId, directory)
Sets the directory path for the map.

## Map.setbuildrank(mapId, rank)
Sets the build rank for the map.

## Map.setjoinrank(mapId, rank)
Sets the join rank for the map.

## Map.setshowrank(mapId, rank)
Sets the show rank for the map.

## Map.setspawn(mapId, x, y, z, yaw, pitch)
Sets the spawn location and orientation for the map.

## Map.setsaveinterval(mapId, interval)
Sets the save interval (in seconds) for the map.

## Map.add(name, directory, width, height, depth)
Adds a new map with the specified parameters.

## Map.load(mapId, filename)
Loads a map from a file.

## Map.resize(mapId, width, height, depth)
Resizes the map to the specified dimensions.

## Map.save(mapId, filename)
Saves the map to a file.

## Map.fill(mapId, type, replaceType)
Fills the map with a specific block type, optionally replacing another type.

## Map.delete(mapId)
Deletes the specified map.

## Map.resend(mapId)
Resends the map data to all clients.

## Map.export(mapId, x, y, z, width, height, depth, filename)
Exports a region of the map to a file.

## Map.import(mapId, x, y, z, width, height, depth, filename, offset)
Imports a region from a file into the map.

## Map.exportsize(mapId)
Returns the size of the last exported region.

## Map.beginfill(mapId)
Starts a mapfill on a map. This creates a temporary in-memory array to assign your blocks to, for speed purposes. Avoids the usual overhead of physics, blockchange queues, block type lookups, etc.

## Map.getfillblock(mapId, x, y, z)
Get the block ID at a specific location inside the temporary fill array.

## Map.setfillblock(mapId, x, y, z, type)
Sets a block inside the temporary mapfill array.

## Map.endfill(mapId)
Ends the mapfill and replaces the current map contents with the contents of the in-memory id.

## Map.fillflat(mapId)
Generates a flatgrass map on the given map.

## Map.createParticle(mapId, effectId, U1, V1, U2, V2, redTint, greenTint, blueTint, frameCount, particleCount, size, sizeVariation, spread, speed, gravity, baseLifetime, lifetimeVariation, collideFlags, fullBright)
Defines a custom particle on a given map. See [here](https://wiki.vg/Classic_Protocol_Extension#CustomParticles) for each of the fields descriptions.

## Map.deleteParticle(mapId, particleId)
Deletes a previously defined particle.

## Map.spawnParticle(mapId, particleId, originX, originY, originZ, positionX, positionY, positionZ)
Spawns a previously defined particle on the specified map. Origin is where the effect will appear. Position is the location where the particles will move away from.
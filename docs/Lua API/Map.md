# Lua Map Module

## Map.getall
## Map.setblock
## Map.setblockclient
## Map.setblockplayer
## Map.moveblock
## Map.getblock
## Map.getrank
## Map.getplayer
## Map.name
## Map.uuid
## Map.directory
## Map.buildrank
## Map.showrank
## Map.joinrank
## Map.size
## Map.spawn
## Map.saveinterval
## Map.setname
## Map.setdirectory
## Map.setbuildrank
## Map.setjoinrank
## Map.setshowrank
## Map.setspawn
## Map.setsaveinterval
## Map.add
## Map.load
## Map.resize
## Map.save
## Map.fill
## Map.delete
## Map.resend
## Map.export
## Map.import
## Map.exportsize
## Map.beginfill(mapId)
Starts a mapfill on a map. This creates a temporary in-memory array to assign your blocks to, for speed purposes.

Avoids the usual overhead of physics, blockchange queues, block type lookups, etc.

## Map.getfillblock(mapId, x, y, z)
Get the block ID at a specific location inside the temporary fill array
## Map.setfillblock(mapId, x, y, z, type)
Sets a block inside the temporary mapfill array.
## Map.endfill(mapId)
Ends the mapfill and replaces the current map contents with the contents of the in-memory id.

## Map.fillflat(mapId)
Generates a flatgrass map on the given map.

## Map.createParticle(mapId, effectId, U1, V1, U2, V2, redTint, greenTint, blueTint, frameCount, particleCount, size, sizeVariation, spread, speed, gravity, baseLifetime, lifetimeVariation, collideFlags, fullBright)
See [here](https://wiki.vg/Classic_Protocol_Extension#CustomParticles) for each of the fields descriptions.

Defines a custom particle on a given map. 
## Map.deleteParticle(mapId, particleId)
Deletes a previously defined particle.
## Map.spawnParticle(mapId, particleId, originX, originY, originZ, positionX, positionY, positionZ)
Spawns a previously defined particle on the specified map.

Origin is where the effect will appear

Position is the location where the particles will move away from.
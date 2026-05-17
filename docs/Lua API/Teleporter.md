# Lua Teleporter Module

Teleporters are named cuboid regions on a map. When an entity enters the region it is moved to the destination.

## Teleporter.getall(mapId)
Returns a Lua table (array) of the names of all teleporters on the given map, plus the count as a second return value.

## Teleporter.add(mapId, name, startX, startY, startZ, endX, endY, endZ, destMapName, destX, destY, destZ, destRotation, destLook)
Creates a teleporter on `mapId`. The region is the cuboid between (startX, startY, startZ) and (endX, endY, endZ). `destMapName` is the name string of the destination map. The destination coordinates and orientation are in block units.

## Teleporter.delete(mapId, name)
Removes the named teleporter from the given map.

## Teleporter.getlocation(mapId, name)
Returns six values defining the teleporter region: `startX, startY, startZ, endX, endY, endZ` (in block coordinates). Returns nothing if the teleporter does not exist.

## Teleporter.getdestination(mapId, name)
Returns seven values for the teleporter's destination: `destMapName, id, destX, destY, destZ, rotation, look`. `destMapName` is a string; `id` is deprecated and always -1; the coordinates and orientation are in block units. Returns nothing if the teleporter does not exist.

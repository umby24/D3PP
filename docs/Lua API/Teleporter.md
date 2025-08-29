# Lua Teleporter Module

## Teleporter.getall()
Returns a table of all defined teleporters.

## Teleporter.add(mapId, name, x1, y1, z1, x2, y2, z2, destMapId, destX, destY, destZ, destYaw, destPitch)
Adds a new teleporter region to the specified map.  
- `mapId`: The ID of the map where the teleporter is placed.  
- `name`: The name of the teleporter.  
- `x1, y1, z1, x2, y2, z2`: The coordinates defining the teleporter region (cuboid).  
- `destMapId`: The ID of the destination map.  
- `destX, destY, destZ`: The destination coordinates.  
- `destYaw, destPitch`: The orientation at the destination.

## Teleporter.delete(mapId, name)
Deletes the teleporter with the specified name from the given map.  
- `mapId`: The ID of the map.  
- `name`: The name of the teleporter to delete.

## Teleporter.getlocation(mapId, name)
Returns the region coordinates for the specified teleporter on the given map.  
- `mapId`: The ID of the map.  
- `name`: The name of the teleporter.

## Teleporter.getdestination(mapId, name)
Returns the destination map and coordinates for the specified teleporter.  
- `mapId`: The ID of the map.  
- `name`: The name of the teleporter.
# Lua Entity Module

## Entity.getall()
Returns a Lua table (array) of all entity IDs currently on the server, plus the count as a second return value.

## Entity.create(name, mapId, x, y, z, rotation, look)
Spawns a fake-player entity on the given map at the given position. Returns the new Entity ID.

## Entity.delete(entityId)
Deletes the given entity. Cannot be used to delete an entity that belongs to a connected player.

## Entity.getplayer(entityId)
Returns the Player_Number of the player associated with the given entity. Returns -1 if the entity is not a player.

## Entity.getmap(entityId)
Returns the ID of the map the entity is currently on. Returns -1 if not found.

## Entity.getposition(entityId)
Returns three values: the X, Y, Z coordinates of the entity (in block units). Returns -1, -1, -1 if not found.

## Entity.getrotation(entityId)
Returns the yaw (rotation) of the given entity. Returns -1 if not found.

## Entity.getlook(entityId)
Returns the pitch (look) of the given entity. Returns -1 if not found.

## Entity.resend(entityId)
Despawns and immediately respawns the given entity for all clients who can see it.

## Entity.sendmessage(entityId, message)
Sends a chat message to all clients associated with the given entity ID.

## Entity.getdisplayname(entityId)
Returns three values: the prefix, name, and suffix strings of the entity's display name.

## Entity.setdisplayname(entityId, name)
## Entity.setdisplayname(entityId, prefix, name, suffix)
Sets the display name (and optional prefix/suffix) of the entity and respawns it. Does not affect the tab list.

## Entity.setposition(entityId, mapId, x, y, z, rotation, look)
Teleports the entity to the specified position, map, and orientation.

## Entity.kill(entityId)
Kills the entity: sends it to the map's spawn point and broadcasts a `"[name] died"` message.

## Entity.setmodel(entityId, model)
Sets the CPE entity model of the given entity (e.g. `"humanoid"`, `"chicken"`).
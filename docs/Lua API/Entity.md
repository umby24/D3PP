# Lua Entity Module

## Entity.getall()
Returns a Lua table of all connected entities.
## Entity.create(Name, Map_ID, X,Y,Z, Rot, Look)
Creates a fake player on the map at the given location. Returns the Entity_ID of the fake client.
## Entity.delete(Entity_ID)
Deletes the given entity.
## Entity.getplayer(Entity_ID)
Returns the Player_Number of the given Entity. -1 if none found.
## Entity.getmap(Entity_ID)
Returns the ID of the map that the entity is currently on.
## Entity.getposition(Entity_ID)
Returns 3 variables, the X,Y,Z location of the given entity.
## Entity.getrotation(Entity_ID)
Returns the rotation of the given entity
## Entity.getlook(Entity_ID)
Returns the look of the given entity
## Entity.resend(Entity_ID)
Despawns and respawns the given entity Id
## Entity.sendmessage(Entity_ID, Message)
Sends a message to the clients associated with the given entity id
## Entity.getdisplayname(Entity_ID)
Returns 3 variables, the prefix, name, and suffix of the given entity.
## Entity.setdisplayname(Entity_ID, prefix, name, suffix)
Sets the display name of a given entity and respawns them. Does not effect the tab list.
## Entity.setposition(Entity_ID, Map_ID, X, Y, Z, Rot, Look,Priority,Own)
Sets the location of the given entity to the values provided.
## Entity.kill(Entity_ID)
Kills the given entity. (Sends them to spawn and displays "[name] died" message)
## Entity.setmodel(Entity_ID, Model)
Sets the CPE Model of the given entity Id.
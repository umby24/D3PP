# Lua Client Module

## Client.getall()
Returns a Lua table (array) of all connected client IDs, plus the count as a second return value.

## Client.getmap(clientId)
Returns the ID of the map the client is currently on. Returns -1 if not logged in or not found.

## Client.getip(clientId)
Returns the IP address string of the given client.

## Client.getloginname(clientId)
Returns the login name of the given client (no colour codes).

## Client.isloggedin(clientId)
Returns the logged-in status of the client as an integer (1 if logged in, 0 otherwise, -1 if not found).

## Client.getentity(clientId)
Returns the Entity ID associated with the given client. Returns -1 if not found or not logged in.

## Client.kick(clientId, reason)
Kicks the given client from the server. `reason` is optional and defaults to `"Kicked by script"`.
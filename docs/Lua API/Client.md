#Lua client module

## Client.getall()
Returns a lua table containing all connected clients.
## Client.getmap(Client_ID)
Returns the ID of the map that the client is currently on.
## Client.getip(Client_ID)
Returns the IP of the given client.
## Client.getloginname(Client_ID)
Returns the Login Name of the given client. (Does not include coloring)
## Client.isloggedin(Client_ID)
If the client given is online, returns their Client_ID, else, returns -1.
## Client.getentity(Client_ID)
Returns the client's Entity ID. If not found, returns -1.
# Lua BuildMode Module
A BuildMode is a state where a player is constructing or providing details needed to perform a build
operation on an area.

While in a non-normal build mode, block changes are local only.

In a build mode, each player has 5 different build variables. You can pick which build variable using the index field when getting a build variable.

Each build variable index can hold 1 coordinate, 1 long, 1 float, and 1 string.

State is for you to track what step the player is at in the process. When changing state, build variables retain their value.

When resetting build mode to normal, all build variables are lost.

## BuildMode.set(Client_ID, Buildmode)
Sets the current buildmode of the given client.
## BuildMode.get(Client_ID)
Returns the current buildmode of the given client.
## BuildMode.setstate(Client_ID, Value)
Sets the value (int) of the client's current build mode.
## BuildMode.getstate(Client_ID)
Returns the state of the client's build mode. -1 if none.
## BuildMode.setcoordinate(Client_ID, Index, Value)
Sets the X,Y,Z Coordinate of the client's build mode.
## BuildMode.getcoordinate(Client_ID, Index)
Returns X,Y,Z coordinate of the client's build mode. -1 if none.
## BuildMode.getlong(Client_ID, Index)
Returns the Long Value of the client's build mode.
## BuildMode.setlong(Client_ID, Index, Value)
Sets the Long Value of the client's build mode.
## BuildMode.setfloat(Client_ID, Index, Value)
Sets the Float Value of the client's build mode.
## BuildMode.getfloat(Client_ID, Index)
Gets the Float Value of the client's build mode.
## BuildMode.getstring(Client_ID, Index)
ets the String value of the client's build mode.
## BuildMode.setstring(Client_ID, Index, Value)
Sets the String Value of the client's build mode.
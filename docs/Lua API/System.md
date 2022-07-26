# Lua System Module

## System.msgAll(Map_ID, Message)
## System.msgAll(Map_ID, Message, messageType)
Send the given message to the given map. Map_ID of -1 will send to all maps.
## System.msg(Client_ID, Message)
## System.msgAll(Map_ID, Message, messageType)
Sends the given message to a specific client.

If MessageType is specified, the message will be sent to the corresponding location on the client's screen.
* 0 = Chat
* 1 = Top Right
* 2 = Top Middle Right
* 3 = Top Bottom Right
* 11 = Bottom Right
* 12 = Bottom Middle Right
* 13 = Bottom top right
* 100 = Announcement


## System.getfile(FileAlias)
Returns the full filepath of the given file alias, from files.json
## System.getfolder(FolderAlias)
Returns the full filepath of the given folder alias, from files.json.
## System.addEvent(EventId, Function, Type, SetOrCheck, Timer, MapId)
Creates a lua event hook. EventID is for your reference for deleting it later.

Function is a string with the name of the lua function that will be called when the event is triggered.

Type is a string of the event you wish to hook, see below for valid values.

Set or check: 0 = Check, 1 = Set.   if 0, the event will not be registered, but the function will return true if the function already exists, otherwise false.
If 1, the event will be registered regardless.

Timer is the value in milliseconds your callback function will be called at, if registering a timer event.

MapID is if you only wish to receive this event from a certain map ID. -1 gives you all events globally.

Valid event types:
* Chat_All(Result, mapId, Message)
  * Triggered when a chat message is going to be sent to all players on a map or server wide. Cancelable, return '0' to cancel.
* Chat_Map(Result, mapid, Message)
  * Triggered when a chat message is going to be sent to all players on a map. Cancelable, return '0' to cancel.
* Chat_Private
  * Triggered when one player is sending a chat message to another.
* Client_Add(Result, clientId)
  * Triggered when a client is connecting to the server. Can be triggered even if a client does not complete the handshake process.
* Client_Delete(Result, clientId)
  * Triggered when a client is disconnecting from the server.
* Client_Login(Result, clientId)
  * Triggered when a client has successfully logged in. (Completed server handshake and received a map, entity id, player id.)
* Client_Logout(Result, clientId)
  * Triggered when a client that is logged in leaves the server.
* Entity_Add(Result, entityId)
  * Triggered when an entity is created / spawned somewhere on the server. 
* Entity_Delete(Result, entityId)
  * Triggered when an entity is destroyed.
* Entity_Die(Result, entityId)
  * Triggered when an entity interacts with a block that can kill you, or is triggered by a script. Cancelable, return '0' to prevent the player from dying.
* Entity_Map_Change(EntityId, newMapId, oldMapId)
  * Triggered when an entity changes from one map to another.
* Entity_Position_Set(Result, EntityId, mapId, X, Y, Z, Rotation, Look, Priority, sendOwn)
  * Triggered whenever an entity moves.
* Map_Action_Delete(result, actionId, mapId)
  * Triggered before a map is deleted.
* Map_Action_Fill(result, actionId, mapId)
  * Triggered before a map is filled.
* Map_Action_Load(result, actionId, mapId)
  * Triggered when a map is loaded, by script or reload.
* Map_Action_Resize(result, actionId, mapId)
  * Triggered when a map is resized, by script or command.
* Map_Action_Save(result, actionId, mapId)
  * Triggered when a map is aved
* Map_Add(result, mapId)
  * Triggered when a map is created
* Map_Block_Change(result, playerId, mapId, x, y, z, blockType, undo, physics, send, priority)
  * Triggered when a block change is made to a map. Cancelable, return '0' to prevent the block change.
* Map_Block_Change_Client(result, clientId, mapId, x, y, z, mode, blockType)
  * Triggered when a client makes a block change (not by script). Cancelable; return '0' to prevent the block change **and resend the old block to the client trying to make the change**.
* Map_Block_Change_Player(result, playerNumber)
  * Not currently triggered
* Timer(mapId)
  * Triggers every x milliseconds, where x is a value you provide when registering the event.

## System.deleteEvent(EventId)
Unregisters this event hook.
## System.log(message)
## System.log(message, type)
Logs using the built in system logger.
Valid types are info, warning, error. Default is info.
## System.getplatform()
Returns 'linux' or 'windows' based on compiler args
## System.addCmd(name, group, minRank, function, description)
Name is the command, for example, for a command of `/potato` the value of name should be `potato`.

Group is the grouping in the command list to include this in. General, Map, OP, etc.

MinRank is the minimum rank to use this command

Function is a string, formatted as `Lua:CALLBACK_FUNCTION`

Description is the help text that you will see when calling `/cmdhelp yourcommand`

## System.setServerName(name)
Set the name of the server in the settings, also updates your heartbeat on the next beat.

## System.setSoftwareName(name)
Set the name of the server software; for use when creating large plugin packages modifying server functionality.
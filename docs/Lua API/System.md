# Lua System Module

## System.msgAll(Map_ID, Message)
Send the given message to the given map. Map_ID of -1 will send to all maps.
## System.msg(Client_ID, Message)
Sends the given message to a specific client.
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
* Chat_All
* Chat_Map
* Chat_Private
* Client_Add
* Client_Delete
* Client_Login
* Client_Logout
* Entity_Add
* Entity_Delete
* Entity_Die
* Entity_Map_Change
* Entity_Position_Set
* Map_Action_Delete
* Map_Action_Fill
* Map_Action_Load
* Map_Action_Resize
* Map_Action_Save
* Map_Add
* Map_Block_Change
* Map_Block_Change_Client
* Map_Block_Change_Player
* Timer

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
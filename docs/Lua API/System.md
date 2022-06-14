# Lua System Module

## System.msgAll
## System.msg
## System.getfile
## System.getfolder
## System.addEvent
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

## System.deleteEvent
## System.log(message)
## System.log(message, type)
Logs using the built in system logger.
Valid types are info, warning, error. Default is info.
## System.getplatform()
Returns 'linux' or 'windows' based on compiler args
## System.addCmd(name, group, minRank, function, description)
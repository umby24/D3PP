# Lua System Module

## System.msgAll(mapId, message)
## System.msgAll(mapId, message, messageType)
Sends `message` to all clients on the given map. Pass `-1` for `mapId` to send to every player on the server. `messageType` defaults to 0 (chat).

## System.msg(clientId, message)
## System.msg(clientId, message, messageType)
Sends `message` to a specific client. `clientId` of `-200` is a special broadcast value.

`messageType` controls where the message appears on the client's screen:
* 0 = Chat
* 1 = Top right
* 2 = Top middle-right
* 3 = Top bottom-right
* 11 = Bottom right
* 12 = Bottom middle-right
* 13 = Bottom top-right
* 100 = Announcement (centre of screen)

## System.getfile(fileAlias)
Returns the full file path for the given alias, as defined in `files.json`.

## System.getfolder(folderAlias)
Returns the full folder path for the given alias, as defined in `files.json`.

## System.addEvent(eventId, function, type, setOrCheck, timer, mapId)
Registers or checks for a Lua event hook.

- `eventId`: a string identifier of your choice, used to delete the event later.
- `function`: name of the Lua function to call when the event fires.
- `type`: event type string (see list below).
- `setOrCheck`: 1 = register the event; 0 = check only — returns `true` if the event already exists, `false` otherwise.
- `timer`: interval in milliseconds, used only for `Timer` events.
- `mapId`: restrict the event to a specific map ID. Pass -1 to receive events from all maps.

Valid event types and their callback signatures:
* `Chat_All(result, mapId, message)` — chat message broadcast to all on a map. Cancelable (return `0`).
* `Chat_Map(result, mapId, message)` — chat message broadcast to a map. Cancelable.
* `Chat_Private` — private chat between players.
* `Client_Add(result, clientId)` — client connecting (may not complete handshake).
* `Client_Delete(result, clientId)` — client disconnecting.
* `Client_Login(result, clientId)` — client fully logged in (map received, entity and player IDs assigned).
* `Client_Logout(result, clientId)` — logged-in client leaving.
* `Entity_Add(result, entityId)` — entity spawned.
* `Entity_Delete(result, entityId)` — entity destroyed.
* `Entity_Die(result, entityId)` — entity touched a kill block or was killed by script. Cancelable (return `0` to prevent death).
* `Entity_Map_Change(entityId, newMapId, oldMapId)` — entity moved between maps.
* `Entity_Position_Set(result, entityId, mapId, x, y, z, rotation, look, priority, sendOwn)` — entity moved.
* `Map_Action_Delete(result, actionId, mapId)` — map about to be deleted.
* `Map_Action_Fill(result, actionId, mapId)` — map about to be filled.
* `Map_Action_Load(result, actionId, mapId)` — map being loaded.
* `Map_Action_Resize(result, actionId, mapId)` — map being resized.
* `Map_Action_Save(result, actionId, mapId)` — map being saved.
* `Map_Add(result, mapId)` — map created.
* `Map_Block_Change(result, playerId, mapId, x, y, z, blockType, undo, physics, send, priority)` — block change queued. Cancelable.
* `Map_Block_Change_Client(result, clientId, mapId, x, y, z, mode, blockType)` — client-originated block change. Cancelable; canceling also resends the original block to the client.
* `Map_Block_Change_Player(result, playerNumber)` — not currently triggered.
* `Timer(mapId)` — fires every `timer` milliseconds.

## System.deleteEvent(eventId)
Unregisters the event hook with the given ID.

## System.log(message)
## System.log(message, type)
Writes to the server log. Valid `type` values are `"info"` (default), `"warning"`, and `"error"`.

## System.getplatform()
Returns `"linux"` or `"windows"` based on the build target.

## System.addCmd(name, group, minRank, function, description)
Registers a chat command.

- `name`: the command name without the leading `/` (e.g. `"potato"` for `/potato`).
- `group`: the category shown in the command list (e.g. `"General"`, `"Map"`, `"OP"`).
- `minRank`: minimum rank number required to use the command.
- `function`: name of the Lua callback function. The `"Lua:"` prefix is added automatically if omitted.
- `description`: help text shown by `/cmdhelp <name>`.

Re-registering an existing command name replaces it.

## System.setSoftwareName(name)
Sets the software name reported by the server. Useful for large plugin packages that fundamentally alter server behaviour.

## System.setServerName(name)
Sets the server's display name in the configuration and triggers an update on the next heartbeat.

## System.addTextColor(character, red, green, blue, alpha)
Registers a custom colour code. `character` must be a single character. RGB and alpha values are integers 0–255. Re-registering the same character replaces the existing entry.

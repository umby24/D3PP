# Lua CPE Module

## CPE.getextversion(clientId, extensionName)
Returns the version number of the named CPE extension that the client supports. Returns 0 if the client does not support the extension.

## CPE.getexts(clientId)
Returns a Lua table of the client's supported extensions in `[ExtensionName] = version` format, plus the count as a second return value.

## CPE.addselection(clientId, selectionId, label, startX, startY, startZ, endX, endY, endZ, red, green, blue, opacity)
Creates a selection cuboid visible to the given client. Selection IDs 250, 252, 253, and 254 are reserved by the server.

## CPE.deleteselection(clientId, selectionId)
Removes a selection cuboid from the given client.

## CPE.getheld(clientId)
Returns the block ID the client is currently holding.

## CPE.setheld(clientId, blockId, canChange)
Forces the client to hold `blockId`. If `canChange` is 0 the client cannot switch away from it.

## CPE.setenvcolors(mapId, red, green, blue, type)
Sets an environment colour channel for the given map. Pass -1 for red, green, and blue to reset a channel to the default.

`type` values:
* 0 = Sky
* 1 = Cloud
* 2 = Fog
* 3 = Ambient light
* 4 = Diffuse (sun) light

## CPE.setblockperms(clientId, blockId, canPlace, canDelete)
Overrides the placement and deletion permissions for `blockId` on the given client.

## CPE.setmapenv(mapId, textureUrl, sideBlockId, edgeBlockId, sideLevel)
Sets the custom texture URL and side/edge block IDs for the given map.

## CPE.setclienthacks(clientId, canFly, noClip, speeding, spawnControl, thirdPerson, jumpHeight)
Enables or disables specific hack flags and sets jump height for the given client.

## CPE.addhotkey(label, action, keycode, keymods)
Registers a text hotkey for clients. See the CPE specification for keycode and keymods values. *(Not yet fully implemented.)*

## CPE.removehotkey(label)
Removes a previously registered hotkey. *(Not yet fully implemented.)*

## CPE.setmaphacks(mapId, canFly, noClip, speeding, spawnControl, thirdPerson, jumpHeight)
Sets the default hack flags and jump height applied to all clients on the given map.

## CPE.setmodel(clientId, model)
Changes the entity model of the given client (e.g. `"humanoid"`, `"chicken"`).

## CPE.setweather(clientId, weatherType)
Sets the weather visible to the given client. Values: 0 = none, 1 = rain, 2 = snow.

## CPE.createblockdef(blockId, name, solidity, movementSpeed, topTexture, sideTexture, bottomTexture, transmitsLight, walkSound, fullBright, shape, drawType, fogDensity, fogR, fogG, fogB)
Creates a custom block definition server-wide (also saved in `Data/CustomBlocks.json`). `blockId` must be 1–255 and cannot be 0 (air).

### Solidity
 - 0 = Walk-through - block does not collide with the player, and does not interfere with jumping. Players fall through this block. (e.g. Air)
 - 1 = Swim-through - block allows the player to descend or ascend slowly (at WalkSpeed), as if they were swimming.
 - 2 = Solid - block collides with the player. Players may walk on this block. (e.g. Stone)
 - 3 = Partially slippery - same as Solid, but player slides if they are directly on top of this block (e.g. Ice)
 - 4 = Fully slippery - same as Solid, but player slides if they touch this block at all
 - 5 = Water - block has same collision behaviour as Water block in Minecraft Classic
 - 6 = Lava - block has same collision behaviour as Lava block in Minecraft Classic
 - 7 = Rope - block has rope/ladder climbing interaction (e.g. Rope)

### Speed
A value of 0 translates to 0.25 ratio (25% speed), 128 to 1.00 ratio (100% speed), and 255 to 3.96 (396% speed).

## CPE.deleteblockdef(blockId)
Removes a custom block definition server-wide.

## CPE.createclientblockdef(blockId, name, solidity, movementSpeed, topTexture, sideTexture, bottomTexture, transmitsLight, walkSound, fullBright, shape, drawType, fogDensity, fogR, fogG, fogB, clientId)
Sends a custom block definition to one specific client only; does not persist server-side.

## CPE.deleteclientblockdef(blockId, clientId)
Sends a block-definition removal packet to one specific client.

## CPE.setblockext(blockId, topTexture, leftTexture, rightTexture, frontTexture, backTexture, bottomTexture, minX, minY, minZ, maxX, maxY, maxZ)
Sets the per-face textures and bounding-box coordinates of an existing custom block definition, server-wide. The block must already exist via `CPE.createblockdef`.

## CPE.setblockextclient(blockId, topTexture, leftTexture, rightTexture, frontTexture, backTexture, bottomTexture, minX, minY, minZ, maxX, maxY, maxZ, clientId)
Same as `CPE.setblockext` but sends the update to one client only.

## CPE.setClientHotbar(clientId, hotbarIndex, blockId)
Places `blockId` into the given hotbar slot (0–8) for the specified client. Requires the client to support the `SetHotbar` extension.

## CPE.setClientInventoryOrder(clientId, order, blockId)
Sets the inventory display order for `blockId` to `order` for the specified client. Requires the client to support the `InventoryOrder` extension.
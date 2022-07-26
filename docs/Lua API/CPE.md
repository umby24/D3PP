# Lua CPE Module

## CPE.getextversion(Client_ID, ExtensionName)
Returns the version of the given extension this client supports. 0 for no support.
## CPE.getexts(Client_ID)
Returns a lua table of the client's supported extensions, in [Extension] = Version format.
## CPE.addselection(Client_ID, Selection_ID, Label, Start_X, Start_Y, Start_Z, End_X, End_Y, End_Z, Red, Green, Blue, Opacity)
Creates a Selection Cuboid on the client. Note that the following Selection_IDs are reserved for the server: 250, 252, 253, 254.
## CPE.deleteselection(Client_ID, Selection_ID)
Deletes a selection cuboid on a client.
## CPE.getheld(Client_ID)
Returns the currently held block-id of the client.
## CPE.setheld(Client_ID, Block_ID, CanChange)
Changes the clients held block. if CanChange = 0, client can change.
## CPE.setenvcolors(Map_ID, Red, Green, Blue, Type)
Sets the Environment colors of the given map to the values provided.
## CPE.setblockperms(Client_ID, Block_ID, Can_Place, Can_Delete)
Sets the permissions on Block_ID for the given client.
## CPE.setmapenv(Map_ID, Texture_URL, Side_Block_ID, Edge_Block_ID, Side_Level)
Sets the custom texture and side blocks
## CPE.setclienthacks(Client_ID, CanFly, CanNoClip, CanWalkFast, CanRespawn, CanUseThirdperson, CanSetWeather, Jumpheight)
Enables or disables certain hacks and jumpheight on the given client.
## CPE.addhotkey(Label, Action, Keycode, Keymods)
Sets a text hotkey for clients who log in. See the CPE page for more information.
## CPE.removehotkey(Label)
Removes a previously set hotkey from clients.
## CPE.setmaphacks(mapId, canFly, noclip, speeding, spawnControl, thirdPerson, jumpHeight)
Enables or disables certain hacks and jumpheight on the given map.
## CPE.setmodel(Client_ID, Model)
Changes the player model of the given Client_ID. Valid models are below.
## CPE.setweather(Client_ID, Weather_Type)
Sets the weather on the given client to the given type. 0 = No weather, 1 = Rain, 2 = Snow.
## CPE.createblockdef(id, name, solidity, walkSpeed, topTexture, sideTexture, bottomTexture, transmitsLight, walkSound, fullBright, shape, drawType, fogDensity, fogR, fogG, fogB)
Creates a custom block definition server-wide. Also saved in Data/CustomBlocks.json
## CPE.deleteblockdef(id)
Deletes a custom block definition.
## CPE.createclientblockdef(id, name, solidity, walkSpeed, topTexture, sideTexture, bottomTexture, transmitsLight, walkSound, fullBright, shape, drawType, fogDensity, fogR, fogG, fogB, clientId)
Creates a custom block definition on one client only.
## CPE.deleteclientblockdef(id)
Deletes a custom block definition on one client only.
## CPE.setblockext(blockId, topTexture, leftTexture, rightTexture, frontTexture, backTexture, bottomTexture, minx, miny, minz, maxX, maxY, maxZ)
Sets the extended options of a custom block, saved server-wide.
## CPE.setblockextclient(blockId, topTexture, leftTexture, rightTexture, frontTexture, backTexture, bottomTexture, minx, miny, minz, maxX, maxY, maxZ, clientId)
Sets the extended options of a custom block, client-local only.

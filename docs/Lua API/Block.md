# Lua 'Block' module

## Block.getall()
Returns a Lua table mapping block ID → `true` for every block defined in the server, plus the total count as a second return value.

## Block.name(blockId)
Returns the name of the given block ID.

## Block.placerank(blockId)
Returns the minimum rank number required to place this block.

## Block.deleterank(blockId)
Returns the minimum rank number required to delete this block.

## Block.setplacerank(blockId, rank)
Sets the minimum rank number that can place this block.

## Block.setdeleterank(blockId, rank)
Sets the minimum rank number that can delete this block.

## Block.setphysics(blockId, physics, onLoad, physPlugin, physRandom, repeat, physTime)
Sets the physics properties of the given block.

`physics` type can be 0, 10, 11, or 20:
* 0: no physics
* 10: block falls straight down
* 11: block falls down and into 45-degree bevels
* 20: Minecraft-style liquid physics

*If you are using Lua-based physics, set `physics` to 0.*

`onLoad`: 1 to trigger physics when the map is loaded, 0 otherwise.

`physPlugin`: string in the form `Lua:LUA_FUNCTION_NAME`. The function is triggered on block place, delete, and whenever physics repeats.

`physRandom`, `repeat`, `physTime`: after a block is placed the physics function fires `physTime + rand(0, physRandom)` milliseconds later. If `repeat` is 1 the function is queued to fire again after each trigger.

## Block.setkills(blockId, kills)
Sets whether a block kills the player on contact. Pass 1 to enable, 0 to disable.

## Block.clienttype(blockId)
Returns the block ID that will be sent to the client for this server-side block ID. For example a server block of 200 might be sent to the client as 49.

## Block.create(blockId, blockName, clientId)
## Block.create(blockId, blockName, clientId, physics, physicsPlugin, physicsTime, physicsRandom)
Creates or replaces a block definition. `blockId` must be ≥ 66 (vanilla blocks cannot be redefined). `clientId` is the block ID transmitted to clients. The optional physics arguments work the same as in `Block.setphysics`.
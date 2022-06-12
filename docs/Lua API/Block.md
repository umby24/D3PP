# Lua 'Block' module

## Block.getall

## Block.getall()
Returns a Lua Table containing every block defined in the server.
## Block.name(blockId)
Returns the name of the given block id.
## Block.placerank(blockId)
Returns the minimum rank to place this block id
## Block.deleterank
Returns the minimum rank to delete this block id
## Block.setplacerank(blockId, rank)
Sets the minimum rank number that can place this block
## Block.setdeleterank
Sets the minimum rank number that can delete this block
## Block.setphysics(blockId, physicsType, onLoad, physPlugin, physRandom, repeat, physTime)
Sets the physics properties of the given block.

physics type can be 0, 10, 11, or 20. 
* 0: no physics.
* 10: Block falls straight down
* 11: Block falls down and into 45 degree bevels
* 20: Minecraft original liquid physics

*If you are using a lua based physics, don't set the physics type!*

Onload can be 0 or 1, indicating if physics should activate when a map is loaded.
physPlugin should be `Lua:LUA_FUNCTION_NAME`.

When set, the function will be triggered on block place, delete, and whenever triggered by physics repeat.

physRandom, repeat, and physTime:

After a block is placed, the physics function will be triggered physTime + (Random(0, physRandom)) milliseconds later.
If repeat is true, after being triggered the lua function will be queued to trigger again.

## Block.setkills(blockId, kills)
Sets if a block kills the player on contact
## Block.clienttype(blockId, type)
Sets what the block ID sent to the client will be.

For example, the block id may be 200 to the server, but can be sent to the client as 49.

## Block.create(blockId, blockName, clientId)
## Block.create(blockId, blockName, clientId, physics, physicsPlugin, physicsTime, physicsRandom)
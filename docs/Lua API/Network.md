# Lua Network Module

## Network.setblock(clientId, x, y, z, blockType)
Sends a block-change packet to the given client only. This is a visual-only change: it does not alter the actual map data, and other clients will not see it.

# Lua Build Module

## Build.line(playerNumber, mapId, x0, y0, z0, x1, y1, z1, material, priority, undo, physics)
Draws a line of blocks between the two given points, attributed to `playerNumber` (-1 for no player).

## Build.box(playerNumber, mapId, x0, y0, z0, x1, y1, z1, material, replaceMaterial, hollow, priority, undo, physics)
Fills the cuboid between the two given points with `material`. `replaceMaterial` can be -1 (replace all) or a specific block ID to replace only that type. `hollow` is 1 for a hollow shell, 0 for solid.

## Build.sphere(playerNumber, mapId, x, y, z, radius, material, replaceMaterial, hollow, priority, undo, physics)
Creates a sphere centred at (x, y, z) with the given radius. `replaceMaterial` and `hollow` work the same as in `Build.box`.

## Build.rankbox(mapId, x0, y0, z0, x1, y1, z1, rank, maxRank)
Marks the cuboid region on `mapId` so that only players whose rank is between `rank` and `maxRank` (inclusive) may build within it.
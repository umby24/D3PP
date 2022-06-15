# Lua Build Module

## Build.line(playerNumber, mapId, x0, y0, z0, x1, y1, z1, material, replaceMaterial, hollow, priority, undo, physics)
Creates a line between the given points, made my the given player. -1 for no player.
## Build.box(playerNumber, mapId, x0, y0, z0, x1, y1, z1, material, replaceMaterial, hollow, priority, undo, physics)
Creates a box ("Cuboid") between the given points. Replace can be -1, or a material to be replaced. Hollow is 0 or 1.
## Build.sphere(playerNumber, mapId, x, y, z, radius, material, replaceMaterial, hollow, priority, undo, physics)
Creates a Sphere at the given point, of the given radius.
## Build.rankbox(mapId, x0, y0, z0, x1, y1, z1, rank, maxRank)
Creates an area where only the given ranks are permitted to build within the given map.
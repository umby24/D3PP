# Lua Player Module

Player IDs (also called player numbers) are persistent database identifiers, not the same as client IDs.

## Player.getall()
Returns a Lua table (array) of all player numbers in the player database, plus the count as a second return value.

## Player.getprefix(playerId)
Returns the rank prefix string for the specified player.

## Player.getname(playerId)
Returns the login name of the specified player.

## Player.getsuffix(playerId)
Returns the rank suffix string for the specified player.

## Player.ip(playerId)
Returns the last known IP address of the specified player.

## Player.rank(playerId)
Returns the rank number of the specified player.

## Player.online(playerId)
Returns 1 if the player is currently online, 0 otherwise.

## Player.ontime(playerId)
Returns the player's cumulative online time as a number (seconds).

## Player.mutetime(playerId)
Returns the remaining mute time for the player in minutes. -1 if not muted.

## Player.setrank(playerId, rank, reason)
Sets the rank of the specified player and records a reason string.

## Player.kick(playerId, reason)
Kicks the specified player from the server with the given reason.

## Player.ban(playerId, reason)
Bans the specified player with the given reason.

## Player.unban(playerId)
Removes the ban on the specified player.

## Player.stop(playerId, reason)
Freezes the specified player with the given reason (they cannot move).

## Player.unstop(playerId)
Unfreezes the specified player.

## Player.mute(playerId, minutes, reason)
Mutes the specified player for the given number of minutes with a reason.

## Player.unmute(playerId)
Removes the mute from the specified player.

# Lua Player Module

## Player.getall()
Returns a table of all connected players.

## Player.getprefix(playerId)
Returns the prefix string for the specified player.

## Player.getname(playerId)
Returns the name of the specified player.

## Player.getsuffix(playerId)
Returns the suffix string for the specified player.

## Player.ip(playerId)
Returns the IP address of the specified player.

## Player.rank(playerId)
Returns the rank of the specified player.

## Player.online(playerId)
Returns true if the specified player is online.

## Player.ontime(playerId)
Returns the total online time (in seconds) for the specified player.

## Player.mutetime(playerId)
Returns the remaining mute time (in seconds) for the specified player.

## Player.setrank(playerId, rank, reason)
Sets the rank of the specified player, with an optional reason.

## Player.kick(playerId, reason)
Kicks the specified player from the server with the given reason.

## Player.ban(playerId, reason)
Bans the specified player from the server with the given reason.

## Player.unban(playerId)
Unbans the specified player.

## Player.stop(playerId, reason)
Stops (freezes) the specified player with the given reason.

## Player.unstop(playerId)
Unfreezes the specified player.

## Player.mute(playerId, minutes, reason)
Mutes the specified player for a given number of minutes with a reason.

## Player.unmute(playerId)
# Lua Rank Module

Ranks are identified by an integer rank number. The `isExact` parameter on lookup functions controls matching behaviour: pass 1 to require an exact rank-number match, or 0 to return the closest rank that is ≤ the given number.

## Rank.getall()
Returns a Lua table (array) of all defined rank numbers, plus the count as a second return value.

## Rank.add(rankNumber, name, prefix, suffix)
Creates or replaces the rank with the given number, display name, prefix, and suffix.

## Rank.delete(rankNumber, isExact)
Deletes the rank. If `isExact` is 1, only the exact rank number is removed; if 0, the nearest matching rank is removed.

## Rank.getname(rankNumber, isExact)
Returns the name of the matching rank.

## Rank.getprefix(rankNumber, isExact)
Returns the prefix string of the matching rank.

## Rank.getsuffix(rankNumber, isExact)
Returns the suffix string of the matching rank.

## Rank.getroot(rankNumber, isExact)
Returns the canonical rank number of the matching rank entry (useful when `isExact` is 0 and you need the actual stored rank number).

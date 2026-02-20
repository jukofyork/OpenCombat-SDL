# Formation Implementation Attempts - Lessons Learned

## Overview
This document records our attempts to implement formation changing functionality for squads in OpenCombat SDL. Multiple approaches were tried, each encountering different issues.

## Military Doctrine - Formation Definitions

Based on US Infantry doctrine (from /r/WarCollege discussion):

### Column Formation
- **Spacing**: Standard/moderate distance between units
- **Layout**: Units follow one behind another with dispersion
- **Use**: Movement along routes, allows quick transition to Line
- **Characteristic**: Sub-units can choose their own formation while maintaining vertical alignment
- **Platoon Level**: Squads in column can each have different internal formations

### File Formation  
- **Spacing**: Tight/compact - individuals close to each other
- **Layout**: Single file, one directly behind the other ("follow the leader")
- **Use**: Dense terrain, low visibility, navigating obstacles
- **Characteristic**: Maintains physical contact between team members
- **Key Difference**: Much more compact than Column at fire team level

### Line Formation
- **Spacing**: Units side-by-side
- **Layout**: Perpendicular to direction of movement
- **Use**: Maximum firepower to front, crossing danger areas, assaulting
- **Characteristic**: Leader typically centered or at one end
- **Trade-off**: Excellent firepower but vulnerable to flanking

**Source Reference**: 
> "the principle difference between column and file is dispersion at the fire team level. at the fire team level file is much more compact with individuals closer to each other to maintain contact between the members which is especially important in complex terrain. at the platoon level column and file are distinguished by the formation of the squads."
> - alertjohn117, /r/WarCollege

## Current State
The formation system is currently a stub implementation in `src/objects/Formation.cpp`:
- Only provides basic horizontal line formation
- Soldiers follow in a single file behind the point man
- No actual formation variety is implemented
- Formation enum exists but all types produce same output

## Failed Attempts

### Attempt 1: Immediate Position Update
**Approach:** When F key pressed, immediately call `SetPosition()` to move soldiers to new formation positions.

**Result:** FAILURE - Soldiers teleported instantly to new positions instead of walking there.

**Why it failed:** `SetPosition()` directly sets coordinates without movement animation or pathfinding.

---

### Attempt 2: Re-issue Follow Orders  
**Approach:** Clear soldier orders and call `Follow()` with updated formation type.

**Code Pattern:**
```cpp
// For each non-point-man soldier:
soldier->ClearOrders();  // WRONG - clears _orders not _actionQueue
soldier->Follow(pointMan, newFormation, spread, idx, SoldierAction::Walk);
```

**Result:** FAILURE - Soldiers glitched and jumped on spot.

**Why it failed:** 
- `ClearOrders()` clears the wrong queue (`_orders` from Object base class)
- Soldiers use `_actionQueue` for actions
- The `FollowInFormation` handler caches formation data in `FollowFormationData` struct
- Even with new Follow() calls, the cached data caused race conditions

---

### Attempt 3: WalkTo Actions with Tile Coordinates
**Approach:** Calculate target tile positions and issue `WalkTo` actions for pathfinding.

**Code Pattern:**
```cpp
// Calculate formation position
int targetX, targetY;
Formation::GetFormationPosition(newFormation, idx, spread, &pointManPos, heading, &targetX, &targetY);

// Convert to tiles
int tileI, tileJ;
ConvertPositionToTile(targetX, targetY, &tileI, &tileJ);

// Order walk
soldier->WalkTo(tileI, tileJ);
```

**Result:** FAILURE - Still too buggy/glitchy.

**Why it failed:**
- Formation offset calculations were problematic
- Coordinate system issues (screen vs world coordinates)
- Rotation math based on heading was incorrect
- Soldiers would walk to wrong positions or get stuck

---

### Attempt 4: Multiple Formations with Different Spacing
**Approach:** Implement Column, File, and Line with different spacing values based on military doctrine.

**Formations Planned:**
- **Column**: Standard spacing, one behind other
- **File**: Tight spacing for dense terrain  
- **Line**: Side by side for combat

**Result:** Never completed - abandoned after simpler 2-formation version failed.

**Why it failed:**
- Core positioning math was fundamentally flawed
- Adding more formations multiplied the problems

---

## Root Causes of Failure

### 1. Coordinate System Confusion
- Screen coordinates: Y increases downward
- World coordinates: Different origin/scale
- Formation offsets calculated incorrectly in different contexts

### 2. Wrong Queue Clearing
- `ClearOrders()` clears `_orders` (Order queue)
- Soldiers use `_actionQueue` (Action queue)
- `HandleStopOrder(nullptr)` is the correct way to clear soldier actions

### 3. Cached Formation Data
- `FollowInFormation` handler stores formation type in `FollowFormationData`
- Even issuing new Follow orders doesn't immediately update cached data
- Race conditions when changing formations mid-follow

### 4. Rotation Math Issues  
- Direction to angle conversion was incorrect
- Screen Y-down vs mathematical Y-up coordinate systems
- Formation offsets rotated incorrectly based on heading

## What Would Work Better

### Option A: Synchronous Formation Update
Don't use the existing Follow system at all:
1. Calculate exact pixel positions for each soldier in new formation
2. Create MoveOrders for each soldier with those destinations
3. Let the existing pathfinding handle movement naturally

### Option B: Formation State Machine
Add proper formation state to Squad:
1. Store desired formation separately from current positions
2. Each frame, calculate desired positions based on formation
3. Have soldiers steer toward desired positions gradually
4. Use flocking/separation to avoid collisions

### Option C: Wait for Idle
Only allow formation changes when squad is idle:
1. Check if all soldiers are in "Stopped" state
2. If so, reposition instantly (no animation needed)
3. If moving, queue formation change for when they stop

## Files Modified in Attempts
- `src/objects/Formation.h` - Enum values
- `src/objects/Formation.cpp` - Position calculations
- `src/objects/Soldier.h` - Method declarations
- `src/objects/Soldier.cpp` - WalkTo implementation
- `src/objects/Squad.h` - Getter methods
- `src/world/World.cpp` - F key handler
- `src/objects/Object.h` - Default formation value

## Recommendation
The formation system needs a ground-up redesign:
1. Fix coordinate system handling first
2. Separate formation calculation from movement logic
3. Use existing pathfinding instead of custom follow logic
4. Test with just 2 soldiers before scaling up

## Related Code Locations
- `src/objects/SoldierActionHandlers.cpp:697` - Formation following logic
- `src/objects/Squad.cpp:344` - Squad move order handling
- `src/objects/Soldier.cpp:334` - Follow method implementation
- `src/world/World.cpp:892` - F key handler (formation cycling)

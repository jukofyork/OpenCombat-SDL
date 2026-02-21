# Object Decoupling Attempt - Post-Mortem

**Date:** February 21, 2026  
**Status:** ABANDONED - Too complex, introduced more bugs than it solved  
**Branch:** `reference-based-registry` (preserved for reference)  

## Background

The OpenCombat SDL codebase has significant coupling issues:

1. **Global State Pattern** - Heavy use of `g_Globals` (284+ usages) creates implicit dependencies
2. **God Objects** - `World` class manages 15+ manager classes
3. **Raw Pointer Web** - Objects hold direct pointers to related objects (Squad→Soldier*, Vehicle→Squad*, etc.)
4. **Circular Dependencies** - Object-Soldier-Squad-Vehicle form tight cycles
5. **Globals Header** - 27 files include a globals header with 18 direct includes

The goal was to decouple these relationships using an ID-based lookup system with a central ObjectRegistry.

## What We Attempted

### Phase 0: Foundation
Created a reference-based ObjectRegistry:
- `ObjectID` typedef (`uint32_t`) in `src/misc/ObjectID.h`
- `ObjectRegistry` class with `operator[]` returning references (like `std::array`)
- Automatic registration when objects are added to World
- Reference-based lookups eliminate null pointer checks

### Phase 1: Squad-Soldier Decoupling
Converted Squad to use ID-based relationships:
- Changed `Squad::_soldiers` from `vector<Soldier*>` to `vector<ObjectID>`
- Changed `Squad::_vehicles` from `vector<Vehicle*>` to `vector<ObjectID>`
- Added helper methods `GetSoldier()` and `GetVehicle()` returning references
- Updated SquadManager to populate `createdSoldiers` and `createdVehicles` vectors
- Modified World::Load() to register all objects in ObjectRegistry

## Why It Failed

### 1. Registration Order Complexity
Objects must be registered in ObjectRegistry BEFORE they're referenced by ID. This created a complex initialization dance:
- Squad creates soldiers → soldiers need IDs → but squad stores IDs
- Vehicle crew soldiers need to be registered
- The registration flow became fragile and error-prone

### 2. Implicit Assumptions Break
The codebase has many implicit assumptions about object lifetimes and relationships:
- `Map::SelectObjects()` assumes `object->GetSquad()` always returns valid Squad
- `Squad::GetSquadLeader()` assumes there's always a leader
- Soldiers need their squad pointer set immediately

When these assumptions were violated (even temporarily during construction), the game crashed or froze.

### 3. Method Name Overloading Issues
`Squad::Select()` has two overloads:
- `Select(int x, int y)` - coordinate-based selection
- `Select(bool s)` - set selection state

Calls to `Select(true)` were ambiguous and caused infinite recursion instead of calling `Object::Select(true)`.

### 4. Defensive Programming Cascade
To fix crashes, we added:
- Null checks in Map::SelectObjects
- Graceful fallbacks in GetSquadLeader
- Explicit Object::Select() calls

This created a mess of defensive code that obscured the original logic and made the codebase harder to understand.

### 5. The Freeze Issue
The game loaded but froze on right-click. Despite multiple fixes:
- Fixed SetSquad calls
- Fixed Select() recursion
- Added null checks

The freeze persisted, suggesting deeper architectural issues with the event handling or state management when using ID-based lookups.

## Lessons Learned

### Don't Fix What's Working (Yet)
The original pointer-based system, while not ideal, was functional. The decoupling attempt introduced instability without clear benefits.

### Incremental vs. Big Bang
This was too big a change. A better approach might be:
1. Start with a small, isolated subsystem
2. Prove the pattern works there
3. Gradually expand

### Reference vs. Pointer Semantics
Using references (`Object&`) instead of pointers (`Object*`) provides compile-time safety but makes the code less flexible for cases where objects might legitimately not exist.

### Testing at Each Step
We should have tested gameplay after each small change rather than building the entire system and then debugging.

## What Was Preserved

The `reference-based-registry` branch contains:
- Working ObjectID typedef and ObjectRegistry foundation
- Partially converted Squad class with ID-based lookups
- Registration flow in World::Load()

This code is preserved for future reference but is NOT in a working state.

## Recommended Future Approach

If decoupling is attempted again:

1. **Start smaller** - Pick one relationship (e.g., just Squad-Soldier, ignore vehicles initially)

2. **Hybrid approach** - Keep pointers for now, add ID-based access as an alternative:
   ```cpp
   class Squad {
       std::vector<Soldier*> _soldiers;  // Keep this
       // Add lookup by ID as additional feature
       Soldier* GetSoldierById(ObjectID id);
   };
   ```

3. **Address Globals first** - The `g_Globals` pattern is the root cause. Consider:
   - Dependency injection
   - Service locator pattern
   - Event-driven architecture

4. **ECS (Entity Component System)** - For a game of this complexity, a proper ECS might be cleaner than ID-based references:
   - Entities are just IDs
   - Components store data
   - Systems process entities with specific components

## Conclusion

While the ID-based decoupling approach is theoretically sound, the implementation complexity and the risk of introducing subtle bugs outweighs the benefits for a codebase that's currently functional. The attempt revealed the depth of coupling in the codebase and the challenges of refactoring legacy game code.

The original pointer-based system, despite its flaws, works. Any decoupling effort should be approached more incrementally and with extensive testing at each stage.

**Files Modified (on branch):**
- `src/misc/ObjectID.h` (new)
- `src/world/ObjectRegistry.h/cpp` (new)
- `src/objects/Object.h/cpp` (ObjectID integration)
- `src/objects/Squad.h/cpp` (ID-based relationships)
- `src/objects/SquadManager.h/cpp` (registration vectors)
- `src/world/World.h/cpp` (ObjectRegistry integration)
- `src/world/Map.cpp` (null checks)
- Various other files for API updates

**Status:** Branch preserved but abandoned. Master branch remains at pre-refactoring state.

# OpenCombat SDL - Modding and Scripting Architecture Exploration

**Document Purpose**: Explore architectural changes to enable easier modding and scripting capabilities

**Date**: February 2026

---

## Executive Summary

The current OpenCombat SDL codebase has a solid foundation for modding with its XML-based data files, but several architectural improvements could make it significantly more accessible for modders. This document explores:

1. **Data-Driven Architecture** - Making units, maps, and content fully data-driven
2. **Scripting Integration** - Using Lua or similar for simulation logic
3. **Modern Patterns** - Applying contemporary game development practices

---

## 1. Current State Analysis

### 1.1 Existing Modding Capabilities

The codebase already supports:

- **XML-based configuration**: Soldiers, weapons, squads, vehicles, terrain, effects
- **Modular animation system**: Animation definitions in XML with TGA sprite references
- **Manager pattern**: Centralized creation via SoldierManager, WeaponManager, etc.
- **Text-based action definitions**: `SoldierActions.txt` defines behavior in tabular format

### 1.2 Current Pain Points for Modding

| Issue | Impact | Example |
|-------|--------|---------|
| **Hard-coded C++ types** | Adding new unit types requires code changes | New soldier types need SoldierManager recompilation |
| **Hard-coded AI behavior** | Cannot modify unit behavior without recompiling | A* pathfinding, target selection in C++ |
| **Fixed animation state mappings** | Adding new states requires enum changes | Soldier::AnimationState enum has 16 hard-coded values |
| **Weapon logic in C++** | Cannot create new weapon types with custom behavior | All weapons use same firing logic in Soldier::CalculateShot |
| **No hot-reloading** | Must restart game to see changes | XML changes require restart |
| **No mod packaging format** | No standard way to distribute mods | Mods must overwrite base game files |

### 1.3 Code Locations Requiring Changes for New Content

Adding a new soldier type:
- config/Soldiers.xml (add definition) ✓ Easy
- config/SoldierAnimations.xml (add animations) ✓ Easy  
- graphics/Soldiers/NewType/ (add sprites) ✓ Easy
- src/objects/SoldierManager.cpp (may need changes) ✗ Hard
- src/objects/Soldier.h (if new capabilities needed) ✗ Hard

Adding a new weapon:
- config/Weapons.xml (add definition) ✓ Easy
- src/objects/Weapon.cpp (if new behavior needed) ✗ Hard
- src/objects/Soldier.cpp (CalculateShot logic) ✗ Hard

---

## 2. Recommended Architecture Changes

### 2.1 Component-Based Entity System (ECS)

**Current Pattern**: Deep inheritance hierarchy
```cpp
class Object -> class Soldier -> specialized behavior in C++
class Object -> class Vehicle -> specialized behavior in C++
```

**Proposed Pattern**: Composition over inheritance
- Entity contains components
- Components define behavior (WeaponComponent, MovementComponent)
- Behavior defined in data/scripts, not C++

**Benefits**:
- New unit types = new XML component combinations, no C++ changes
- Modders can create novel unit types by mixing components
- Enables data-driven behavior through component properties

### 2.2 Scriptable Behavior System with Lua

**Current**: All behavior hard-coded in C++
- Target selection in Soldier::FindTarget()
- Combat logic in Soldier::CalculateShot()
- Pathfinding in AStar class

**Proposed**: Scriptable with Lua
- XML references Lua script: `<BehaviorScript>sniper_behavior.lua</BehaviorScript>`
- Lua overrides specific behaviors
- Fallback to C++ if no script defined

**Implementation**: Lua with sol2 library
- Mature, well-documented
- Easy C++ binding generation
- ~200KB binary size increase

### 2.3 Hot-Reloadable Data System

**Current**: Data loaded once at startup
- XML parsed in Load() methods
- Must restart game to see changes

**Proposed**: File watcher with hot-reload
- Monitor config files for changes
- Reload definitions in-place
- Update existing entities without restart

**Benefits**:
- Instant iteration when tuning values
- Modders can test changes without restart
- Enables "mod browser" in-game

### 2.4 Mod Packaging System

**Current**: No structure - files overwrite base game

**Proposed**: Standardized mod format
```
mods/
├── my_mod/
│   ├── mod.json              <- Mod metadata
│   ├── config/
│   ├── graphics/
│   ├── scripts/
│   └── maps/
```

```json
{
    "name": "Elite Units Pack",
    "version": "1.0.0",
    "author": "ModderName",
    "dependencies": ["base_game >= 1.0"],
    "load_order": 100,
    "overrides": {
        "config/Soldiers.xml": "merge"
    }
}
```

---

## 3. Lua Integration Architecture

### 3.1 Key Binding Points

```cpp
// Expose C++ classes to Lua
lua.new_usertype<Soldier>("Soldier",
    "get_position", &Soldier::GetPosition,
    "get_health", &Soldier::GetHealth,
    "add_order", &Soldier::AddOrder
);

lua.new_usertype<World>("World",
    "query_radius", &World::QueryRadius,
    "find_path", &World::FindPath
);
```

### 3.2 Script Hook Points in Simulation

Replace hard-coded behavior with script hooks:

```cpp
void Soldier::Simulate(long dt, World* world) {
    // Call Lua script if defined
    if(!_behaviorScript.empty()) {
        auto result = g_LuaAPI->Call("Simulate", this, dt, world);
        if(result.valid()) return;  // Script handled it
    }
    // Fall back to C++
}
```

### 3.3 Example: System Dynamics in Lua

```lua
-- mods/scripts/system_dynamics.lua

function UpdateMentalState(soldier, dt)
    local state = soldier:GetMentalState()
    
    -- Suppression builds from nearby explosions
    local threats = World.QueryThreats(soldier:GetPosition(), 100)
    for _, threat in ipairs(threats) do
        state.suppression = state.suppression + (threat.intensity * dt * 0.1)
    end
    
    -- Morale affected by squad casualties
    local squad = soldier:GetSquad()
    if squad then
        state.morale = state.morale - (squad:GetRecentCasualties() * 5)
    end
    
    -- Apply state changes
    soldier:SetMentalState(state)
    
    -- Behavior modification
    if state.suppression > 70 then
        soldier:SetBehavior("pinned")
    end
end
```

---

## 4. Modern Game Programming Patterns

### 4.1 Event Bus / Message Queue

**Current**: Direct function calls (tight coupling)

**Proposed**: Decoupled event system
- Systems publish events
- Other systems subscribe
- Enables save/load via event replay

### 4.2 Command Pattern

Implement for undo/redo support:
- IssueOrderCommand
- MoveUnitCommand
- Ctrl+Z support in editor mode

### 4.3 Spatial Hashing

**Current**: Linked list per tile (O(n) for large maps)

**Proposed**: Spatial hash or quadtree
- Better cache locality
- Faster queries for AI
- Parallel-friendly

### 4.4 Data-Oriented Design (DOD)

**Current**: Object-oriented with scattered data

**Proposed**: Structure of Arrays (SoA)
- Cache-friendly iteration
- SIMD optimization potential
- Easier serialization

### 4.5 Hierarchical State Machines

**Current**: Flat state bitfield (64 bits)

**Proposed**: HSM with stack-based states
- Natural state composition
- Easier to add new states
- Better organization

---

## 5. Implementation Roadmap

### Phase 1: Foundation (2-3 months)
1. Hot-reloading system
2. Mod packaging format
3. Event bus system

### Phase 2: Scripting (3-4 months)
1. Lua integration with sol2
2. Behavior script hooks
3. Data-driven components

### Phase 3: Advanced Features (4-6 months)
1. Full ECS migration
2. System Dynamics in Lua
3. Parallel system execution

---

## 6. Benefits Summary

**For Modders**:
- Add new units without recompiling
- Modify AI behavior with Lua scripts
- Hot-reload for rapid iteration
- Standard packaging format

**For Developers**:
- Cleaner architecture with ECS
- Easier testing with scriptable behaviors
- Better performance with DOD patterns
- More extensible codebase

**Trade-offs**:
- ~3-6 months development time
- ~200KB binary size increase
- Slight performance overhead (<5%)

---

## 7. Recommended Dependencies

| Library | Purpose | Size | License |
|---------|---------|------|---------|
| sol2 | Lua-C++ bindings | Header-only | MIT |
| Lua 5.4 | Scripting runtime | ~200KB | MIT |
| efsw | File watcher | ~100KB | MIT |

---

## 8. Conclusion

The proposed changes would transform OpenCombat SDL from a hard-coded C++ game into a data-driven, scriptable platform suitable for extensive modding.

**Start with**: Hot-reloading and mod packaging (high value, low risk)
**Then add**: Lua scripting once foundation is solid

This approach preserves the existing codebase while opening up powerful new capabilities for modders and future development.

# OpenCombat SDL - Architecture Documentation

**Last Updated**: February 2026

---

## Overview

**OpenCombat SDL** is a real-time tactical wargame engine originally written for Windows/DirectX and ported to SDL2 for cross-platform compatibility. The game simulates World War II squad-level combat with realistic ballistics, line-of-sight, and unit behavior.

**Key Characteristics:**
- **Genre**: Real-time tactical wargame (RTS/TBS hybrid)
- **Scale**: Squad-level (4-12 soldiers per squad, multiple squads per team)
- **Setting**: World War II (primarily Western Front)
- **Perspective**: Top-down isometric view
- **Engine**: SDL2-based (port from DirectX)
- **Language**: C++17
- **Lines of Code**: ~21,500 across 139 source files
- **Status**: SDL2 port complete, testing phase

---

## Architecture Documentation Chapters

This comprehensive architecture documentation has been organized into 10 chapters, each focusing on a specific aspect of the codebase. These chapters follow a consistent pedagogical flow:

1. **Concept Level**: Gameplay overview and high-level concepts
2. **Algorithm Level**: How systems work (language-neutral design)
3. **Implementation Level**: Actual C++ code details

Read sequentially for a complete understanding, or jump to specific chapters for reference. The documentation provides sufficient detail for someone unfamiliar with the codebase to understand and potentially reimplement the system.

### [Chapter 1: Overview and High-Level Architecture](./chapters/01-overview.md)

**Scope**: Foundation concepts, project structure, and architectural patterns

**Topics Covered**:
- High-level system architecture and module relationships
- Core design patterns (Manager, Reference Counting, Non-Owning References)
- Directory structure and file organization
- Main execution flow and game loop
- Self-test infrastructure
- Coordinate systems and time management
- Debug rendering flags
- C++17 modernization summary (constexpr, enum class, std::array, nullptr, static_cast)

**Key Files**: `src/main.cpp`, `src/application/GameApplication.h`, `src/application/Globals.h`

---

### [Chapter 2: Object System](./chapters/02-object-system.md)

**Scope**: Game entities and their relationships

**Topics Covered**:
- Object base class and common functionality
- Soldier class (animations, states, attributes, weapons, physics)
- Vehicle class (tank mechanics, turret rotation, crew management)
- Squad class (formation system, order distribution, quality ratings)
- Object ownership model and memory management
- Selection mechanisms and hit detection
- Order queues and action queues
- Combat actions and death handling

**Key Files**: `src/objects/Object.h/cpp`, `src/objects/Soldier.h/cpp`, `src/objects/Vehicle.h/cpp`, `src/objects/Squad.h/cpp`

---

### [Chapter 3: State Machine and Action System](./chapters/03-state-machine.md)

**Scope**: Behavior state management and action execution

**Topics Covered**:
- State class - 64-bit bitfield implementation
- Soldier states (22 logical states mapped to 16 animation states)
- Action system structure and definitions
- Action loading from SoldierActions.txt
- Action handler system (22 action types)
- Action data structures (TileData, FireActionData, FollowFormationData)
- Action execution flow and prerequisites
- Animation marker system for timing
- State transition examples and complex transitions

**Key Files**: `src/states/State.h` (header-only), `src/states/Action.h`, `src/states/ObjectActions.h/cpp`, `src/objects/SoldierActionHandlers.h/cpp`, `config/SoldierActions.txt`

---

### [Chapter 4: Order System and AI Pathfinding](./chapters/04-orders-pathfinding.md)

**Scope**: High-level commands and pathfinding algorithms

**Topics Covered**:
- Order base class and reference counting
- Order types (Move, MoveFast, Sneak, Fire, Ambush, Defend, Stop, Pause)
- Order queue management in Object base class
- Order processing in Soldier::Simulate()
- Squad-level order distribution
- A* pathfinding implementation
  - Node structure and memory management
  - MinHeap priority queue
  - Hash table (closed list)
  - 8-directional successor generation
  - Terrain cost calculation
  - Octile distance heuristic
- Path structure and memory pools
- Target selection algorithms
- Complete order-to-movement flow

**Key Files**: `src/orders/Order.h/cpp`, `src/orders/*.h` (MoveOrder, FireOrder, etc.), `src/ai/AStar.h/cpp`, `src/ai/Path.h`

---

### [Chapter 5: Graphics and Rendering System](./chapters/05-graphics.md)

**Scope**: Software rendering and visual presentation

**Topics Covered**:
- Screen class - software rendering surface
- Pixel buffer management and format detection
- 10 Blit method variants (basic, transparency, source rect, effects, alpha, mask, rotation)
- Transparency and color-key rendering
- Alpha blending calculations
- Mask-based rendering for soldiers
- Primitive drawing (lines, rectangles)
- Hit testing with ray casting
- FontManager and text rendering
- Animation system
- TGA image loading
- Effect rendering (muzzle flashes, explosions)
- Self-test for rendering validation

**Key Files**: `src/graphics/Screen.h/cpp`, `src/graphics/FontManager.h/cpp`, `src/graphics/Animation.h/cpp`, `src/misc/TGA.h/cpp`, `src/graphics/Effect.h/cpp`, `src/graphics/SoldierMasks.h`

---

### [Chapter 6: World, Map, and Terrain System](./chapters/06-world-terrain.md)

**Scope**: Game world simulation and environment

**Topics Covered**:
- World class - central simulation manager
- World initialization and setup from XML
- Object management (_mobileObjects vector)
- Simulation loop and time management
- Map class and terrain representation
- Tile system (10x10 pixel tiles)
- Mega-tile regions (12x12 tiles)
- Terrain types and elevation
- Passability and hindrance systems
- Element system (trees, buildings, cover)
- Element levels (Prone, Low, Medium, High)
- Building system and interiors
- Line-of-sight (LOS) calculations
- Spatial partitioning for rendering
- World rendering pipeline
- Debug visualization options

**Key Files**: `src/world/World.h/cpp`, `src/world/Map.h/cpp`, `src/world/Element.h/cpp`, `src/world/Building.h/cpp`, `src/world/LineOfSight.h/cpp`

---

### [Chapter 7: Configuration Files and Data Schemas](./chapters/07-configuration.md)

**Scope**: XML-based data definitions

**Topics Covered**:
- XML schema overview using tinyxml2
- Soldier template definitions
- Weapon specifications and attributes
- Vehicle configurations
- Squad composition templates
- Animation sequence definitions
- Effect definitions
- Terrain element definitions
- Map file format
- Building definitions
- Color modifier specifications
- Configuration loading and validation

**Key Files**: `config/*.xml`, `src/misc/tinyxml2.h/cpp`

---

### [Chapter 8: Asset Structure and File Formats](./chapters/08-assets.md)

**Scope**: Game assets and their formats

**Topics Covered**:
- Directory structure (graphics/, maps/, sounds/)
- TGA image format support
- 32-bit ARGB and 24-bit RGB TGA files
- Sprite sheets and animation frames
- Soldier sprite organization
- Vehicle sprite requirements (hull, turret, wreck)
- Terrain tile graphics
- Building graphics and layering
- Sound format (WAV files)
- Audio loading with SDL2_mixer
- Asset naming conventions
- File paths and portability

**Key Files**: Graphics files in `graphics/`, `maps/`, `sounds/` directories

---

### [Chapter 9: UI System and Combat Module](./chapters/09-ui-combat.md)

**Scope**: User interface and game module implementation

**Topics Covered**:
- CombatModule as main game module
- Module lifecycle (Initialize, Update, Simulate, Render)
- Input handling and SDL event processing
- F-Key toggle system for debug features
- FPS and frame time tracking
- Minimap rendering and interaction
- UI panels and controls
- Squad selection and command interface
- Order issuing (move, fire, ambush, defend)
- Mouse interaction (selection, panning, zooming)
- Keyboard shortcuts and help system
- Game state visualization

**Key Files**: `src/application/CombatModule.h/cpp`, `src/application/GameApplication.h/cpp`, `src/main.h/cpp` (CSDLApplication)

---

### [Chapter 10: Build System and Dependencies](./chapters/10-build-system.md)

**Scope**: Compilation, dependencies, and platform considerations

**Topics Covered**:
- Makefile-based build system
- Supported platforms (Linux primary, cross-platform capable)
- Required dependencies:
  - SDL2 (core)
  - SDL2_mixer (audio)
  - SDL2_ttf (fonts, optional)
  - tinyxml2 (XML parsing)
- Compilation flags and options
- Debug vs Release builds
- Installation instructions
- Dependency management
- Troubleshooting common issues
- Testing commands and self-tests
- Portability considerations

**Key Files**: `Makefile`, `README.md`, `AGENTS.md`

---

## Supplementary Documentation

The following additional documentation files are available in the `docs/other/` directory for more specific topics:

- **ASSET_REORGANIZATION_PLAN.md** - Future plans for reorganizing game assets
- **CRITICAL_DESIGN_PATTERNS.md** - Important design patterns and common pitfalls
- **DECOUPLING_ATTEMPT_POSTMORTEM.md** - Lessons from code decoupling efforts
- **FORMATION_IMPLEMENTATION_LESSONS.md** - Insights from implementing squad formations
- **MODDING_ARCHITECTURE_IDEAS.md** - Proposals for modding support
- **PROBLEMS.md** - Known issues and limitations
- **SOLDIER_ANIMATION_MIGRATION_PLAN.md** - Animation system migration details
- **VEHICLE_COMBAT_IMPLEMENTATION_PLANS.md** - Plans for completing vehicle combat
- **ZOOM_IMPLEMENTATION_NOTES.md** - Technical details of zoom feature

---

## Reading Guide

### For New Contributors

Start with these chapters in order:
1. **Chapter 1** - Understand the big picture
2. **Chapter 2** - Learn about game entities
3. **Chapter 5** - Understand how things are displayed
4. **Chapter 10** - Get the build working

### For Implementing Features

- **Adding new unit types**: Chapters 2, 3, 4
- **Modifying rendering**: Chapters 5, 6
- **Adding new orders/commands**: Chapters 3, 4
- **Changing game data**: Chapters 7, 8
- **UI changes**: Chapters 5, 9

### For Code Review

- **Understanding ownership**: Chapter 2, Section 2.5
- **State management**: Chapter 3
- **Reference counting**: Chapter 2, Section 2.1.4 and Chapter 4, Section 4.1.2
- **Memory management**: Throughout all chapters

---

## Document Maintenance

These architecture documents should be updated when:
- New major features are added
- Existing systems are refactored
- APIs change significantly
- New patterns are introduced

When updating, ensure:
- Code examples compile and match current implementation
- File paths and line numbers are accurate
- Cross-references between chapters remain valid

---

## Questions and Issues

For questions about the architecture or to report documentation issues:
- Check the relevant chapter for detailed information
- Refer to source code for implementation details
- See `docs/other/` for specific topic deep-dives
- Review `AGENTS.md` for coding conventions

**Repository**: `/home/juk/git/OpenCombat-SDL`



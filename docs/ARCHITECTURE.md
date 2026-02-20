# OpenCombat SDL - Architecture Documentation

**Document History**: This document was renamed from `DESIGN.md` (originally created in commit 95ea30a) to better reflect its purpose as comprehensive architecture documentation for the SDL2 port.

**Last Updated**: February 2026
- Added: C++17 modernization summary (Section 1.11)
- Updated: Code examples to use modern C++ (std::array, enum class, constexpr, nullptr)
- Updated: All #define constants converted to constexpr
- Updated: C-style casts converted to static_cast
- Updated: C-style arrays converted to std::array
- Updated: Enums converted to enum class for type safety
- Added: Self-test infrastructure (Section 1.6)
- Added: Debug rendering flags (Section 1.10)
- Added: F-Key toggle system documentation (Section 9.1.5)
- Added: FPS tracking implementation (Section 9.1.6)
- Added: Vehicle combat implementation status (Section 2.3.8)
- Updated: Lines of code from ~21,420 to ~14,000 (SDL2 port optimized codebase)

## Table of Contents

1. [Overview and High-Level Architecture](#1-overview-and-high-level-architecture)
   - 1.6 [Self-Test Infrastructure](#16-self-test-infrastructure)
   - 1.10 [Debug Rendering Flags](#110-debug-rendering-flags)
   - 1.11 [C++17 Modernization Summary](#111-c17-modernization-summary)
2. [Object System](#2-object-system)
   - 2.3.8 [Vehicle Combat Implementation Status](#238-combat-implementation-status)
3. [State Machine and Action System](#3-state-machine-and-action-system)
4. [Order System and AI Pathfinding](#4-order-system-and-ai-pathfinding)
5. [Graphics and Rendering System](#5-graphics-and-rendering-system)
6. [World, Map, and Terrain System](#6-world-map-and-terrain-system)
7. [Configuration Files and Data Schemas](#7-configuration-files-and-data-schemas)
8. [Asset Structure and File Formats](#8-asset-structure-and-file-formats)
9. [UI System and Combat Module](#9-ui-system-and-combat-module)
   - 9.1.5 [F-Key Toggle System](#915-input-handling---f-key-toggle-system)
   - 9.1.6 [FPS and Frame Time Tracking](#916-fps-and-frame-time-tracking)
10. [Build System and Dependencies](#10-build-system-and-dependencies)

---

## 1. Overview and High-Level Architecture

### 1.1 Project Overview

**OpenCombat SDL** is a real-time tactical wargame engine originally written for Windows/DirectX and ported to SDL2 for cross-platform compatibility. The game simulates World War II squad-level combat with realistic ballistics, line-of-sight, and unit behavior.

**Key Characteristics:**
- **Genre**: Real-time tactical wargame (RTS/TBS hybrid)
- **Scale**: Squad-level (4-12 soldiers per squad, multiple squads per team)
- **Setting**: World War II (primarily Western Front)
- **Perspective**: Top-down isometric view
- **Engine**: SDL2-based (port from DirectX)
- **Language**: C++17
- **Lines of Code**: ~14,000 across 158 source files
- **Status**: SDL2 port complete, testing phase

### 1.2 High-Level Architecture

The architecture follows a modular design with clear separation between game logic, rendering, and platform abstraction:

```
┌──────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                         │
│  ┌──────────────┐  ┌──────────────────┐  ┌────────────────┐  │
│  │   main.cpp   │  │  GameApplication │  │ CSDLApplication│  │
│  │  (Entry)     │  │    (Module)      │  │  (SDL Wrapper) │  │
│  └──────────────┘  └──────────────────┘  └────────────────┘  │
└──────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     MODULE SYSTEM                           │
│  ┌──────────────────┐  ┌──────────────────┐                 │
│  │  CombatModule    │  │  Introduction    │ (Future mods)   │
│  │  (Game Logic)    │  │  (Menu/Screens)  │                 │
│  └──────────────────┘  └──────────────────┘                 │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌───────────────────────────────────────────────────────────┐
│                      GAME WORLD                           │
│  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐  │
│  │   World   │ │    Map    │ │ Building  │ │  LineOf   │  │
│  │ (Manager) │ │  (Terrain)│ │ (Manager) │ │   Sight   │  │
│  └───────────┘ └───────────┘ └───────────┘ └───────────┘  │
└───────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────┐
│                    OBJECT SYSTEM                        │
│  ┌──────────┐ ┌────────────┐ ┌──────────┐ ┌──────────┐  │
│  │  Object  │ │  Soldier   │ │  Vehicle │ │  Squad   │  │
│  │  (Base)  │ │ (Infantry) │ │  (Tank)  │ │  (Team)  │  │
│  └──────────┘ └────────────┘ └──────────┘ └──────────┘  │
└─────────────────────────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────────┐
│                      SUBSYSTEMS                              │
│  ┌────────────┐ ┌────────────┐ ┌────────────┐ ┌───────────┐  │
│  │   Orders   │ │   States   │ │  Actions   │ │    AI     │  │
│  │ (Commands) │ │ (Behavior) │ │ (Handlers) │ │(Pathfind) │  │
│  └────────────┘ └────────────┘ └────────────┘ └───────────┘  │
│  ┌────────────┐ ┌────────────┐ ┌────────────┐ ┌───────────┐  │
│  │  Graphics  │ │   Sound    │ │    UI      │ │   Input   │  │
│  │  (Render)  │ │  (Audio)   │ │ (Widgets)  │ │  (Events) │  │
│  └────────────┘ └────────────┘ └────────────┘ └───────────┘  │
└──────────────────────────────────────────────────────────────┘
```

### 1.3 Core Design Patterns

#### 1.3.1 Manager Pattern
All resource types use manager classes for centralized creation and lifetime management:

- **SoldierManager**: Creates soldiers from XML templates
- **WeaponManager**: Manages weapon definitions
- **VehicleManager**: Handles vehicle types
- **SquadManager**: Creates squad compositions
- **AnimationManager**: Loads and clones animation sequences
- **EffectManager**: Manages visual effects
- **ElementManager**: Terrain element definitions

#### 1.3.2 Reference Counting
Orders use manual reference counting to handle shared ownership:

```cpp
class Order {
    int _refCount;
public:
    void IncrementRefCount() { ++_refCount; }
    void Release() { if(--_refCount <= 0) delete this; }
};
```

**Critical Rule**: Never add the same Order pointer to multiple objects without incrementing reference count.

#### 1.3.3 Non-Owning References
Squads maintain references to soldiers/vehicles without ownership:

```cpp
class Squad {
    std::vector<Soldier*> _soldiers;  // Non-owning - World owns objects
    std::vector<Vehicle*> _vehicles;  // Non-owning
};
```

Objects are owned by `World::_mobileObjects` vector.

#### 1.3.4 State Machine with Prerequisites
Actions automatically chain based on state prerequisites:

```cpp
// If requirements not met, system finds and prepends prerequisite actions
if(!requirementsMet) {
    _actionQueue.push_front(new Action(prerequisiteAction));
}
```

### 1.4 Directory Structure

```
OpenCombat-SDL/
├── src/
│   ├── ai/              # A* pathfinding
│   ├── application/     # Main application, modules
│   ├── graphics/        # Rendering, animations, effects
│   ├── misc/            # Utilities, TGA loader, XML parser
│   ├── objects/         # Game objects (Soldier, Vehicle, Squad)
│   ├── orders/          # Order types
│   ├── sound/           # Audio management
│   ├── states/          # State machine, actions
│   └── world/           # World, Map, Buildings, LOS
├── config/              # XML configuration files
├── graphics/            # Visual assets (TGA files)
├── maps/                # Map data and building graphics
└── sounds/              # Audio assets (WAV files)
```

### 1.5 Main Execution Flow

```
1. main()
   └── Parse command-line arguments (--test-screen, --test-actionqueue, --test-all)
   └── Run requested SelfTests (Screen, ActionQueue)
   └── If tests run, exit without starting game
   └── Initialize SDL (video, audio, timer)
   └── Create GameApplication
   └── ChooseModule(Combat)
   └── Run main loop

2. Game Loop (per frame)
   └── Poll SDL Events (keyboard, mouse, window)
   └── Update() - Process input, update mouse states
   └── Simulate(dt) - Update game logic
   │   └── CombatModule::Simulate(dt)
   │       └── World::Simulate(dt)
   │           └── For each object: object->Simulate(dt, world)
   │       └── Track FPS and frame timing statistics
   └── Render(screen)
       └── CombatModule::Render(screen)
           └── World::Render(screen, clip)
               └── Render map, buildings, objects, effects
           └── Render UI panels, minimap
           └── Render FPS/stats overlays (if enabled)
           └── Render help text overlay (if enabled)
   └── SDL_RenderPresent()

3. Shutdown
   └── Cleanup managers and resources
   └── SDL_Quit()
```

### 1.6 Self-Test Infrastructure

The codebase includes a self-test framework for validating critical subsystems:

**Test Entry Point**: `src/main.cpp:51-85`

```cpp
// Command-line test options
bool testScreen = false;
bool testActionQueue = false;

for(int i = 1; i < argc; i++) {
    if(strcmp(argv[i], "--test-screen") == 0 || strcmp(argv[i], "--test-all") == 0) {
        testScreen = true;
    }
    if(strcmp(argv[i], "--test-actionqueue") == 0 || strcmp(argv[i], "--test-all") == 0) {
        testActionQueue = true;
    }
}

// Run tests and exit
if(testScreen) {
    Screen::SelfTest();      // Tests blitting, clipping, alpha
}
if(testActionQueue) {
    ActionQueue::SelfTest(); // Tests circular buffer queue
}
```

**Available Tests**:
- `Screen::SelfTest()` - Tests software rendering, blitting, clipping, alpha blending (src/graphics/Screen.cpp:702)
- `ActionQueue::SelfTest()` - Tests circular buffer implementation for action indices (src/states/ActionQueue.h:92)

**Running Tests**:
```bash
./opencombat --test-screen        # Test graphics only
./opencombat --test-actionqueue   # Test action queue only
./opencombat --test-all           # Run all tests
```

### 1.7 Global State Management

The `Globals` struct provides centralized access to all subsystems:

```cpp
struct Globals {
    WorldGlobals World;           // Game world state
    ApplicationGlobals Application; // Platform state
};

struct WorldGlobals {
    SoldierManager* Soldiers;
    SquadManager* Squads;
    WeaponManager* Weapons;
    EffectManager* Effects;
    AnimationManager* Animations;
    World* CurrentWorld;
    AStar Pathing;
    TeamAttributes Teams[32];
    int NumTeams;
};
```

Access via global pointer: `g_Globals->World.CurrentWorld`

### 1.8 Coordinate Systems

| System | Units | Origin | Description |
|--------|-------|--------|-------------|
| Screen | pixels | (0,0) top-left | Window coordinates |
| World | pixels | (0,0) top-left | Absolute map position |
| Tile | blocks | (0,0) top-left | 10x10 pixel grid |
| Mega-tile | 12x12 tiles | - | 120x120 pixel regions |

**Conversions**:
- World to Screen: `screen = world - origin`
- World to Tile: `tile = world / 10`
- Tile to World: `world = tile * 10 + 5` (center)

### 1.9 Time and Simulation

- **Simulation timestep**: 50ms (configurable)
- **Frame time**: Variable (vsync-independent)
- **Animation timing**: Milliseconds per frame (33ms = ~30fps)
- **Weapon timing**: All times in milliseconds

```cpp
void Update() {
    long currentMillis = GetTickCount();
    if(currentMillis - oldMillis >= SIMULATION_TIMESTEP_MS) {
        _millis = currentMillis;
        _game->Simulate(SIMULATION_TIMESTEP_MS);
    }
}
```

### 1.10 Debug Rendering Flags

The `WorldGlobals` struct includes debugging flags for visualizing game state:

**Location**: `src/application/Globals.h:150-161`

```cpp
struct WorldGlobals {
    bool bRenderElevation;        // Show terrain elevation
    bool bRenderElements;         // Show terrain elements (trees, etc.)
    bool bRenderStats;            // Show FPS and frame time
    bool bWeaponFan;              // Show weapon line-of-sight fan
    bool bRenderBoundingBoxes;    // Show object bounding boxes
    bool bRenderPaths;            // Show movement paths
    bool bRenderHelpText;         // Show control help overlay
    bool bRenderBuildingOutlines; // Show building outlines
    bool bRenderBuildingInteriors;// Show building interiors
};
```

**Default Values** (constructor in Globals.h:193):
```cpp
WorldGlobals() { 
    bRenderElevation = false;
    bRenderElements = true;
    bWeaponFan = false;
    bRenderBoundingBoxes = false;
    bRenderStats = false;
    bRenderPaths = false;
    bRenderHelpText = false;
    bRenderBuildingOutlines = false;
    bRenderBuildingInteriors = false;
}
```

### 1.11 C++17 Modernization Summary

The codebase has been systematically modernized from C-style to modern C++17. These changes improve type safety, readability, and maintainability.

#### 1.11.1 Constants - constexpr instead of #define

All `#define` constants have been converted to `constexpr` with explicit types:

**Before:**
```cpp
#define MAX_WEAPONS_PER_SOLDIER 8
#define MAX_CREW 8
#define SIMULATION_TIMESTEP_MS 50
#define DA 0.01f  // Turret rotation epsilon
```

**After:**
```cpp
constexpr int MAX_WEAPONS_PER_SOLDIER = 8;
constexpr int MAX_CREW = 8;
constexpr long SIMULATION_TIMESTEP_MS = 50;
constexpr float TARGET_ANGLE_EPSILON = 0.01f;  // Renamed for clarity
```

**Benefits:**
- Type safety with explicit types
- Namespaced (no macro name collisions)
- Scoped lifetime
- Better debugger support
- Respects C++ naming conventions

#### 1.11.2 Type Safety - enum class instead of enum

Plain enums have been converted to `enum class` for strong type checking:

**Before:**
```cpp
enum Direction {
    South = 0, SouthWest, West, NorthWest,
    North, NorthEast, East, SouthEast, NumDirections
};

// Implicit conversion to int allowed
Direction dir = South;
int i = dir;  // Compiles - potential bug
```

**After:**
```cpp
enum class Direction {
    South = 0, SouthWest, West, NorthWest,
    North, NorthEast, East, SouthEast, NumDirections
};

// Explicit conversion required
Direction dir = Direction::South;
int i = static_cast<int>(dir);  // Must cast explicitly
```

**Benefits:**
- Prevents accidental integer conversions
- Requires explicit scope (Direction::South)
- Eliminates name collisions in global namespace
- Enables function overloading based on enum type

#### 1.11.3 Container Safety - std::array instead of C-style arrays

C-style arrays have been replaced with `std::array`:

**Before:**
```cpp
Weapon* _weapons[MAX_WEAPONS_PER_SOLDIER];
int _weaponsNumClips[MAX_WEAPONS_PER_SOLDIER];

// No bounds checking
_weaponNumClips[10] = 5;  // Undefined behavior, no warning
```

**After:**
```cpp
std::array<Weapon*, MAX_WEAPONS_PER_SOLDIER> _weapons;
std::array<int, MAX_WEAPONS_PER_SOLDIER> _weaponsNumClips;

// Can use bounds-checked access
_weaponsNumClips.at(10) = 5;  // Throws std::out_of_range
// Or use .size() in loops
for (size_t i = 0; i < _weaponsNumClips.size(); ++i) { ... }
```

**Benefits:**
- Value semantics (copyable, comparable)
- Bounds checking with `.at()`
- Standard container interface (iterators, `.size()`, `.data()`)
- No implicit decay to pointer
- Better integration with STL algorithms

#### 1.11.4 Pointer Safety - nullptr instead of NULL

All null pointer constants use `nullptr`:

**Before:**
```cpp
#define NULL 0  // Or compiler-defined

void* ptr = NULL;
if (ptr == NULL) { ... }
```

**After:**
```cpp
void* ptr = nullptr;
if (ptr == nullptr) { ... }
```

**Benefits:**
- Distinct type for null pointers (std::nullptr_t)
- Prevents ambiguous overload resolution
- Self-documenting intent
- No macro redefinition issues

#### 1.11.5 Type Safety - static_cast instead of C-style casts

All C-style casts have been converted to explicit `static_cast`:

**Before:**
```cpp
// C-style cast - could be any of: static_cast, reinterpret_cast, const_cast
int i = (int)floatValue;
Soldier* s = (Soldier*)object;
unsigned char b = (unsigned char)(value >> 8);
```

**After:**
```cpp
// Explicit cast type - clear intent
int i = static_cast<int>(floatValue);
Soldier* s = static_cast<Soldier*>(object);
unsigned char b = static_cast<unsigned char>(value >> 8);
```

**Benefits:**
- Clear intent and searchable in code
- Compiler checks cast validity at compile time
- Cannot accidentally cast away constness
- Safer than C-style (reinterpret_cast required for dangerous casts)

#### 1.11.6 Container Modernization - std::vector with pair

Parallel arrays have been consolidated using `std::vector` with `std::pair`:

**Before:**
```cpp
// Parallel arrays - must keep in sync
std::vector<std::string> _names;
std::vector<Color> _colors;

// Access requires indexing both
std::string name = _names[i];
Color color = _colors[i];  // Must be same index!
```

**After:**
```cpp
// Single vector of pairs - data always together
std::vector<std::pair<std::string, Color>> _nameColorPairs;

// Access both at once
auto& [name, color] = _nameColorPairs[i];  // C++17 structured binding
```

**Benefits:**
- Data locality (name and color always paired)
- Single insertion/removal operation
- Prevents index synchronization bugs
- Clearer semantic relationship

#### 1.11.7 Summary of Changes

| Aspect | Old Style | Modern Style | Files Affected |
|--------|-----------|--------------|----------------|
| Constants | `#define MAX 100` | `constexpr int MAX = 100;` | 15+ headers |
| Enums | `enum Color { Red };` | `enum class Color { Red };` | 8 files |
| Arrays | `int arr[10];` | `std::array<int, 10> arr;` | 12 files |
| Null pointers | `NULL` or `0` | `nullptr` | 50+ locations |
| Casts | `(Type)value` | `static_cast<Type>(value)` | 200+ locations |
| Parallel arrays | Two vectors | `std::vector<std::pair<T, U>>` | ColorManager |

---

## 2. Object System

### 2.1 Object Base Class

**Location**: `src/objects/Object.h`, `src/objects/Object.cpp`

The `Object` class is the abstract base for all game entities, providing common functionality for position, selection, orders, and rendering.

#### 2.1.1 Core Interface

```cpp
class Object {
public:
    // Position in world coordinates
    Point Position;
    
    // Linked list for spatial partitioning
    Object* NextObject;
    Object* PrevObject;
    
    // Pure virtual methods
    virtual void Render(Screen* screen, Rect* clip) = 0;
    virtual void Simulate(long dt, World* world) = 0;
    virtual bool IsMobile() { return false; }
    
    // Selection
    virtual bool Select(int x, int y);
    virtual void Select(bool s) { _isSelected = s; }
    virtual bool IsSelected() { return _isSelected; }
    virtual bool Contains(int x, int y);
    
    // Orders (reference counted)
    virtual void AddOrder(Order* o);
    virtual void InsertOrder(Order* o, int i);
    virtual void ClearOrders();
    
    // Capabilities
    bool CanMove();
    bool CanFire();
    bool CanDefend();
    // ... etc
    
    // Identity
    long GetID();
    const std::string& GetName();
    const std::string& GetIconName();
};
```

#### 2.1.2 Member Variables

```cpp
protected:
    bool _isSelected;                    // Selection state
    Bounds _minBounds;                   // Hit detection bounds
    
    // Capability flags (set from XML)
    bool _canFire, _canMove, _canMoveFast;
    bool _canSneak, _canDefend, _canAmbush, _canSmoke;
    
    // Order queue (reference counted)
    std::deque<Order*> _orders;
    
    // Action queue (owned, deleted when processed)
    std::deque<Action*> _actionQueue;
    
    // Visual effects
    std::vector<std::unique_ptr<Effect>> _effects;
    
    // Identity
    std::string _iconName;
    std::string _name;
    long _id;
    
    // Combat
    int _health;                         // 0-100
    Object* _currentTarget;
    Target::Type _currentTargetType;
    Direction _currentHeading;
    
    // State
    bool _moving;
    bool _pathComplete;
    bool _bSquadLeader;
    bool _bHighlight;
    Color _highlightColor;
    
    // Relationships
    Squad* _currentSquad;
    int _currentTeamID;
    Element* _currentTileElement;
};
```

#### 2.1.3 Selection Mechanism

**Contains(x, y)** - Bounds-based hit testing:

```cpp
bool Object::Contains(int x, int y) {
    Region r;
    r.points[0].x = _minBounds.x0; r.points[0].y = _minBounds.y0;
    r.points[1].x = _minBounds.x1; r.points[1].y = _minBounds.y0;
    r.points[2].x = _minBounds.x1; r.points[2].y = _minBounds.y1;
    r.points[3].x = _minBounds.x0; r.points[3].y = _minBounds.y1;
    return Screen::PointInRegion(x, y, &r);
}
```

Uses ray casting algorithm: cast horizontal ray from point to right, count edge crossings.

#### 2.1.4 Order Queue Management

Orders use reference counting for memory management:

```cpp
void Object::AddOrder(Order* o) {
    o->IncrementRefCount();  // Take ownership
    _orders.push_back(o);
}

void Object::ClearOrders() {
    while(!_orders.empty()) {
        Order* o = _orders.front();
        _orders.pop_front();
        o->Release();  // Release ownership, delete if ref==0
    }
}
```

**CRITICAL**: Never add the same order to multiple objects without proper reference counting.

### 2.2 Soldier Class

**Location**: `src/objects/Soldier.h`, `src/objects/Soldier.cpp`

The `Soldier` class represents individual infantry units with complex animation states, attributes, and weapon handling.

#### 2.2.1 Animation States

Soldiers have 16 animation states mapped from 22 logical states:

```cpp
enum class AnimationState {
    Standing = 0,        // Idle standing
    Prone,               // Lying prone
    Walking,             // Walking animation
    Sneaking,            // Crawling/sneaking
    Running,             // Running animation
    StandingFiring,      // Firing while standing
    ProneFiring,         // Firing while prone
    StandingReloading,   // Reload stand
    ProneReloading,      // Reload prone
    DyingBlownUp,        // Death by explosion
    DyingBackward,       // Falling backward
    DyingForward,        // Falling forward
    Dead,                // Static corpse
    StandingUp,          // Prone to standing transition
    LyingDown,           // Standing to prone (reverse of StandingUp)
    NumStates
};
```

#### 2.2.2 Soldier States

Logical states (bitfield, 22 total):

```cpp
namespace SoldierState {
    enum class State {
        Standing = 0,           // Upright
        Prone,                  // On ground
        Stopped,                // Not moving
        Moving,                 // In motion
        Firing,                 // Currently firing
        Walking,                // Walking
        WalkingSlow,            // Slow walk
        Crawling,               // Prone movement
        Running,                // Running
        Reloading,              // Reloading weapon
        DyingBlownUp,           // Death animation
        DyingBackward,          // Death animation
        DyingForward,           // Death animation
        Dead,                   // Deceased
        Reloaded,               // Has ammo
        OutOfAmmo,              // Empty
        NoTarget,               // No valid target
        FindingCover,           // Seeking cover
        Following,              // Following leader
        FollowingInFormation,   // Formation movement
        Defending,              // Defensive posture
        Ambushing               // Ambush mode
    };
}
```

#### 2.2.3 Attributes

All attributes use the `Rating` class (0-100, default 50):

```cpp
struct {
    Rating Aggressiveness;   // Attack intensity preference
    Rating Leadership;       // Command influence radius
    Rating Charisma;         // Morale influence
    Rating Knowledge;        // Tactical skill
    Rating Experience;       // Combat experience
    Rating Intelligence;     // Learning rate
    Rating Morale;           // Mental health
    Rating Stamina;          // Physical endurance
} Attributes;
```

#### 2.2.4 Weapon System

```cpp
constexpr int MAX_WEAPONS_PER_SOLDIER = 8;

std::array<Weapon*, MAX_WEAPONS_PER_SOLDIER> _weapons;
std::array<int, MAX_WEAPONS_PER_SOLDIER> _weaponsNumClips;
int _currentWeaponIdx;
int _numWeapons;
```

Weapons are equipped from XML templates and can be fired, reloaded, and depleted.

#### 2.2.5 Movement Physics

```cpp
Vector2 _velocity;       // Current velocity vector
Vector2 _position;       // Floating-point position

// Acceleration values (loaded from XML)
float _runningAccel;
float _walkingAccel;
float _walkingSlowAccel;
float _crawlingAccel;
```

**Movement formula**:
```cpp
// Accelerate toward heading
velocity.x -= accel * dt * sin(headingAngle) / 1000.0f;
velocity.y += accel * dt * cos(headingAngle) / 1000.0f;

// Cap at max speed
if(velocity.Magnitude() > maxSpeed) {
    velocity.Normalize();
    velocity.Multiply(maxSpeed);
}

// Update position
_position.x += velocity.x * dt * PixelsPerMeter / 1000.0f;
_position.y += velocity.y * dt * PixelsPerMeter / 1000.0f;
```

#### 2.2.6 Action Handlers

22 action types with dedicated handlers:

```cpp
std::array<SoldierActionHandlers::SoldierActionHandler, static_cast<size_t>(SoldierAction::NumActions)> _actionHandlers;
```

Handlers include: StandingFire, ProneFire, Run, Walk, WalkSlow, Crawl, Stand, LieDown, Stop, DestinationReached, Reload, FindCover, Follow, FollowInFormation, WalkTo, RunTo, WalkSlowTo, CrawlTo, Turn, Defend, Ambush, Wait.

#### 2.2.7 Path Following

```cpp
void FollowPath(Path* path, SoldierAction::Action movementStyle);
```

Converts path waypoints to movement actions:
1. Stop current movement
2. Wait briefly (reaction time)
3. Add movement action for each path node
4. Add DestinationReached and Stop actions

#### 2.2.8 Formation Following

```cpp
void Follow(Object* object, Formation::Type formationType, 
            float formationSpread, int formationIdx, 
            SoldierAction::Action movementStyle);
```

Follows a leader object while maintaining formation position.

#### 2.2.9 Combat Actions

```cpp
void Shoot(Weapon* weapon, Object* target, Target::Type targetType, 
           int targetX, int targetY);
bool CalculateShot(Soldier* shooter, Weapon* weapon);
Soldier* FindTarget(Squad* squad);
```

#### 2.2.10 Death Handling

```cpp
void Kill();  // Trigger death
bool IsDead();  // Check death state
```

Randomly selects one of three death animations (DyingBlownUp, DyingBackward, DyingForward) and plays death sound.

### 2.3 Vehicle Class

**Location**: `src/objects/Vehicle.h`, `src/objects/Vehicle.cpp`

The `Vehicle` class represents armored vehicles with turret rotation, crew management, and distinct weapon mounting positions.

#### 2.3.1 Vehicle States

```cpp
enum class State {
    Stopped = 0,
    Moving,
    Firing,
    NumStates
};
```

#### 2.3.2 Hull/Turret Rotation

```cpp
float _currentHullAngle;        // Hull facing (radians)
float _currentTurretAngle;      // Turret facing (radians)

// Rotation parameters (ms to rotate 22.5 degrees)
int _turretRotationRate;        // Turret rotation speed
int _hullRotationRate;          // Hull rotation speed

// Rotation state
bool _turretRotating;
bool _hullRotating;
float _turretTargetAngle;
float _hullTargetAngle;
float _turretRotationDirection; // 1.0 or -1.0
float _hullRotationDirection;
```

**Rotation physics**:
```cpp
_currentTurretAngle += _turretRotationDirection * dt * 2PI / (16 * _turretRotationRate);

// Snap when close
if(_currentTurretAngle <= (_turretTargetAngle + DA) && 
   _currentTurretAngle >= (_turretTargetAngle - DA)) {
    _turretRotating = false;
    _currentTurretAngle = _turretTargetAngle;
}
```

#### 2.3.3 Graphics

```cpp
TGA* _hullGraphics;     // Hull sprite
TGA* _turretGraphics;   // Turret sprite
TGA* _wreckGraphics;    // Destroyed sprite

Point _turretPosition;  // Turret offset on hull
Point _muzzlePosition;  // Muzzle flash offset
```

#### 2.3.4 Crew Management

```cpp
constexpr int MAX_CREW = 8;
constexpr int MAX_WEAPONS_PER_VEHICLE = 8;

struct CrewSlot {
    Soldier* soldier;
    int weaponSlot;
};

std::array<CrewSlot, MAX_CREW> _crew;
int _numCrew;
```

**Crew assignment**:
```cpp
void AddCrew(Soldier* soldier, int slot);
```

Crew members receive copies of vehicle weapons to fire.

#### 2.3.5 Weapon Mounting

```cpp
std::array<Weapon*, MAX_WEAPONS_PER_VEHICLE> _weapons;
std::array<bool, MAX_WEAPONS_PER_VEHICLE> _weaponIsOnHull;  // true=hull, false=turret
```

Weapons can only fire when their mount is aligned:
- Hull weapons: require hull aligned to target
- Turret weapons: require turret aligned to target

#### 2.3.6 Movement

Vehicles must align hull to destination before moving:

```cpp
Point _destination;       // Final destination
Point _shortDestination;  // Intermediate waypoint

void PlanMovement(long dt);
```

Different physics from infantry:
```cpp
// Accelerate in heading direction
_velocity.x += -_acceleration * secs * sin(_currentHullAngle);
_velocity.y += -_acceleration * secs * cos(_currentHullAngle);

// Cap at max road speed
if(_velocity.Magnitude() > _maxRoadSpeed) {
    _velocity.Normalize();
    _velocity.Multiply(_maxRoadSpeed);
}
```

#### 2.3.7 Turret Aiming

```cpp
void AimTurret(int x, int y);      // Aim at world position
void AimTurret(Direction dir);      // Aim in cardinal direction
```

Calculates shortest rotation direction and sets target angles for both hull and turret.

#### 2.3.8 Combat Implementation Status

**Current Status**: Vehicle combat is partially implemented (as of SDL2 port).

**What Works**:
- Vehicles can fire weapons and show visual effects (muzzle flashes, explosions)
- Turret and hull rotation for aiming
- Target acquisition (soldiers only)
- Weapon cycling and ammo depletion

**What Is Disabled**:
- **Vehicle -> Soldier damage**: The `CalculateShot()` method was never implemented for vehicles
- **Vehicle -> Vehicle damage**: Not implemented
- **Soldier -> Vehicle targeting**: No `Target::Vehicle` case in `Soldier::Shoot()`

**Historical Context** (from src/objects/Vehicle.cpp:421-438):
```cpp
/*
 * DISABLED CODE: Vehicle damage calculation
 *
 * This code was already disabled in the original DirectX/Windows codebase from 2005.
 * The g_World->CalculateShot() function never existed in the World class.
 *
 * Current status: Vehicle weapons can fire and show effects, but do no damage.
 *
 * To implement vehicle combat properly:
 * 1. Add CalculateShot() method to Vehicle class (similar to Soldier::CalculateShot)
 * 2. Design vehicle damage model (armor values, hit locations, penetration)
 * 3. Add proper target acquisition and damage application
 *
 * See docs/VEHICLE_COMBAT_IMPLEMENTATION_PLANS.md for detailed analysis.
 */
```

**Implementation Options**:
See `docs/VEHICLE_COMBAT_IMPLEMENTATION_PLANS.md` for two proposed approaches:
1. **Minimal Implementation**: Simple health-based damage following existing soldier patterns
2. **Weapon Damage Types**: Add `<DamageType>` to weapons and `<Armor>` to vehicles for realistic penetration

### 2.4 Squad Class

**Location**: `src/objects/Squad.h`, `src/objects/Squad.cpp`

The `Squad` class manages groups of soldiers and vehicles as a single tactical unit.

#### 2.4.1 Quality Ratings

```cpp
enum class Quality {
    Useless = 0,
    Fragile,
    Weak,
    Average,
    Good,
    Strong,
    NumQuality
};
```

Quality affects unit performance and is randomly assigned on creation.

#### 2.4.2 Composition

```cpp
std::vector<Soldier*> _soldiers;  // Non-owning references
std::vector<Vehicle*> _vehicles;  // Non-owning references
```

#### 2.4.3 Formation System

```cpp
Formation::Type _currentFormation;
float _currentFormationSpread;
int _formationPosition;
```

**Formation types**:
- `Formation::Column` - Single file
- `Formation::File` - Line abreast
- `Formation::Line` - Extended line

#### 2.4.4 Path Management

```cpp
Path* _currentPath;
int _currentPointManIdx;  // Leading soldier
```

Only the point man follows the path directly; others use formation following.

#### 2.4.5 Order Distribution

```cpp
void AddOrder(Order* o);
void HandleMoveOrder(MoveOrder* order, SoldierAction::Action movementStyle, 
                     Mark::Color markColor);
void HandleAmbushOrder(AmbushOrder* order);
void HandleDefendOrder(DefendOrder* order);
```

Different order types propagate differently:
- **Move/Fast/Sneak**: Squad calculates path, assigns FollowPath to point man, FollowInFormation to others
- **Ambush/Defend**: Squad handles soldier ordering directly
- **Fire**: Sets target for all members
- **Stop**: Clears orders at all levels

#### 2.4.6 Mark System

```cpp
bool _bShowMark;
Mark::Color _markColor;
```

Displays colored marks at movement destinations.

#### 2.4.7 Selection

Squad selection highlights all members:

```cpp
void Select(bool s);  // Also selects all soldiers in squad
void Highlight(Color* color);
void UnHighlight();
```

### 2.5 Object Ownership Summary

```
World (Owner)
├── _mobileObjects (owns Soldiers and Vehicles)
├── SquadManager (owns Squads)
│   └── Squad::_soldiers (references, non-owning)
│   └── Squad::_vehicles (references, non-owning)
├── WeaponManager (owns Weapon prototypes)
│   └── Weapons assigned to Soldiers/Vehicles (copies)
└── EffectManager (owns Effect prototypes)
    └── Effects cloned into Objects (owned by Object)

Object (owns)
├── _effects (unique_ptr)
└── _actionQueue (raw pointers, deleted when processed)

Order (reference counted)
└── Shared between objects, deleted when ref==0
```

**Key Rules**:
1. World owns all mobile objects (Soldiers, Vehicles)
2. SquadManager owns Squads
3. Squads reference but don't own soldiers/vehicles
4. Orders use reference counting - always call IncrementRefCount before adding to queue
5. Actions are owned by Object and deleted when popped from queue

---

*[Continue to Section 3: State Machine and Action System]*
## 3. State Machine and Action System

### 3.1 State Class - Bitfield Implementation

**Location**: `src/states/State.h`, `src/states/State.cpp`

The `State` class uses a 64-bit bitfield to efficiently track multiple boolean states simultaneously.

#### 3.1.1 Implementation

```cpp
class State {
private:
    unsigned long long _bits;  // 64 bits = 64 possible states
    
public:
    State() : _bits(0) {}
    
    // Check if a specific bit is set
    bool IsSet(unsigned int state) {
        uint64_t flag = 1ULL << state;
        return (_bits & flag) != 0;
    }
    
    // Set a specific bit
    void Set(unsigned int state) {
        uint64_t flag = 1ULL << state;
        _bits |= flag;
    }
    
    // Clear a specific bit
    void UnSet(unsigned int state) {
        uint64_t flag = 1ULL << state;
        _bits &= ~flag;
    }
};
```

#### 3.1.2 Bit Layout

```
Bit Position:  63 62 61 ... 22 21 20 ... 2  1  0
               |  |  |      |  |  |      |  |  |
State:         -- reserved --  Wt Am De ... Pr St

Example states:
Bit 0  (0x0001): Standing
Bit 1  (0x0002): Prone
Bit 2  (0x0004): Stopped
Bit 3  (0x0008): Moving
Bit 4  (0x0010): Firing
Bit 21 (0x200000): Waiting
```

**Example operations**:
```cpp
State state;
state.Set(SoldierState::Standing);    // _bits = 0b0001
state.Set(SoldierState::Stopped);     // _bits = 0b0101
state.IsSet(SoldierState::Prone);     // false
state.UnSet(SoldierState::Standing);  // _bits = 0b0100
```

### 3.2 Soldier States

**Location**: `src/states/ObjectStates.h`, `config/SoldierStates.txt`

States are defined in `SoldierStates.txt` (1-indexed in file, 0-indexed in code):

```
1:  Standing             (0)  - Upright posture
2:  Prone                (1)  - Lying on ground
3:  Stopped              (2)  - Not moving
4:  Moving               (3)  - Currently in motion
5:  Firing               (4)  - Currently firing weapon
6:  Walking              (5)  - Walking movement
7:  WalkingSlow          (6)  - Slow walking
8:  Crawling             (7)  - Crawling/sneaking
9:  Running              (8)  - Running movement
10: Reloading            (9)  - Reloading weapon
11: DyingBlownUp         (10) - Death by explosion
12: DyingBackward        (11) - Falling backward
13: DyingForward         (12) - Falling forward
14: Dead                 (13) - Fully dead
15: Reloaded             (14) - Weapon has ammo
16: OutOfAmmo            (15) - No ammunition
17: NoTarget             (16) - No valid target
18: FindingCover         (17) - Seeking cover
19: Following            (18) - Following leader
20: FollowingInFormation (19) - Formation following
21: Defending            (20) - Defensive posture
22: Ambushing            (21) - Ambush mode
23: Waiting              (22) - Paused/delay
```

**C++ Enum**:
```cpp
namespace SoldierState {
    enum State {
        Standing = 0,
        Prone = 1,
        Stopped = 2,
        Moving = 3,
        Firing = 4,
        Walking = 5,
        WalkingSlow = 6,
        Crawling = 7,
        Running = 8,
        Reloading = 9,
        DyingBlownUp = 10,
        DyingBackward = 11,
        DyingForward = 12,
        Dead = 13,
        Reloaded = 14,
        OutOfAmmo = 15,
        NoTarget = 16,
        FindingCover = 17,
        Following = 18,
        FollowingInFormation = 19,
        Defending = 20,
        Ambushing = 21,
        Waiting = 22
    };
}
```

### 3.3 Action System

**Location**: `src/states/Action.h`, `src/states/ObjectActions.h/cpp`, `src/objects/SoldierActionHandlers.h/cpp`

#### 3.3.1 Action Structure

```cpp
struct Action {
    std::string Name;           // Human-readable name
    std::string Group;          // Category (Fire, Move, etc.)
    long Time;                  // Duration (-1=animation-based, 0=instant)
    
    // State requirements (ALL must be satisfied)
    int* Requirements;
    int NumRequirements;
    
    // States to add
    int* Adds;
    int NumAdds;
    
    // States to remove
    int* Subtracts;
    int NumSubtracts;
    
};
```

#### 3.3.2 Action Definitions (SoldierActions.txt)

Tab-separated format:
```
Name                    Group   Time    Requirements                    Changes
--------------------------------------------------------------------------------
StandingFire            Fire    0       Stopped,Standing,Reloaded       +Firing,-NoTarget
ProneFire               Fire    0       Stopped,Prone,Reloaded          +Firing,-NoTarget
Run                     Move    0       Standing                        +Running,-Walking,-WalkingSlow,-Crawling,-Stopped,+Moving,-Firing
Walk                    Move    0       Standing                        +Walking,-Running,-WalkingSlow,-Crawling,-Stopped,+Moving,-Firing
WalkSlow                Move    0       Standing                        +WalkingSlow,-Walking,-Running,-Crawling,-Stopped,+Moving,-Firing
Crawl                   Move    0       Prone                           +Crawling,-Running,-Walking,-WalkingSlow,-Stopped,+Moving,-Firing
Stand                   Move    -1      Stopped,Prone                   -Prone,+Standing,-Firing
LieDown                 Move    -1      Stopped,Standing                -Standing,+Prone,-Firing
Stop                    Move    0       nil                             -Moving,+Stopped,-Firing,-Running,-Walking,-WalkingSlow,-Crawling,-FindingCover,-Following,-FollowingInFormation,-Defending,-Ambushing
DestinationReached      Move    0       Stopped                         nil
Reload                  Fire    0       nil                             +Reloaded
FindCover               Move    0       nil                             +FindingCover
Follow                  Move    0       nil                             +Following
FollowInFormation       Move    0       nil                             +FollowingInFormation
WalkTo                  Move    0       Standing                        +Walking,-Running,-WalkingSlow,-Crawling,-Stopped,+Moving,-Firing
RunTo                   Move    0       Standing                        +Running,-Walking,-WalkingSlow,-Crawling,-Stopped,+Moving,-Firing
WalkSlowTo              Move    0       Standing                        +WalkingSlow,-Walking,-Running,-Crawling,-Stopped,+Moving,-Firing
CrawlTo                 Move    0       Prone                           +Crawling,-Running,-Walking,-WalkingSlow,-Stopped,+Moving,-Firing
Turn                    Move    0       nil                             nil
Defend                  Fire    0       Prone                           +Defending
Ambush                  Fire    0       Prone                           +Ambushing
Wait                    Move    0       nil                             nil
```

**Modern C++ Note**: When queuing actions, always use `nullptr` instead of `NULL`:
```cpp
// Modern C++
soldier->_actionQueue.push_front(new Action(prereqAction, nullptr));

// NOT this (old style)
// soldier->_actionQueue.push_front(new Action(prereqAction, NULL));
```

**Column meanings**:
- **Name**: Unique action identifier
- **Group**: Logical category (Fire, Move)
- **Time**: Duration in ms (-1=animation-based, 0=instant)
- **Requirements**: Required states (comma-separated, "nil"=none)
- **Changes**: State modifications (+X adds, -X removes)

#### 3.3.3 Action Types (Enum)

```cpp
namespace SoldierAction {
    enum class Action {
        StandingFire = 0,       // 0
        ProneFire,              // 1
        Run,                    // 2
        Walk,                   // 3
        WalkSlow,               // 4
        Crawl,                  // 5
        Stand,                  // 6
        LieDown,                // 7
        Stop,                   // 8
        DestinationReached,     // 9
        Reload,                 // 10
        FindCover,              // 11
        Follow,                 // 12
        FollowInFormation,      // 13
        WalkTo,                 // 14
        RunTo,                  // 15
        WalkSlowTo,             // 16
        CrawlTo,                // 17
        Turn,                   // 18
        Defend,                 // 19
        Ambush,                 // 20
        Wait,                   // 21
        NumActions              // 22
    };
}
```

#### 3.3.4 Action Loading Process

**Location**: `src/states/SoldierActionLoader.cpp`

```cpp
void SoldierActionLoader::Load(const std::filesystem::path& fileName) {
    // Open file
    std::ifstream file(fileName);
    
    // Skip header line
    std::string line;
    std::getline(file, line);
    
    // Parse each line
    while(std::getline(file, line)) {
        // Split by tabs
        std::vector<std::string> fields = Split(line, '\t');
        
        // Parse fields
        std::string name = Trim(fields[0]);
        std::string group = Trim(fields[1]);
        long time = std::stol(fields[2]);
        
        // Parse requirements
        std::vector<std::string> reqTokens = Split(fields[3], ',');
        for(auto& token : reqTokens) {
            if(token != "nil") {
                action.Requirements[action.NumRequirements++] = 
                    StringToState(token);
            }
        }
        
        // Parse changes
        std::vector<std::string> changeTokens = Split(fields[4], ',');
        for(auto& token : changeTokens) {
            if(token[0] == '+') {
                action.Adds[action.NumAdds++] = 
                    StringToState(token.substr(1));
            } else if(token[0] == '-') {
                action.Subtracts[action.NumSubtracts++] = 
                    StringToState(token.substr(1));
            }
        }
        
        // Store action
        AddAction(action);
    }
}
```

#### 3.3.5 Action Handler System

**Location**: `src/objects/SoldierActionHandlers.cpp`

Handler function signature:
```cpp
typedef bool (*SoldierActionHandler)(Soldier* soldier, Action* action, long dt);
// Returns: true = action complete, false = action continues
```

Handler array:
```cpp
SoldierActionHandler SoldierActionHandlers::_handlers[NumActions] = {
    StandingFireActionHandler,    // 0
    ProneFireActionHandler,       // 1
    RunActionHandler,             // 2
    WalkActionHandler,            // 3
    WalkSlowActionHandler,        // 4
    CrawlActionHandler,           // 5
    StandActionHandler,           // 6
    LieDownActionHandler,         // 7
    StopActionHandler,            // 8
    DestinationReachedActionHandler, // 9
    ReloadActionHandler,          // 10
    FindCoverActionHandler,       // 11
    FollowActionHandler,          // 12
    FollowInFormationActionHandler, // 13
    WalkToActionHandler,          // 14
    RunToActionHandler,           // 15
    WalkSlowToActionHandler,      // 16
    CrawlToActionHandler,         // 17
    TurnActionHandler,            // 18
    DefendActionHandler,          // 19
    AmbushActionHandler,          // 20
    WaitActionHandler             // 21
};
```

#### 3.3.6 Main Handler Dispatch

```cpp
bool SoldierActionHandlers::Handle(Soldier* soldier, Action* action, long dt) {
    // Check prerequisites
    int prereqAction = g_Globals->World.Actions.Soldiers.CheckRequirements(
        action->Index, &(soldier->_currentState));
    
    if(prereqAction >= 0) {
        // Missing requirement - prepend prerequisite
        soldier->_actionQueue.push_front(new Action(prereqAction, NULL));
        return false;
    }
    
    // Call specific handler
    return _handlers[action->Index](soldier, action, dt);
}
```

#### 3.3.7 Requirements Checking

```cpp
int ObjectActions::CheckRequirements(int actionID, State* state) {
    Action* action = &Actions[actionID];
    
    // Check each required state
    for(int i = 0; i < action->NumRequirements; i++) {
        if(!state->IsSet(action->Requirements[i])) {
            // Find action that adds this state
            for(int j = 0; j < NumActions; j++) {
                for(int k = 0; k < Actions[j].NumAdds; k++) {
                    if(action->Requirements[i] == Actions[j].Adds[k]) {
                        return j;  // Return prerequisite action
                    }
                }
            }
        }
    }
    return -1;  // All requirements satisfied
}
```

#### 3.3.8 State Updates

```cpp
void ObjectActions::UpdateState(int actionID, State* state) {
    // Apply all additions
    for(int i = 0; i < Actions[actionID].NumAdds; i++) {
        state->Set(Actions[actionID].Adds[i]);
    }
    
    // Apply all removals
    for(int i = 0; i < Actions[actionID].NumSubtracts; i++) {
        state->UnSet(Actions[actionID].Subtracts[i]);
    }
}
```

### 3.4 Key Handler Implementations

#### 3.4.1 Stand/LieDown Handlers (Animation-Based)

```cpp
bool StandActionHandler(Soldier* soldier, Action* action, long dt) {
    if(soldier->_currentAnimationState == Soldier::AnimationState::StandingUp) {
        // Check if animation finished
        int currentFrame = soldier->_animations[soldier->_currentHeading]
            ->GetCurrentFrameNumber(soldier->_currentHeading);
            
        if(soldier->_currentFrameCurrentState == currentFrame && 
           !soldier->_currentAnimationMarker) {
            // Animation complete
            g_Globals->World.Actions.Soldiers.UpdateState(
                action->Index, &soldier->_currentState);
            soldier->_currentAnimationState = Soldier::AnimationState::Standing;
            return true;
        }
    } else {
        // Start animation
        soldier->_currentAnimationState = Soldier::AnimationState::StandingUp;
        soldier->_animations[soldier->_currentAnimationState]->Reset();
        soldier->_currentFrameCurrentState = 
            soldier->_animations[soldier->_currentAnimationState]
                ->GetCurrentFrameNumber(soldier->_currentHeading);
        soldier->_currentAnimationMarker = true;
    }
    return false;
}
```

**Reverse Animation** (LyingDown):
```cpp
bool LieDownActionHandler(Soldier* soldier, Action* action, long dt) {
    // Same structure but uses StandingUp with SetReverse(true)
    soldier->_animations[Soldier::AnimationState::StandingUp]->SetReverse(true);
    // ... rest same as StandActionHandler
}
```

#### 3.4.2 Movement Handlers (WalkTo, RunTo, etc.)

```cpp
bool WalkToActionHandler(Soldier* soldier, Action* action, long dt) {
    TileData* data = (TileData*)action->Data;
    
    // Update state
    soldier->_moving = true;
    soldier->_currentAnimationState = Soldier::AnimationState::Walking;
    g_Globals->World.Actions.Soldiers.UpdateState(
        action->Index, &soldier->_currentState);
    
    // Check if at destination
    if(AtDestination(soldier, data->TileI, data->TileJ)) {
        delete data;
        action->Data = NULL;
        return true;
    }
    
    // Calculate heading
    Direction newHeading = CalculateNewHeading(
        soldier, data->TileI, data->TileJ);
    if(soldier->_currentHeading != newHeading) {
        soldier->_velocity.x = 0;
        soldier->_velocity.y = 0;
        soldier->_currentHeading = newHeading;
    }
    
    // Move
    MoveSoldier(soldier, dt);
    return false;
}
```

#### 3.4.3 Stop Handler

```cpp
bool StopActionHandler(Soldier* soldier, Action* action, long dt) {
    // Update state machine
    g_Globals->World.Actions.Soldiers.UpdateState(
        action->Index, &soldier->_currentState);
    
    // Reset physics
    soldier->_velocity.x = 0;
    soldier->_velocity.y = 0;
    soldier->_moving = false;
    
    return true;  // Always completes immediately
}
```

#### 3.4.4 Fire Handlers

```cpp
bool ProneFireActionHandler(Soldier* soldier, Action* action, long dt) {
    FireActionData* fireData = (FireActionData*)action->Data;
    
    // Update state
    g_Globals->World.Actions.Soldiers.UpdateState(
        action->Index, &soldier->_currentState);
    soldier->_currentAnimationState = Soldier::AnimationState::ProneFiring;
    
    if(soldier->_weapons[soldier->_currentWeaponIdx]->CanFire()) {
        switch(fireData->TargetType) {
        case Target::Soldier:
            if(fireData->TargetObject != NULL) {
                soldier->_currentHeading = Utilities::FindHeading(
                    soldier->Position.x, soldier->Position.y,
                    fireData->TargetObject->Position.x,
                    fireData->TargetObject->Position.y);
                    
                ((Soldier*)fireData->TargetObject)->CalculateShot(
                    soldier, soldier->_weapons[soldier->_currentWeaponIdx]);
            }
            break;
            
        case Target::Squad:
            fireData->TargetObject = soldier->FindTarget(
                (Squad*)fireData->TargetObject);
            fireData->TargetType = Target::Soldier;
            break;
            
        case Target::Area:
            soldier->_currentHeading = Utilities::FindHeading(
                soldier->Position.x, soldier->Position.y,
                fireData->X, fireData->Y);
            break;
        }
        
        soldier->_weapons[soldier->_currentWeaponIdx]->Fire();
        
        // Add muzzle flash effect
        soldier->_effects.push_back(std::unique_ptr<Effect>(
            g_Globals->World.Effects->GetEffect(
                soldier->_weapons[soldier->_currentWeaponIdx]->GetEffect(
                    soldier->_currentHeading))));
    }
    return false;  // Continue firing
}
```

### 3.5 Action Data Structures

#### 3.5.1 TileData (Movement Actions)

```cpp
struct TileData {
    int TileI;  // Target tile column
    int TileJ;  // Target tile row
};
```

Used by: WalkTo, RunTo, WalkSlowTo, CrawlTo

#### 3.5.2 FireActionData (Fire Actions)

```cpp
struct FireActionData {
    Object* TargetObject;   // Target entity (nullptr for area fire)
    Target::Type TargetType; // Soldier, Squad, Vehicle, Area
    int X, Y;               // Area target coordinates
};
```

Used by: StandingFire, ProneFire

#### 3.5.3 FollowFormationData (Formation Following)

```cpp
struct FollowFormationData {
    Object* TargetObject;           // Leader to follow
    Formation::Type TargetFormation; // Formation type
    int FormationIndex;              // Position in formation
    float FormationSpread;           // Distance between units
    SoldierAction::Action MovementStyle; // Walk/Run/Crawl
};
```

Used by: FollowInFormation

### 3.6 Complete Action Execution Flow

```
1. User issues order (e.g., Move)
   |
2. Order converted to Action
   └── MoveOrder -> WalkTo action with TileData
   |
3. Action added to _actionQueue
   |
4. Each frame, Soldier::Simulate() processes queue
   |
5. SoldierActionHandlers::Handle() called
   ├── CheckRequirements() - prerequisites met?
   │   └── If not: prepend prerequisite action
   │       └── Example: WalkTo requires Standing
   │           └── If Prone: prepend Stand action
   │               └── Stand requires Stopped
   │                   └── If Moving: prepend Stop action
   │                       └── Stop has no requirements
   │
6. Handler called
   ├── UpdateState() - apply state changes
   ├── Update animation state
   ├── Perform action logic
   └── Return: true (complete) or false (continue)
   |
7. If complete
   └── _actionQueue.pop_front()
   └── delete action
   |
8. If queue empty
   └── Soldier idle
```

### 3.7 Animation Marker System

For animation-based actions (Stand, LieDown), a marker tracks completion:

```cpp
// Start of action
soldier->_currentAnimationMarker = true;
soldier->_currentFrameCurrentState = initialFrame;

// Each frame
if(soldier->_currentFrameCurrentState == currentFrame && 
   !soldier->_currentAnimationMarker) {
    // Animation has looped (completed for reverse) or advanced
    // This indicates completion
} else if(soldier->_currentFrameCurrentState != currentFrame) {
    // Frame changed - clear marker
    soldier->_currentAnimationMarker = false;
}
```

### 3.8 State Transition Examples

**Example 1: Standing up and firing**
```
Initial state: Prone, Stopped, Reloaded

1. User orders fire while prone
   └── ProneFire action queued
   
2. Check requirements: Stopped ✓, Prone ✓, Reloaded ✓
   └── All met, execute ProneFire
   
3. UpdateState(ProneFire)
   └── +Firing, -NoTarget
   └── New state: Prone, Stopped, Reloaded, Firing
```

**Example 2: Complex transition (Prone -> Running)**
```
Initial state: Prone, Stopped, Reloaded

1. User orders MoveFast
   └── RunTo action queued
   
2. Check requirements: Standing ✗ (we have Prone)
   └── Missing: Standing
   └── Find action that adds Standing: Stand
   └── Prepend Stand to queue
   
3. Next frame: Stand action
   └── Check requirements: Stopped ✓, Prone ✓
   └── Start StandingUp animation
   └── Return false (continues)
   
4. Animation completes
   └── UpdateState(Stand)
   └── -Prone, +Standing
   └── New state: Standing, Stopped, Reloaded
   └── Return true (complete)
   
5. Next action: RunTo
   └── Check requirements: Standing ✓
   └── UpdateState(RunTo)
   └── +Running, +Moving, -Stopped, etc.
   └── New state: Standing, Running, Moving, Reloaded
```

---

*[Continue to Section 4: Order System and AI Pathfinding]*
## 4. Order System and AI Pathfinding

### 4.1 Order Base Class

**Location**: `src/orders/Order.h`, `src/orders/Order.cpp`

The `Order` class is the base for all high-level commands issued to units.

#### 4.1.1 Implementation

```cpp
namespace Orders {
    enum OrderType {
        Move,           // Standard walking movement
        MoveFast,       // Running movement
        Sneak,          // Crawling/slow movement
        Fire,           // Attack target
        Ambush,         // Set directional ambush
        Hide,           // Take cover (placeholder)
        Smoke,          // Deploy smoke (placeholder)
        Destination,    // Final destination marker
        Stop,           // Halt all movement
        Pause,          // Temporary wait
        Defend          // Defensive facing
    };
}

class Order {
public:
    Order(void);
    virtual ~Order(void);
    
    inline Orders::OrderType GetType() { return _orderType; }
    
    // Reference counting
    inline void IncrementRefCount() { ++_refCount; }
    inline void Release() { 
        --_refCount; 
        if(_refCount <= 0) { 
            delete this; 
        } 
    }

protected:
    Orders::OrderType _orderType;

private:
    int _refCount;  // Starts at 0
};
```

#### 4.1.2 Reference Counting Pattern

**CRITICAL**: Orders use manual reference counting to handle shared ownership.

```cpp
// CORRECT usage
Order* order = new MoveOrder(x, y, Orders::Move);
soldier->AddOrder(order);  // refCount = 1

// To share with another object:
order->IncrementRefCount();  // refCount = 2
otherSoldier->AddOrder(order);

// When processed:
order->Release();  // refCount decrements, deleted when 0
```

**Bug prevention**: Never add the same order to multiple objects without incrementing ref count. The "Infantry Firing Bug" was caused by double-free when the same order was added to multiple selected objects.

### 4.2 Order Types

#### 4.2.1 MoveOrder

**Location**: `src/orders/MoveOrder.h`

```cpp
class MoveOrder : public Order {
public:
    MoveOrder(int x, int y, Orders::OrderType type);
    
    int X;  // Destination X (world pixels)
    int Y;  // Destination Y (world pixels)
};
```

Used for: Move, MoveFast, Sneak (distinguished by type parameter)

**Coordinate system**: World pixel coordinates (not tile indices)

#### 4.2.2 FireOrder

**Location**: `src/orders/FireOrder.h`

```cpp
class FireOrder : public Order {
public:
    // Target object
    FireOrder(Object* target, Target::Type targetType);
    
    // Area target (ground position)
    FireOrder(int x, int y);
    
    Object* Target;           // Target entity (NULL for area fire)
    Target::Type TargetType;  // Soldier, Squad, Vehicle, Area
    int X, Y;                 // Target coordinates (for Area type)
};
```

**Target types**:
```cpp
namespace Target {
    enum Type {
        Soldier,        // Individual soldier
        Squad,          // Squad (picks random member)
        Vehicle,        // Vehicle target
        Area,           // Ground position
        NoTarget,       // No target assigned
        NumTargetTypes
    };
}
```

#### 4.2.3 AmbushOrder

**Location**: `src/orders/AmbushOrder.h`

```cpp
class AmbushOrder : public Order {
public:
    AmbushOrder(Direction dir);
    
    Direction Heading;  // Facing direction for ambush arc
};
```

Soldiers in ambush mode will automatically fire at enemies entering the designated direction.

#### 4.2.4 DefendOrder

**Location**: `src/orders/DefendOrder.h`

```cpp
class DefendOrder : public Order {
public:
    DefendOrder(Direction dir);
    
    Direction Heading;  // Defensive facing direction
};
```

#### 4.2.5 StopOrder

**Location**: `src/orders/StopOrder.h`

```cpp
class StopOrder : public Order {
public:
    StopOrder(void);  // Simple halt command
};
```

Immediately clears action queue and halts movement.

#### 4.2.6 PauseOrder

**Location**: `src/orders/PauseOrder.h`

```cpp
class PauseOrder : public Order {
public:
    PauseOrder(long pauseTime, int oldState, int pauseState);
    
    long _pauseTime;    // Duration in milliseconds
    long _totalTime;    // Accumulated time
    int _oldState;      // State to restore after pause
    int _pauseState;    // State during pause
    
    inline long GetPauseTime() { return _pauseTime; }
    inline long GetTotalTime() { return _totalTime; }
    inline void IncrementTotalTime(long dt) { _totalTime += dt; }
    inline int GetOldState() { return _oldState; }
    inline int GetPauseState() { return _pauseState; }
};
```

Used for brief delays (reaction times, weapon cycling).

### 4.3 Order Processing Flow

#### 4.3.1 Order Queue (Object base)

```cpp
std::deque<Order*> _orders;

void Object::AddOrder(Order* o) {
    o->IncrementRefCount();
    _orders.push_back(o);
}

void Object::ClearOrders() {
    while(!_orders.empty()) {
        Order* o = _orders.front();
        _orders.pop_front();
        o->Release();
    }
}
```

#### 4.3.2 Order Handling in Soldier::Simulate()

```cpp
void Soldier::Simulate(long dt, World* world) {
    // Process orders
    if(!_orders.empty()) {
        Order* order = _orders.front();
        bool handled = false;
        
        switch(order->GetType()) {
        case Orders::Destination:
            handled = HandleDestinationOrder((MoveOrder*)order);
            break;
        case Orders::Stop:
            handled = HandleStopOrder((StopOrder*)order);
            break;
        case Orders::Fire:
            handled = HandleFireOrder((FireOrder*)order);
            break;
        // Move, MoveFast, Sneak handled via actions
        default:
            handled = true;
            break;
        }
        
        if(handled) {
            _orders.pop_front();
            order->Release();
        }
    }
    
    // Process action queue
    if(!_actionQueue.empty()) {
        Action* action = _actionQueue.front();
        if(SoldierActionHandlers::Handle(this, action, dt)) {
            _actionQueue.pop_front();
            delete action;
        }
    }
}
```

#### 4.3.3 Order Handlers

**HandleDestinationOrder()**:
```cpp
bool Soldier::HandleDestinationOrder(MoveOrder* order) {
    Vector2 range;
    range.x = (float)(Position.x - order->X);
    range.y = (float)(Position.y - order->Y);
    
    // Within 5 pixels of destination
    if(range.Magnitude() < 5.01f) {
        AddOrder(new StopOrder());
        return true;
    }
    return false;
}
```

**HandleStopOrder()**:
```cpp
bool Soldier::HandleStopOrder(StopOrder* order) {
    Action* action = new Action();
    action->Index = SoldierAction::Stop;
    action->Data = NULL;
    
    _actionQueue.clear();
    _actionQueue.push_back(action);
    
    return true;
}
```

**HandleFireOrder()**:
```cpp
bool Soldier::HandleFireOrder(FireOrder* order) {
    Action* action = new Action();
    action->Index = SoldierAction::ProneFire;
    
    FireActionData* data = new FireActionData();
    data->TargetObject = order->Target;
    data->TargetType = order->TargetType;
    data->X = order->X;
    data->Y = order->Y;
    action->Data = data;
    
    HandleStopOrder(NULL);
    _actionQueue.push_back(action);
    
    return true;
}
```

#### 4.3.4 Squad-Level Order Distribution

```cpp
void Squad::AddOrder(Order* o) {
    _bShowMark = false;
    
    switch(o->GetType()) {
    case Orders::Ambush:
        HandleAmbushOrder((AmbushOrder*)o);
        for(auto* vehicle : _vehicles) vehicle->AddOrder(o);
        return;
        
    case Orders::Defend:
        HandleDefendOrder((DefendOrder*)o);
        for(auto* vehicle : _vehicles) vehicle->AddOrder(o);
        return;
        
    case Orders::Fire:
        FireOrder* f = (FireOrder*)o;
        _currentTarget = f->Target;
        _currentTargetType = f->TargetType;
        _bShowMark = true;
        break;
        
    case Orders::Move:
        HandleMoveOrder((MoveOrder*)o, SoldierAction::WalkTo, Mark::Blue);
        for(auto* vehicle : _vehicles) vehicle->AddOrder(o);
        return;
        
    case Orders::MoveFast:
        HandleMoveOrder((MoveOrder*)o, SoldierAction::RunTo, Mark::Purple);
        for(auto* vehicle : _vehicles) vehicle->AddOrder(o);
        return;
        
    case Orders::Sneak:
        HandleMoveOrder((MoveOrder*)o, SoldierAction::CrawlTo, Mark::Yellow);
        for(auto* vehicle : _vehicles) vehicle->AddOrder(o);
        return;
    }
    
    // Default: propagate to all members
    for(auto* vehicle : _vehicles) vehicle->AddOrder(o);
    for(auto* soldier : _soldiers) soldier->AddOrder(o);
}
```

**HandleMoveOrder()**:
```cpp
void Squad::HandleMoveOrder(MoveOrder* order, 
                            SoldierAction::Action movementStyle,
                            Mark::Color markColor) {
    // Convert world pixels to tile coordinates
    int i, j, di, dj;
    g_Globals->World.CurrentWorld->ConvertPositionToTile(
        Position.x, Position.y, &i, &j);
    g_Globals->World.CurrentWorld->ConvertPositionToTile(
        order->X, order->Y, &di, &dj);
    
    // Determine posture level for pathfinding
    Element::Level level = Element::Medium;
    switch(movementStyle) {
    case SoldierAction::CrawlTo:
        level = Element::Prone;
        break;
    case SoldierAction::RunTo:
        level = Element::High;
        break;
    case SoldierAction::WalkTo:
    default:
        level = Element::Medium;
        break;
    }
    
    // Calculate path once for entire squad
    _currentPath = g_Globals->World.Pathing.FindPath(i, j, di, dj, level);
    
    // Point man follows actual path
    _soldiers[_currentPointManIdx]->FollowPath(_currentPath, movementStyle);
    
    // Others follow point man in formation
    int formationIndex = 0;
    for(size_t idx = 0; idx < _soldiers.size(); idx++) {
        if(idx != _currentPointManIdx) {
            _soldiers[idx]->Follow(_soldiers[_currentPointManIdx],
                                   _currentFormation,
                                   _currentFormationSpread,
                                   formationIndex++,
                                   movementStyle);
        }
    }
    
    _bShowMark = true;
    _markColor = markColor;
    _bMarkTargetPosition = true;
}
```

### 4.4 A* Pathfinding System

**Location**: `src/ai/AStar.h`, `src/ai/AStar.cpp`

#### 4.4.1 Node Structure

```cpp
struct Node {
    int X, Y;               // Tile coordinates
    
    // A* heuristics
    float F, G, H;          // F = G + H
    
    // Path reconstruction
    Node* Parent;
    
    // MinHeap management
    int HeapIndex;
    
    // Hash table linking
    Node* HashNext;
    Node* HashPrev;
    
    // Memory pool linking
    Node* MemNext;
};
```

**Field meanings**:
- **G**: Cost from start to this node (actual cost)
- **H**: Heuristic to goal (estimated cost)
- **F**: Total cost (G + H)
- **Parent**: Previous node for path reconstruction
- **HeapIndex**: Position in priority queue

#### 4.4.2 MinHeap Implementation

```cpp
class MinHeap {
    #define Left(i)     ((i) << 1)       // i * 2
    #define Right(i)    (((i) << 1) + 1) // i * 2 + 1
    #define Parent(i)   ((i) >> 1)       // i / 2
    
    HeapNode** _heapNodes;
    int _size;
    int _last;
    
public:
    int Insert(Node* data, float priority);  // Returns index
    Node* ExtractMin();
    int GetNumNodes();
    void Clear();
    
private:
    void SiftDown(int i);
};
```

**Insert operation**:
```cpp
int MinHeap::Insert(Node* data, float priority) {
    HeapNode* newNode = AllocateHeapNode();
    newNode->Priority = priority;
    newNode->Data = data;
    
    ++_last;
    int i = _last;
    
    // Bubble up
    while((i > 1) && (_heapNodes[Parent(i)]->Priority >= newNode->Priority)) {
        _heapNodes[i] = _heapNodes[Parent(i)];
        _heapNodes[i]->Data->HeapIndex = i;
        i = Parent(i);
    }
    
    _heapNodes[i] = newNode;
    _heapNodes[i]->Data->HeapIndex = i;
    return i;
}
```

**ExtractMin operation**:
```cpp
Node* MinHeap::ExtractMin() {
    HeapNode* min = _heapNodes[1];  // Root has minimum F
    _heapNodes[1] = _heapNodes[_last];
    _heapNodes[1]->Data->HeapIndex = 1;
    --_last;
    SiftDown(1);
    
    Node* n = min->Data;
    FreeHeapNode(min);
    return n;
}
```

**SiftDown**:
```cpp
void MinHeap::SiftDown(int i) {
    int k, j, n;
    k = i;
    n = _last;
    
    do {
        j = k;
        // Check left child
        if((Left(j) <= n) && 
           (_heapNodes[Left(j)]->Priority <= _heapNodes[k]->Priority)) {
            k = Left(j);
        }
        // Check right child
        if((Right(j) <= n) && 
           (_heapNodes[Right(j)]->Priority <= _heapNodes[k]->Priority)) {
            k = Right(j);
        }
        
        // Swap if needed
        if(j != k) {
            HeapNode* temp = _heapNodes[j];
            _heapNodes[j] = _heapNodes[k];
            _heapNodes[k] = temp;
            _heapNodes[j]->Data->HeapIndex = j;
            _heapNodes[k]->Data->HeapIndex = k;
        }
    } while(j != k);
}
```

#### 4.4.3 Hash Table (Closed List)

```cpp
class Hash {
    Node** _nodes;
    int _size;  // 60013 (prime)
    
public:
    unsigned int HashFunction(int x, int y) {
        return (x << 16 | y) % _size;
    }
    
    void Insert(Node* node);
    Node* Remove(int x, int y);
    Node* Find(int x, int y);
    void Clear();
};
```

#### 4.4.4 AStar Class

```cpp
class AStar {
    MinHeap* _openNodes;     // Open list (priority queue)
    Hash* _closedNodes;      // Closed list (visited nodes)
    
    int _destX, _destY;      // Destination coordinates
    Element::Level _level;   // Movement posture level
    
    Node* _freeNodes;        // Memory pool
    long _nFreeNodes;
    long _nAllocatedNodes;
    
public:
    Path* FindPath(int x0, int y0, int x1, int y1, Element::Level level);
    
protected:
    Node* GetBestNode();
    void GenerateSuccessors(Node* node);
    bool CanMove(int x, int y);
    void DoMove(Node* node, int x, int y);
    float GetTerrainCost(int x, int y);
    float Heuristic(int x, int y);
    Node* AllocateNode();
    void FreeNode(Node* node);
};
```

#### 4.4.5 Pathfinding Algorithm

```cpp
Path* AStar::FindPath(int x0, int y0, int x1, int y1, Element::Level level) {
    _destX = x1;
    _destY = y1;
    _level = level;
    
    // Initialize
    _openNodes->Clear();
    _closedNodes->DeepClear();  // Deep clear needed for closed list
    
    // Create start node
    Node* start = AllocateNode();
    start->X = x0;
    start->Y = y0;
    start->G = 0;
    start->H = Heuristic(x0, y0);
    start->F = start->G + start->H;
    start->Parent = NULL;
    
    _openNodes->Insert(start, start->F);
    
    while(_openNodes->GetNumNodes() > 0) {
        Node* current = GetBestNode();
        
        // Check if reached goal
        if(current->X == _destX && current->Y == _destY) {
            return ReconstructPath(current);
        }
        
        // Move to closed list
        _closedNodes->Insert(current);
        
        // Generate successors
        GenerateSuccessors(current);
    }
    
    return NULL;  // No path found
}
```

#### 4.4.6 Successor Generation (8 Directions)

```cpp
void AStar::GenerateSuccessors(Node* node) {
    int x = node->X;
    int y = node->Y;
    
    // 8 neighboring tiles
    if(CanMove(x-1, y-1)) DoMove(node, x-1, y-1);  // Upper-left
    if(CanMove(x,   y-1)) DoMove(node, x,   y-1);  // Upper
    if(CanMove(x+1, y-1)) DoMove(node, x+1, y-1);  // Upper-right
    if(CanMove(x-1, y))   DoMove(node, x-1, y);    // Left
    if(CanMove(x+1, y))   DoMove(node, x+1, y);    // Right
    if(CanMove(x-1, y+1)) DoMove(node, x-1, y+1);  // Lower-left
    if(CanMove(x,   y+1)) DoMove(node, x,   y+1);  // Lower
    if(CanMove(x+1, y+1)) DoMove(node, x+1, y+1);  // Lower-right
}
```

#### 4.4.7 Movement Validation

```cpp
bool AStar::CanMove(int x, int y) {
    // Bounds check
    if(x < 0 || y < 0 || 
       x >= g_Globals->World.CurrentWorld->NumTiles.x ||
       y >= g_Globals->World.CurrentWorld->NumTiles.y) {
        return false;
    }
    
    // Passability check
    return g_Globals->World.CurrentWorld->IsPassable(x, y);
}
```

#### 4.4.8 Terrain Cost Calculation

```cpp
float AStar::GetTerrainCost(int x, int y) {
    Element* e = g_Globals->World.CurrentWorld->GetTileElement(x, y);
    // Return hindrance value for current posture level
    return (float)e->Hindrance[_level] / 100.0f;
}
```

**Element::Level mapping**:
- `Prone` (0): Lowest profile, highest hindrance penalties
- `Low` (1): Crouched
- `Medium` (2): Normal walking
- `High` (3): Running

#### 4.4.9 Heuristic Function

```cpp
float AStar::Heuristic(int x, int y) {
    // Octile distance (allows diagonal with proper cost)
    int dx = abs(x - _destX);
    int dy = abs(y - _destY);
    
    float diag = (float)std::min(dx, dy);
    float straight = (float)(dx + dy);
    
    // sqrt(2) ≈ 1.414 for diagonals, 1.0 for straight
    return 1.414f * diag + (straight - 2.0f * diag);
}
```

#### 4.4.10 Node Processing

```cpp
void AStar::DoMove(Node* parent, int x, int y) {
    // Calculate movement cost
    float cost = GetTerrainCost(x, y);
    
    // Diagonal movement costs more
    if(x != parent->X && y != parent->Y) {
        cost *= 1.414f;
    }
    
    float g = parent->G + cost;
    
    // Check if node already in closed list
    Node* existing = _closedNodes->Find(x, y);
    if(existing != NULL) {
        if(g < existing->G) {
            // Found better path, reopen
            existing->G = g;
            existing->F = existing->G + existing->H;
            existing->Parent = parent;
            _closedNodes->Remove(x, y);
            _openNodes->Insert(existing, existing->F);
        }
        return;
    }
    
    // Check if node in open list
    existing = _openNodes->Find(x, y);
    if(existing != NULL) {
        if(g < existing->G) {
            // Better path found, update
            existing->G = g;
            existing->F = existing->G + existing->H;
            existing->Parent = parent;
            // Update heap position
        }
        return;
    }
    
    // Create new node
    Node* node = AllocateNode();
    node->X = x;
    node->Y = y;
    node->G = g;
    node->H = Heuristic(x, y);
    node->F = node->G + node->H;
    node->Parent = parent;
    
    _openNodes->Insert(node, node->F);
}
```

#### 4.4.11 Path Reconstruction

```cpp
Path* AStar::ReconstructPath(Node* goal) {
    Path* path = NULL;
    
    // Trace back from goal to start
    Node* current = goal;
    while(current != NULL) {
        Path* node = AllocatePath();
        node->X = current->X;
        node->Y = current->Y;
        node->Next = path;
        path = node;
        
        current = current->Parent;
    }
    
    return path;
}
```

### 4.5 Path Structure

**Location**: `src/ai/Path.h`

```cpp
struct Path {
    int X, Y;           // Tile coordinates
    Path* Next;         // Singly-linked list
};

Path* AllocatePath();
void FreePath(Path* path, bool recurse);
```

**Memory management**: Paths are allocated from a pool and can be freed recursively.

### 4.6 Target Selection

#### 4.6.1 FindTarget Algorithm

```cpp
Soldier* Soldier::FindTarget(Squad* squad) {
    if(NULL == squad) return NULL;
    
    // Check if squad has any living members
    std::vector<Soldier*>* soldiers = squad->GetSoldiers();
    bool anyAlive = false;
    
    for(int i = soldiers->size() - 1; i >= 0; --i) {
        if(!(*soldiers)[i]->IsDead()) {
            anyAlive = true;
            break;
        }
    }
    
    if(!anyAlive) return NULL;
    
    // Random selection from living members
    Soldier* target;
    for(;;) {
        int idx = rand() % soldiers->size();
        target = (*soldiers)[idx];
        if(!target->IsDead()) {
            return target;
        }
    }
}
```

#### 4.6.2 Target Type Flow

```
FireOrder Created
       |
Target::Squad ──FindTarget()──→ Target::Soldier
       |                              |
       |                              └──Killed?──→ FindTarget() again
       |
Target::Area (ground position)
```

### 4.7 Complete Order-to-Movement Flow

```
1. User clicks destination
   |
2. World::LeftMouseUp() calls IssueOrder(new MoveOrder(x, y))
   |
3. For each selected squad:
   └── Squad::AddOrder(MoveOrder)
       |
4. Squad::HandleMoveOrder():
   ├── Convert world to tile coordinates
   ├── Determine Element::Level from movement style
   ├── AStar::FindPath(start, end, level)
   │   ├── Create start node, add to open list
   │   ├── While open list not empty:
   │   │   ├── GetBestNode() (lowest F)
   │   │   ├── Check if goal reached
   │   │   ├── GenerateSuccessors() for 8 directions
   │   │   ├── CanMove() checks bounds and passability
   │   │   ├── DoMove() calculates G, H, F
   │   │   └── Add to open or update existing
   │   └── ReconstructPath() from goal to start
   ├── Point man: FollowPath(path, WalkTo)
   │   └── Converts Path to WalkTo actions with TileData
   └── Others: Follow(pointMan, Column, spread, idx, WalkTo)
       └── Creates FollowInFormation actions
   |
5. Each frame, Soldier::Simulate():
   ├── Process any Orders
   └── Process Action queue
       └── SoldierActionHandlers::Handle()
           └── WalkToActionHandler
               ├── CheckRequirements (Standing?)
               ├── UpdateState (+Walking, +Moving, -Stopped)
               ├── AtDestination check
               ├── CalculateNewHeading
               └── MoveSoldier (physics update)
   |
6. At each tile:
   └── WalkTo action completes
   └── Next WalkTo action begins
   |
7. At final destination:
   └── DestinationReached action
   └── Stop action
   └── _pathComplete = true
```

---

*[Continue to Section 5: Graphics and Rendering System]*
## 5. Graphics and Rendering System

### 5.1 Screen Class - Software Rendering

**Location**: `src/graphics/Screen.h`, `src/graphics/Screen.cpp`

The `Screen` class provides the core software rendering surface with manual pixel manipulation.

#### 5.1.1 Class Definition

```cpp
class Screen {
public:
    Screen(void);
    virtual ~Screen(void);
    
    // Surface management
    virtual void SetSurface(SDL_Surface* surface);
    virtual void SetRenderer(SDL_Renderer* renderer);
    virtual void SetCapabilities(unsigned char* bits, int width, int height, 
                                  int format, int pitch);
    virtual void Cleanup();
    
    // Clipping
    virtual void SetClippingRectangle(int x, int y, int w, int h);
    
    // Utility
    virtual void Clear(Color* c);
    virtual int GetWidth();
    virtual int GetHeight();
    
    // Origin offset
    Point Origin;
    void SetOrigin(int x, int y);
    
    // Cursor position
    int GetCursorX();
    int GetCursorY();
    void SetCursorPosition(int x, int y);
    
    // BLIT METHODS (10 variants)
    // 1. Basic memcpy
    virtual void Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                      int sw, int sh, int sbytes_per_pixel);
    
    // 2. With transparency
    virtual void Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                      int sw, int sh, int sbytes_per_pixel, 
                      Color* transparentColor);
    
    // 3. With source rectangle
    virtual void Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                      int sx, int sy, int sw, int sh, int sbytes_per_pixel);
    
    // 4. Full effects (shadow, highlight)
    virtual void Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                      int sx, int sy, int sw, int sh,
                      Color* transparentColor, Color* shadowColor,
                      Color* hilitColor, Color* hilitShadowColor,
                      bool bHilit, int sbytes_per_pixel);
    
    // 5. With alpha channel
    virtual void Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                      int sx, int sy, int sw, int sh, int sbytes_per_pixel,
                      bool useAlpha);
    
    // 6. With mask and color modifier (soldier rendering)
    virtual void Blit(unsigned char* src, unsigned char* mask,
                      int dx, int dy, int dw, int dh,
                      int sx, int sy, int sw, int sh,
                      bool bHilit, Color* hilitColor,
                      int sbytes_per_pixel, int colorModifierIdx);
    
    // 7. With rotation
    virtual void Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                      int sw, int sh, int sbytes_per_pixel,
                      Color* transparentColor,
                      int rotx, int roty, double angle);
    
    // 8. With shadow and transparency flags
    virtual void Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                      int sx, int sy, int sw, int sh, int sbytes_per_pixel,
                      bool bUseShadow, bool bUseTransparency);
    
    // SDL surface blitting (for text)
    virtual void BlitSurface(SDL_Surface* src, int dx, int dy);
    
    // Primitives
    virtual void DrawLine(int sx, int sy, int dx, int dy, int width, Color* c);
    virtual void DrawRect(int x, int y, int w, int h, int width, Color* c);
    virtual void FillRect(int x, int y, int w, int h, Color* c);
    
    // Hit testing
    static bool PointInRegion(int x, int y, Region* r);
    static bool PointInRegion(int x, int y, int rx, int ry, int rw, int rh);
    static bool PointInRegion(int x, int y, std::vector<Point>* points);
    
    // Self test
    static bool SelfTest();

protected:
    SDL_Surface* _surface;
    SDL_Renderer* _renderer;
    int _width, _height;
    int _format;
    int _bytes_per_pixel;
    int _pitch;
    int _cursorX, _cursorY;
    int _originX, _originY;
    unsigned char* _bits;
    Rect _clip;
};
```

#### 5.1.2 Pixel Buffer Management

**Format Detection** (from SetCapabilities):
```cpp
switch(format) {
    case SDL_PIXELFORMAT_ARGB8888:
    case SDL_PIXELFORMAT_RGBA8888:
    case SDL_PIXELFORMAT_ABGR8888:
    case SDL_PIXELFORMAT_BGRA8888:
        _bytes_per_pixel = 4;
        break;
    case SDL_PIXELFORMAT_RGB565:
    case SDL_PIXELFORMAT_BGR565:
        _bytes_per_pixel = 2;
        break;
    default:
        _bytes_per_pixel = 4;
        break;
}
```

**Pixel Address Calculation**:
```cpp
// For position (x, y) in screen coordinates
pixel_offset = (y * _pitch) + (x * _bytes_per_pixel);

// Pixel layout (BGR order for 32-bit):
_bits[pixel_offset + 0] = Blue
_bits[pixel_offset + 1] = Green
_bits[pixel_offset + 2] = Red
_bits[pixel_offset + 3] = Alpha (if present)
```

#### 5.1.3 Basic Blit (memcpy)

```cpp
void Screen::Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                  int sw, int sh, int sbytes_per_pixel) {
    // Calculate pitch (bytes per row)
    int src_pitch = sw * sbytes_per_pixel;
    int dst_pitch = _pitch;
    
    // Clip to destination bounds
    int y_start = (dy < 0) ? 0 : dy;
    int y_end = (dy + dh > _height) ? _height : dy + dh;
    int x_start = (dx < 0) ? 0 : dx;
    int x_end = (dx + dw > _width) ? _width : dx + dw;
    
    // Copy each scanline
    for(int y = y_start; y < y_end; y++) {
        int src_y = y - dy;
        int dst_offset = y * dst_pitch + x_start * _bytes_per_pixel;
        int src_offset = src_y * src_pitch + (x_start - dx) * sbytes_per_pixel;
        
        memcpy(_bits + dst_offset, src + src_offset, 
               (x_end - x_start) * sbytes_per_pixel);
    }
}
```

#### 5.1.4 Blit with Transparency

```cpp
void Screen::Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                  int sw, int sh, int sbytes_per_pixel,
                  Color* transparentColor) {
    // Apply clipping rectangle
    int clip_x0 = (_clip.x > dx) ? _clip.x : dx;
    int clip_y0 = (_clip.y > dy) ? _clip.y : dy;
    int clip_x1 = (_clip.x + _clip.w < dx + dw) ? _clip.x + _clip.w : dx + dw;
    int clip_y1 = (_clip.y + _clip.h < dy + dh) ? _clip.y + _clip.h : dy + dh;
    
    // Pixel-by-pixel with transparency check
    for(int y = clip_y0; y < clip_y1; y++) {
        for(int x = clip_x0; x < clip_x1; x++) {
            int src_x = x - dx;
            int src_y = y - dy;
            int src_idx = (src_y * sw + src_x) * sbytes_per_pixel;
            int dst_idx = y * _pitch + x * _bytes_per_pixel;
            
            // Check if transparent (BGR order in source)
            unsigned char b = src[src_idx + 0];
            unsigned char g = src[src_idx + 1];
            unsigned char r = src[src_idx + 2];
            
            if(r != transparentColor->red || 
               g != transparentColor->green || 
               b != transparentColor->blue) {
                _bits[dst_idx + 0] = b;
                _bits[dst_idx + 1] = g;
                _bits[dst_idx + 2] = r;
                if(sbytes_per_pixel == 4) {
                    _bits[dst_idx + 3] = src[src_idx + 3];
                }
            }
        }
    }
}
```

#### 5.1.5 Full Effects Blit (Shadow, Highlight)

```cpp
void Screen::Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                  int sx, int sy, int sw, int sh,
                  Color* transparentColor, Color* shadowColor,
                  Color* hilitColor, Color* hilitShadowColor,
                  bool bHilit, int sbytes_per_pixel) {
    
    for(int y = clip_y0; y < clip_y1; y++) {
        for(int x = clip_x0; x < clip_x1; x++) {
            int src_x = sx + (x - dx);
            int src_y = sy + (y - dy);
            int src_idx = (src_y * sw + src_x) * sbytes_per_pixel;
            int dst_idx = y * _pitch + x * _bytes_per_pixel;
            
            unsigned char b = src[src_idx + 0];
            unsigned char g = src[src_idx + 1];
            unsigned char r = src[src_idx + 2];
            
            // Check transparent
            if(r == transparentColor->red && 
               g == transparentColor->green && 
               b == transparentColor->blue) {
                continue;
            }
            
            // Check shadow color (darken destination by 25%)
            if(r == shadowColor->red && 
               g == shadowColor->green && 
               b == shadowColor->blue) {
                _bits[dst_idx + 0] = (_bits[dst_idx + 0] >> 2) * 3;
                _bits[dst_idx + 1] = (_bits[dst_idx + 1] >> 2) * 3;
                _bits[dst_idx + 2] = (_bits[dst_idx + 2] >> 2) * 3;
                continue;
            }
            
            // Check highlight
            if(r == hilitShadowColor->red && 
               g == hilitShadowColor->green && 
               b == hilitShadowColor->blue) {
                if(bHilit) {
                    // Use highlight color
                    _bits[dst_idx + 0] = hilitColor->blue;
                    _bits[dst_idx + 1] = hilitColor->green;
                    _bits[dst_idx + 2] = hilitColor->red;
                } else {
                    // Remove highlight (make transparent, darken shadow)
                    _bits[dst_idx + 0] = (_bits[dst_idx + 0] >> 2) * 3;
                    _bits[dst_idx + 1] = (_bits[dst_idx + 1] >> 2) * 3;
                    _bits[dst_idx + 2] = (_bits[dst_idx + 2] >> 2) * 3;
                }
                continue;
            }
            
            // Regular pixel
            _bits[dst_idx + 0] = b;
            _bits[dst_idx + 1] = g;
            _bits[dst_idx + 2] = r;
        }
    }
}
```

#### 5.1.6 Alpha Blending

```cpp
void Screen::Blit(unsigned char* src, int dx, int dy, int dw, int dh,
                  int sx, int sy, int sw, int sh, int sbytes_per_pixel,
                  bool useAlpha) {
    
    for(int y = clip_y0; y < clip_y1; y++) {
        for(int x = clip_x0; x < clip_x1; x++) {
            int src_x = sx + (x - dx);
            int src_y = sy + (y - dy);
            int src_idx = (src_y * sw + src_x) * sbytes_per_pixel;
            int dst_idx = y * _pitch + x * _bytes_per_pixel;
            
            unsigned char alpha = src[src_idx + 3];
            
            if(alpha == 0) continue;  // Fully transparent
            if(alpha == 255) {
                // Fully opaque - direct copy
                _bits[dst_idx + 0] = src[src_idx + 0];
                _bits[dst_idx + 1] = src[src_idx + 1];
                _bits[dst_idx + 2] = src[src_idx + 2];
            } else {
                // Alpha blend: new = alpha * src + (1 - alpha) * dst
                // Implemented as: (alpha * (src - dst) >> 8) + dst
                _bits[dst_idx + 0] = ((alpha * (src[src_idx + 0] - _bits[dst_idx + 0])) >> 8) + _bits[dst_idx + 0];
                _bits[dst_idx + 1] = ((alpha * (src[src_idx + 1] - _bits[dst_idx + 1])) >> 8) + _bits[dst_idx + 1];
                _bits[dst_idx + 2] = ((alpha * (src[src_idx + 2] - _bits[dst_idx + 2])) >> 8) + _bits[dst_idx + 2];
            }
        }
    }
}
```

#### 5.1.7 Mask-Based Blit (Soldier Rendering)

**Mask values** (from SoldierMasks.h):
```cpp
#define MASK_TRANSPARENT    0xFFFFFF  // Skip pixel
#define MASK_SHADOW         0x007F00  // Darken 25%
#define MASK_EDGE           0x00C800  // Edge highlight
#define MASK_SHADOW_EDGE    0x1FFF1F  // Shadow edge
#define MASK_BODY           0x640000  // Apply body color modifier
#define MASK_LEGS           0x780000  // Apply legs color modifier
#define MASK_HEAD           0x8C0000  // Apply head color modifier
#define MASK_BELT           0xA00000  // Apply belt color modifier
#define MASK_BOOTS          0xB40000  // Apply boots color modifier
#define MASK_WEAPON         0xC80000  // Apply weapon color modifier
```

```cpp
void Screen::Blit(unsigned char* src, unsigned char* mask,
                  int dx, int dy, int dw, int dh,
                  int sx, int sy, int sw, int sh,
                  bool bHilit, Color* hilitColor,
                  int sbytes_per_pixel, int colorModifierIdx) {
    
    ColorModifiers* mod = &g_ColorModifiers[colorModifierIdx];
    
    for(int y = clip_y0; y < clip_y1; y++) {
        for(int x = clip_x0; x < clip_x1; x++) {
            int src_x = sx + (x - dx);
            int src_y = sy + (y - dy);
            int src_idx = (src_y * sw + src_x) * sbytes_per_pixel;
            int mask_idx = (src_y * sw + src_x) * 4;  // Mask is always 32-bit
            int dst_idx = y * _pitch + x * _bytes_per_pixel;
            
            // Read mask value
            unsigned int maskValue = 
                (mask[mask_idx + 0]) |
                (mask[mask_idx + 1] << 8) |
                (mask[mask_idx + 2] << 16);
            
            switch(maskValue) {
            case MASK_TRANSPARENT:
                continue;  // Skip
                
            case MASK_SHADOW:
                // Darken destination 25%
                _bits[dst_idx + 0] = (_bits[dst_idx + 0] >> 2) * 3;
                _bits[dst_idx + 1] = (_bits[dst_idx + 1] >> 2) * 3;
                _bits[dst_idx + 2] = (_bits[dst_idx + 2] >> 2) * 3;
                break;
                
            case MASK_BODY:
                // Apply color modifier
                _bits[dst_idx + 0] = Clamp(src[src_idx + 0] + mod->Body.blue, 0, 255);
                _bits[dst_idx + 1] = Clamp(src[src_idx + 1] + mod->Body.green, 0, 255);
                _bits[dst_idx + 2] = Clamp(src[src_idx + 2] + mod->Body.red, 0, 255);
                break;
                
            case MASK_LEGS:
                _bits[dst_idx + 0] = Clamp(src[src_idx + 0] + mod->Legs.blue, 0, 255);
                _bits[dst_idx + 1] = Clamp(src[src_idx + 1] + mod->Legs.green, 0, 255);
                _bits[dst_idx + 2] = Clamp(src[src_idx + 2] + mod->Legs.red, 0, 255);
                break;
                
            // ... similar for HEAD, BELT, BOOTS, WEAPON
            
            default:
                // Regular pixel
                _bits[dst_idx + 0] = src[src_idx + 0];
                _bits[dst_idx + 1] = src[src_idx + 1];
                _bits[dst_idx + 2] = src[src_idx + 2];
                break;
            }
        }
    }
}
```

#### 5.1.8 Line Drawing

**Bresenham's Algorithm**:
```cpp
void Screen::DrawLine(int x0, int y0, int x1, int y1, int width, Color* c) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while(true) {
        // Draw point (with width support)
        for(int wy = 0; wy < width; wy++) {
            for(int wx = 0; wx < width; wx++) {
                int px = x0 + wx;
                int py = y0 + wy;
                if(px >= _clip.x && px < _clip.x + _clip.w &&
                   py >= _clip.y && py < _clip.y + _clip.h) {
                    int idx = py * _pitch + px * _bytes_per_pixel;
                    _bits[idx + 0] = c->blue;
                    _bits[idx + 1] = c->green;
                    _bits[idx + 2] = c->red;
                }
            }
        }
        
        if(x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if(e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if(e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
```

#### 5.1.9 Point-in-Region Testing

**Ray Casting Algorithm**:
```cpp
bool Screen::PointInRegion(int x, int y, Region* r) {
    int crossings = 0;
    
    for(int i = 0; i < 4; i++) {
        int j = (i + 1) % 4;
        
        Point* p1 = &r->points[i];
        Point* p2 = &r->points[j];
        
        // Check if edge straddles horizontal ray from point
        if(((p1->y <= y) && (p2->y > y)) || ((p1->y > y) && (p2->y <= y))) {
            // Calculate x intersection
            float xIntersection = p1->x + (y - p1->y) * 
                                 (float)(p2->x - p1->x) / (float)(p2->y - p1->y);
            
            if(x < xIntersection) {
                crossings++;
            }
        }
    }
    
    return (crossings % 2) == 1;  // Odd = inside
}
```

### 5.2 TGA Image Loading

**Location**: `src/misc/TGA.h`, `src/misc/TGA.cpp`

#### 5.2.1 TGA Header Structure

```cpp
#pragma pack(push, 1)
typedef struct {
    char  idlength;              // ID field length
    char  colourmaptype;         // Color map type (0 = none)
    char  datatypecode;          // Image type (2 = uncompressed RGB)
    short int colourmaporigin;
    short int colourmaplength;
    char  colourmapdepth;
    short int x_origin;          // X origin
    short int y_origin;          // Y origin
    short width;                 // Image width
    short height;                // Image height
    char  bitsperpixel;          // 16, 24, or 32
    char  imagedescriptor;       // Image descriptor
} TGA_HEADER;
#pragma pack(pop)
```

#### 5.2.2 TGA Class

```cpp
class TGA {
public:
    TGA(void);
    ~TGA(void);
    
    // Factory method
    static TGA* Create(const std::filesystem::path& fileName);
    
    // Accessors
    unsigned char* GetData();
    int GetDepth();          // Always 4 (32-bit)
    int GetWidth();
    int GetHeight();
    
    // Hotspot/origin for positioning
    void SetOrigin(int x, int y);
    int GetOriginX();
    int GetOriginY();
    
    // Transparent color
    void SetTransparentColor(unsigned char r, unsigned char g, unsigned char b);
    Color* GetTransparentColor();

private:
    unsigned char* _data;    // Pixel buffer (BGRA)
    long _idx;               // Unique index
    int _width, _height;
    int _depth;
    int _originX, _originY;
    Color _transparentColor;
};
```

#### 5.2.3 Loading Process

```cpp
TGA* TGA::Create(const std::filesystem::path& fileName) {
    FILE* file = fopen(fileName.c_str(), "rb");
    if(!file) return NULL;
    
    // Read header
    TGA_HEADER header;
    fread(&header, sizeof(TGA_HEADER), 1, file);
    
    // Skip ID field
    if(header.idlength > 0) {
        fseek(file, header.idlength, SEEK_CUR);
    }
    
    // Skip color map
    if(header.colourmaptype != 0) {
        fseek(file, header.colourmaplength * (header.colourmapdepth / 8), SEEK_CUR);
    }
    
    // Create TGA object
    TGA* tga = new TGA();
    tga->_width = header.width;
    tga->_height = header.height;
    tga->_depth = 4;  // Always 32-bit
    
    // Allocate buffer
    int size = tga->_width * tga->_height * 4;
    tga->_data = new unsigned char[size];
    
    // Read pixel data
    switch(header.bitsperpixel) {
    case 32:
        fread(tga->_data, size, 1, file);
        break;
        
    case 24:
        // Convert 24-bit to 32-bit
        for(int y = 0; y < tga->_height; y++) {
            for(int x = 0; x < tga->_width; x++) {
                int srcIdx = ((tga->_height - 1 - y) * tga->_width + x) * 3;
                int dstIdx = (y * tga->_width + x) * 4;
                
                unsigned char pixel[3];
                fread(pixel, 3, 1, file);
                
                tga->_data[dstIdx + 0] = pixel[0];  // B
                tga->_data[dstIdx + 1] = pixel[1];  // G
                tga->_data[dstIdx + 2] = pixel[2];  // R
                tga->_data[dstIdx + 3] = 255;       // A
            }
        }
        break;
        
    case 16:
        // Convert 16-bit (5-5-5 RGB) to 32-bit
        for(int y = 0; y < tga->_height; y++) {
            for(int x = 0; x < tga->_width; x++) {
                unsigned short pixel;
                fread(&pixel, 2, 1, file);
                
                int dstIdx = ((tga->_height - 1 - y) * tga->_width + x) * 4;
                
                // Extract 5-5-5 RGB
                unsigned char b = (pixel & 0x1F) << 3;
                unsigned char g = ((pixel >> 5) & 0x1F) << 3;
                unsigned char r = ((pixel >> 10) & 0x1F) << 3;
                
                tga->_data[dstIdx + 0] = b;
                tga->_data[dstIdx + 1] = g;
                tga->_data[dstIdx + 2] = r;
                tga->_data[dstIdx + 3] = 255;
            }
        }
        break;
    }
    
    fclose(file);
    
    // Parse origin from filename
    ParseOriginFromFilename(tga, fileName);
    
    return tga;
}
```

#### 5.2.4 Filename Origin Parsing

**CRITICAL**: Strip `.tga` extension BEFORE parsing coordinates.

```cpp
void ParseOriginFromFilename(TGA* tga, const std::filesystem::path& fileName) {
    std::string fName = fileName.filename().string();
    
    // Step 1: Strip .tga extension
    size_t extDot = fName.rfind('.');
    if(extDot != std::string::npos) {
        fName = fName.substr(0, extDot);
    }
    
    // Step 2: Find Y coordinate (after last dot)
    size_t yDot = fName.rfind('.');
    if(yDot == std::string::npos) return;
    
    std::string yStr = fName.substr(yDot + 1);
    fName = fName.substr(0, yDot);
    
    // Step 3: Find X coordinate (after next dot)
    size_t xDot = fName.rfind('.');
    if(xDot == std::string::npos) return;
    
    std::string xStr = fName.substr(xDot + 1);
    
    // Parse integers
    int x = atoi(xStr.c_str());
    int y = atoi(yStr.c_str());
    
    tga->SetOrigin(x, y);
}
```

**Example parsing**:
```
Filename: image001.-15.-3.tga
Strip .tga: image001.-15.-3
Find last dot: image001.-15 and -3 (y = -3)
Find next dot: image001 and -15 (x = -15)
Result: origin = (-15, -3)
```

### 5.3 Animation System

**Location**: `src/graphics/Animation.h`, `src/graphics/Animation.cpp`

#### 5.3.1 Animation Class

```cpp
class Animation {
public:
    Animation(const std::string &name);
    ~Animation(void);
    
    void AddFrame(Frame* f, Direction dir);
    Animation* Clone();
    
    void Render(Screen* screen, Direction heading, int x, int y,
                bool hilit, Color* hilitColor, int camouflageIdx);
    void Update(long dt);
    
    void GetExtents(Direction heading, int x, int y, Region* r);
    int GetCurrentFrameNumber(Direction heading);
    void Reset();
    void SetReverse(bool b);
    
    const std::string& GetName() const;

protected:
    // 8-directional frame arrays
    std::array<std::vector<std::unique_ptr<Frame>>, static_cast<size_t>(Direction::NumDirections)> _frames;
    
    std::string _name;
    std::array<int, static_cast<size_t>(Direction::NumDirections)> _currentFrameNums;
    std::array<long, static_cast<size_t>(Direction::NumDirections)> _totalTimes;
    std::array<long, static_cast<size_t>(Direction::NumDirections)> _incrementalTimes;
    bool _reverse;
};
```

#### 5.3.2 Direction Enum

```cpp
enum class Direction {
    South = 0,      // 0 degrees (facing down)
    SouthWest,      // 45 degrees
    West,           // 90 degrees
    NorthWest,      // 135 degrees
    North,          // 180 degrees (facing up)
    NorthEast,      // 225 degrees
    East,           // 270 degrees
    SouthEast,      // 315 degrees
    NumDirections   // 8
};
```

#### 5.3.3 Frame Storage

```cpp
void Animation::AddFrame(Frame* f, Direction dir) {
    _frames[dir].push_back(std::unique_ptr<Frame>(f));
}
```

Each direction has its own vector of frames. For 8-directional animations, frames are duplicated 8 times with different directional sprites.

#### 5.3.4 Frame Update

```cpp
void Animation::Update(long dt) {
    for(int dir = 0; dir < NumDirections; dir++) {
        if(_frames[dir].empty()) continue;
        
        _totalTimes[dir] += dt;
        _incrementalTimes[dir] += dt;
        
        Frame* currentFrame = _frames[dir][_currentFrameNums[dir]].get();
        
        if(_totalTimes[dir] > currentFrame->GetDisplayTime()) {
            _totalTimes[dir] = 0;
            
            if(_reverse) {
                _currentFrameNums[dir]--;
                if(_currentFrameNums[dir] < 0) {
                    _currentFrameNums[dir] = _frames[dir].size() - 1;
                }
            } else {
                _currentFrameNums[dir] = 
                    (_currentFrameNums[dir] + 1) % _frames[dir].size();
            }
        }
    }
}
```

#### 5.3.5 Frame Rendering

```cpp
void Animation::Render(Screen* screen, Direction heading, int x, int y,
                       bool hilit, Color* hilitColor, int camouflageIdx) {
    if(_frames[heading].empty()) return;
    
    Frame* frame = _frames[heading][_currentFrameNums[heading]].get();
    frame->Render(screen, x, y, hilit, hilitColor, camouflageIdx);
}
```

### 5.4 Frame Class

**Location**: `src/graphics/Frame.h`, `src/graphics/Frame.cpp`

#### 5.4.1 Frame Definition

```cpp
class Frame {
public:
    Frame(TGA* source, int displayTime, int width, int height,
          int sourceX, int sourceY, Color* transparentColor);
    ~Frame(void);
    
    Frame* Clone();
    void Render(Screen* screen, int x, int y, bool hilit, 
                Color* hilitColor, int camouflageIdx);
    void GetExtents(int x, int y, Region* r);
    int GetDisplayTime();

protected:
    int _displayTime;        // Milliseconds to display
    TGA* _tga;              // Source image (not owned)
    int _width, _height;    // Frame dimensions
    int _sourceX, _sourceY; // Position in source TGA
    Bounds _minBounds;      // Tight bounds (excludes transparent)
    
    // Color effects
    Color _transparentColor;
    Color _shadowColor;
    Color _hilitColor;
    Color _hilitShadowColor;
};
```

#### 5.4.2 Min Bounds Calculation

Calculates tight bounding box by scanning for non-transparent pixels:

```cpp
// In constructor
_minBounds.x0 = width;
_minBounds.y0 = height;
_minBounds.x1 = 0;
_minBounds.y1 = 0;

for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
        int idx = ((sourceY + y) * _tga->GetWidth() + (sourceX + x)) * 4;
        
        unsigned char b = _tga->GetData()[idx + 0];
        unsigned char g = _tga->GetData()[idx + 1];
        unsigned char r = _tga->GetData()[idx + 2];
        
        // Check if not transparent and not shadow
        if((r != transparentColor->red || 
            g != transparentColor->green || 
            b != transparentColor->blue) &&
           (r != _shadowColor.red || 
            g != _shadowColor.green || 
            b != _shadowColor.blue)) {
            
            if(x < _minBounds.x0) _minBounds.x0 = x;
            if(x > _minBounds.x1) _minBounds.x1 = x;
            if(y < _minBounds.y0) _minBounds.y0 = y;
            if(y > _minBounds.y1) _minBounds.y1 = y;
        }
    }
}
```

#### 5.4.3 Frame Rendering

```cpp
void Frame::Render(Screen* screen, int x, int y, bool hilit,
                   Color* hilitColor, int camouflageIdx) {
    // Calculate centered position
    int dx = x - ((_sourceX + (_width >> 1)) - _minBounds.x0);
    int dy = y - ((_sourceY + (_height >> 1)) - _minBounds.y0);
    int dw = _minBounds.x1 - _minBounds.x0;
    int dh = _minBounds.y1 - _minBounds.y0;
    
    // Blit with full effects
    screen->Blit(_tga->GetData(),
                 dx, dy, dw, dh,
                 _minBounds.x0, _minBounds.y0,
                 _tga->GetWidth(), _tga->GetHeight(),
                 &_transparentColor, &_shadowColor,
                 hilitColor, &_hilitShadowColor,
                 hilit, _tga->GetDepth());
}
```

### 5.5 Effect System

**Location**: `src/graphics/Effect.h`, `src/graphics/Effect.cpp`

#### 5.5.1 Effect Class

```cpp
class Effect {
public:
    Effect(const std::string &name);
    ~Effect(void);
    
    Effect* Clone();
    void AddFrame(TGA* tga, long frameHoldTime);
    
    void Render(Screen* screen);
    void Simulate(long dt);
    
    bool IsCompleted();
    bool IsDynamic();
    bool IsPlaceOnTurret();
    
    void SetPosition(int x, int y);
    void SetSound(const std::string &name);
    void SetDynamic(bool d);
    void SetPlaceOnTurret(bool p);
    
    const std::string& GetName() const;
    
    Point Position;

protected:
    std::string _name;
    std::string _sound;
    long _frameHoldTime;
    long _totalTime;
    long _incrementalTime;
    bool _completed;
    std::vector<TGA*> _frames;
    int _currentFrameNumber;
    bool _dynamic;
    bool _bPlaceOnTurret;
};
```

#### 5.5.2 Effect Types

- **Static**: Fixed world position (explosions)
- **Dynamic**: Offset from parent entity (muzzle flashes)
- **Turret-placed**: Follows turret rotation (tank muzzle flashes)

#### 5.5.3 Effect Simulation

```cpp
void Effect::Simulate(long dt) {
    // Play sound on first update
    if(_totalTime == 0 && !_sound.empty()) {
        g_Globals->World.SoundEffects->GetSound(_sound)->Play();
    }
    
    _totalTime += dt;
    _incrementalTime += dt;
    
    if(_incrementalTime > _frameHoldTime) {
        _incrementalTime = 0;
        _currentFrameNumber++;
        
        if(_currentFrameNumber >= _frames.size()) {
            _completed = true;
        }
    }
}
```

#### 5.5.4 Effect Rendering

```cpp
void Effect::Render(Screen* screen) {
    if(_completed || _frames.empty()) return;
    
    TGA* tga = _frames[_currentFrameNumber];
    
    // Apply origin offset
    int x = Position.x - tga->GetOriginX() - screen->Origin.x;
    int y = Position.y - tga->GetOriginY() - screen->Origin.y;
    
    screen->Blit(tga->GetData(),
                 x, y,
                 tga->GetWidth(), tga->GetHeight(),
                 0, 0,
                 tga->GetWidth(), tga->GetHeight(),
                 tga->GetDepth(), true);  // Use alpha
}
```

### 5.6 Rendering Pipeline

#### 5.6.1 Frame Rendering Order

```
1. Clear screen
   |
2. World::Render(screen, clip)
   ├── Render map background
   ├── Render buildings (exterior)
   ├── For each visible tile (back-to-front):
   │   ├── Render terrain objects (trees)
   │   ├── Render objects on tile
   │   │   └── Object::Render()
   │   │       ├── Animation::Render()
   │   │       │   └── Frame::Render()
   │   │       │       └── Screen::Blit()
   │   │       └── Render effects
   │   └── Render building interiors (if occupied)
   ├── Render effects (explosions)
   └── Render marks/waypoints
   |
3. Render UI
   ├── Minimap
   ├── Team panel
   ├── Unit panel
   └── Bottom command bar
   |
4. Present to screen
```

#### 5.6.2 Object Rendering

```cpp
void Soldier::Render(Screen* screen, Rect* clip) {
    // Update animation
    _animations[_currentAnimationState]->Update(SIMULATION_TIMESTEP_MS);
    
    // Determine highlight color
    Color* hilitColor = NULL;
    if(_isSelected) {
        hilitColor = GetSelectionColor();
    } else if(_bHighlight) {
        hilitColor = &_highlightColor;
    }
    
    // Render animation
    _animations[_currentAnimationState]->Render(
        screen, _currentHeading,
        Position.x - screen->Origin.x,
        Position.y - screen->Origin.y,
        _isSelected || _bHighlight,
        hilitColor,
        _camoIdx);
    
    // Render effects
    for(auto& effect : _effects) {
        effect->Render(screen);
    }
}
```

### 5.7 Font Rendering

**Location**: `src/graphics/FontManager.h`, `src/graphics/FontManager.cpp`

Uses SDL_ttf for TrueType font rendering:

```cpp
class FontManager {
public:
    enum FontSize {
        FontSize_Small = 0,   // 9pt
        FontSize_Large = 1,   // 14pt
        FontSize_Count = 2
    };
    
    void Initialize(void* data);
    void Cleanup();
    
    void Render(Screen* screen, const std::string &msg, 
                int x, int y, Color* c, FontSize size = FontSize_Small);
    void GetTextSize(const std::string &msg, int *w, int *h,
                     FontSize size = FontSize_Small);

private:
    TTF_Font* _fonts[FontSize_Count];
};
```

Rendering process:
```cpp
void FontManager::Render(Screen* screen, const std::string &msg,
                         int x, int y, Color* c, FontSize size) {
    // Create SDL color
    SDL_Color sdlColor = {c->red, c->green, c->blue, 255};
    
    // Render to surface
    SDL_Surface* textSurface = TTF_RenderText_Blended(
        _fonts[size], msg.c_str(), sdlColor);
    
    if(textSurface) {
        // Blit to screen
        screen->BlitSurface(textSurface, x, y);
        SDL_FreeSurface(textSurface);
    }
}
```

---

*[Continue to Section 6: World, Map, and Terrain System]*
## 6. World, Map, and Terrain System

### 6.1 World Class

**Location**: `src/world/World.h`, `src/world/World.cpp`

The `World` class is the central game world manager, handling objects, simulation, rendering, and user interaction.

#### 6.1.1 World States

```cpp
enum WorldState {
    Normal,             // Default gameplay
    ContextSelecting,   // Right-click menu open
    ContextSelected,    // Menu item selected, performing action
    Ambushing,          // Choosing ambush direction
    Defending           // Choosing defend direction
};
```

#### 6.1.2 Core Members

```cpp
class World {
public:
    World(void);
    ~World(void);
    
    // Lifecycle
    void Load(const std::filesystem::path& fileName,
              SoldierManager* soldierManager,
              AnimationManager* animationManager);
    void Simulate(long dt);
    void Render(Screen* screen, Rect* clip);
    
    // Object management
    void AddObject(Object* o);
    void MoveObject(Object* object, Point* from, Point* to);
    bool TryMove(Object* o, int x, int y);
    
    // Input
    void LeftMouseDown(int x, int y);
    void LeftMouseUp(int x, int y);
    void LeftMouseDrag(int x, int y);
    void RightMouseDown(int x, int y);
    void RightMouseUp(int x, int y);
    void RightMouseDrag(int x, int y);
    void MiddleMouseDown(int x, int y);
    void MiddleMouseUp(int x, int y);
    void MiddleMouseDrag(int x, int y);
    void KeyUp(int key);
    void KeyDown(int key);
    
    // Orders
    void IssueOrder(Order* o);
    
    // Camera/Origin
    void SetOrigin(int x, int y);
    void GetOrigin(int *x, int *y);
    inline int GetWidth() { return _currentMap->GetWidth(); }
    inline int GetHeight() { return _currentMap->GetHeight(); }
    
    // Object access
    inline std::vector<Object*>* GetObjects() { return &_mobileObjects; }
    
    // Coordinate conversion
    void ConvertTileToPosition(int i, int j, int *x, int *y);
    void ConvertPositionToTile(int x, int y, int *i, int *j);
    
    // Marks
    void AddMark(Mark::Color markColor, int x, int y);
    void ClearMarks();
    
    // Victory locations
    int GetNumVictoryLocations();
    void GetVictoryLocation(int index, int *x, int *y, Nationality** nationality);
    const std::string& GetVictoryLocationName(int index);
    
    // Terrain queries
    bool IsPassable(int i, int j) { 
        return _currentMap->GetTileElement(i,j)->Passable; 
    }
    Element* GetTileElement(int i, int j) {
        return _currentMap->GetTileElement(i,j);
    }
    
    // Interface state for UI
    InterfaceState State;
    
    // Public properties
    Point NumTiles;
    Size TileSize;

protected:
    // Core map
    Map* _currentMap;
    
    // Object collections
    std::vector<Object*> _mobileObjects;
    std::vector<Object*> _staticObjects;
    std::vector<Object*> _selectedObjects;
    std::vector<Effect*> _effects;
    
    // State
    WorldState _currentState;
    CombatContextMenu::ContextMenuChoice _currentChoice;
    
    // Camera
    int _originX, _originY;
    int _screenWidth, _screenHeight;
    
    // Selection
    int _rangerX, _rangerY;
    Object* _rangerSelectedObject;
    Color _rangerColor;
    LineOfSight* _lineOfSight;
    
    // Marks
    Point* _markPoints;
    Mark::Color* _markColors;
    int _numMarks;
    int _maxMarks;
    
    // Managers
    SoldierManager* _soldierManager;
    AnimationManager* _animationManager;
    SquadManager* _squadManager;
    EffectManager* _effectManager;
    WeaponManager* _weaponManager;
    ElementManager* _elementManager;
    VehicleManager* _vehicleManager;
    ColorManager* _colorManager;
    CombatContextMenu* _contextMenu;
    MiniMap* _currentMiniMap;
    
    // Scroll state
    bool _scrollLeft, _scrollRight, _scrollUp, _scrollDown;
    bool _scrollRepeating;
    long _scrollTimer;
    
    // Middle mouse drag
    bool _middleDragActive;
    int _middleDragLastX, _middleDragLastY;
    
    // Arc selection
    Direction _currentHeadingArc;
};
```

#### 6.1.3 Object Ownership

```
World (Owner)
├── _mobileObjects (owns all mobile entities)
│   ├── Soldiers (created by SoldierManager, owned here)
│   ├── Vehicles (created by VehicleManager, owned here)
│   └── Squads (created by SquadManager, owned here)
├── _staticObjects (owned)
├── _selectedObjects (references, non-owning)
└── _effects (references to EffectManager prototypes)
```

**CRITICAL**: Squads are owned by World (via _mobileObjects), NOT by SquadManager.

#### 6.1.4 Selection System

**ClearSelect()**:
```cpp
void World::ClearSelect(int x, int y) {
    for(auto* obj : _selectedObjects) {
        obj->Select(false);
        
        // If object is in a squad, deselect entire squad
        Squad* squad = dynamic_cast<Squad*>(obj);
        if(squad) {
            for(auto* soldier : *squad->GetSoldiers()) {
                soldier->Select(false);
            }
        }
    }
    _selectedObjects.clear();
}
```

**Select()**:
```cpp
void World::Select(int x, int y) {
    ClearSelect(x, y);
    
    // Convert to world coordinates
    int worldX = x + _originX;
    int worldY = y + _originY;
    
    // Search map (3x3 tile area)
    _currentMap->SelectObjects(worldX, worldY, &_selectedObjects);
    
    // If nothing found, check mobile objects
    if(_selectedObjects.empty()) {
        for(auto* obj : _mobileObjects) {
            if(obj->Select(worldX, worldY)) {
                _selectedObjects.push_back(obj);
                break;
            }
        }
    }
    
    // Highlight selected
    for(auto* obj : _selectedObjects) {
        obj->Select(true);
        
        // If squad member selected, select squad
        Squad* squad = obj->GetSquad();
        if(squad) {
            squad->Select(true);
            if(std::find(_selectedObjects.begin(), _selectedObjects.end(), squad)
               == _selectedObjects.end()) {
                _selectedObjects.push_back(squad);
            }
        }
    }
    
    UpdateState();
}
```

**The Infantry Firing Bug Fix**:
```cpp
// OLD (buggy):
_currentMap->SelectObjects(worldX, worldY, &_selectedObjects);
for(auto* obj : _mobileObjects) {  // Always iterated!
    if(obj->Select(worldX, worldY)) {
        _selectedObjects.push_back(obj);
    }
}

// NEW (fixed):
_currentMap->SelectObjects(worldX, worldY, &_selectedObjects);
if(_selectedObjects.empty()) {  // Only check mobile if map found nothing
    for(auto* obj : _mobileObjects) {
        if(obj->Select(worldX, worldY)) {
            _selectedObjects.push_back(obj);
            break;  // Only select one
        }
    }
}
```

#### 6.1.5 Order Issuance

```cpp
void World::IssueOrder(Order* o) {
    for(auto* obj : _selectedObjects) {
        obj->ClearOrders();
        obj->AddOrder(o);  // Increments ref count
    }
    
    // Release our reference
    o->Release();
}
```

**Note**: Order is released after adding to all objects because AddOrder() increments ref count.

#### 6.1.6 Origin/Camera Management

```cpp
void World::SetOrigin(int x, int y) {
    _originX = x;
    _originY = y;
    
    // Clamp to map bounds
    if(_originX < 0) _originX = 0;
    else if((_originX + _screenWidth) > _currentMap->GetWidth())
        _originX = _currentMap->GetWidth() - _screenWidth;
    
    if(_originY < 0) _originY = 0;
    else if((_originY + _screenHeight) > _currentMap->GetHeight())
        _originY = _currentMap->GetHeight() - _screenHeight;
    
    // Update minimap
    if(_currentMiniMap) {
        _currentMiniMap->Update();
    }
}
```

#### 6.1.7 Middle Mouse Drag Scrolling

```cpp
void World::MiddleMouseDown(int x, int y) {
    _middleDragActive = true;
    _middleDragLastX = x;
    _middleDragLastY = y;
}

void World::MiddleMouseUp(int x, int y) {
    _middleDragActive = false;
}

void World::MiddleMouseDrag(int x, int y) {
    if(_middleDragActive) {
        int deltaX = _middleDragLastX - x;  // Inverted for natural feel
        int deltaY = _middleDragLastY - y;
        
        SetOrigin(_originX + deltaX, _originY + deltaY);
        
        _middleDragLastX = x;
        _middleDragLastY = y;
    }
}
```

#### 6.1.8 Key-Based Scrolling

```cpp
void World::KeyDown(int key) {
    switch(key) {
    case 0x25: _scrollLeft = true; break;   // Left arrow
    case 0x26: _scrollUp = true; break;     // Up arrow
    case 0x27: _scrollRight = true; break;  // Right arrow
    case 0x28: _scrollDown = true; break;   // Down arrow
    }
}

void World::KeyUp(int key) {
    switch(key) {
    case 0x25: _scrollLeft = false; break;
    case 0x26: _scrollUp = false; break;
    case 0x27: _scrollRight = false; break;
    case 0x28: _scrollDown = false; break;
    }
}

// In Simulate()
if(_scrollLeft || _scrollRight || _scrollUp || _scrollDown) {
    if(!_scrollRepeating) {
        _scrollTimer += dt;
        if(_scrollTimer >= SCROLL_INITIAL_DELAY_MS) {
            _scrollRepeating = true;
            _scrollTimer = 0;
        }
    }
    
    if(_scrollRepeating || _scrollTimer == 0) {
        int speed = SCROLL_SPEED_PPS * dt / 1000;
        if(_scrollLeft) SetOrigin(_originX - speed, _originY);
        if(_scrollRight) SetOrigin(_originX + speed, _originY);
        if(_scrollUp) SetOrigin(_originX, _originY - speed);
        if(_scrollDown) SetOrigin(_originX, _originY + speed);
    }
}
```

### 6.2 Map Class

**Location**: `src/world/Map.h`, `src/world/Map.cpp`

#### 6.2.1 Map Structure

```cpp
class Map {
public:
    Map(void);
    ~Map(void);
    
    void Create(const std::filesystem::path& fileName);
    
    // Dimensions
    inline int GetWidth() { return _nBlocksX * _nPixelsPerBlockX; }
    inline int GetHeight() { return _nBlocksY * _nPixelsPerBlockY; }
    
    // Tile access
    Element* GetTileElement(int i, int j);
    int GetTileElevation(int i, int j);
    
    // Object management
    void SelectObjects(int x, int y, std::vector<Object*>* dest);
    void MoveObject(Object* object, Point* from, Point* to);
    
    // Buildings
    void PopulateBuildingsIndices(std::vector<Building*>* buildings);
    
    // Victory locations
    inline int GetNumVictoryLocations() { return _victoryLocations.size(); }
    void GetVictoryLocation(int index, int *x, int *y, Nationality** nationality);
    const std::string& GetVictoryLocationName(int index);
    
    // Graphics
    TGA* GetBackground() { return _bgTGA; }
    const std::string& GetMiniName() { return _miniName; }
    const std::string& GetOverlandName() { return _overlandName; }

protected:
    // Grid dimensions
    int _nBlocksX, _nBlocksY;              // Tile count
    int _nPixelsPerBlockX, _nPixelsPerBlockY;  // Tile size (10x10)
    
    // Per-tile data arrays
    unsigned short* _elements;             // Element type indices
    unsigned char* _elevations;            // Height in meters
    Object** _objects;                     // Linked list heads per tile
    unsigned short* _buildingIndices;      // 1-based building index
    
    // Graphics
    TGA* _bgTGA;
    std::string _miniName;
    std::string _overlandName;
    
    // Buildings
    std::vector<Building*> _buildings;
    
    // Victory locations
    std::vector<VictoryLocation*> _victoryLocations;
};
```

#### 6.2.2 Tile Array Indexing

```cpp
// Convert 2D tile coordinates to 1D array index
int index = j * _nBlocksX + i;

// Access arrays
Element* element = g_Globals->World.Elements->GetElement(_elements[index]);
int elevation = _elevations[index];
Object* objectList = _objects[index];
int buildingIdx = _buildingIndices[index];  // 0 = no building
```

#### 6.2.3 Spatial Object Linked List

Each tile maintains an intrusive doubly-linked list of objects:

```
Tile (i,j)
  └── _objects[j*nx+i] → ObjectA
                           ├── NextObject → ObjectB
                           └── PrevObject → NULL
                        ObjectB
                           ├── NextObject → ObjectC
                           └── PrevObject → ObjectA
                        ObjectC
                           ├── NextObject → NULL
                           └── PrevObject → ObjectB
```

**MoveObject()** - Updates linked list when object moves between tiles:
```cpp
void Map::MoveObject(Object* object, Point* from, Point* to) {
    int oi = from->x / _nPixelsPerBlockX;
    int oj = from->y / _nPixelsPerBlockY;
    int ni = to->x / _nPixelsPerBlockX;
    int nj = to->y / _nPixelsPerBlockY;
    
    if(oi == ni && oj == nj) return;  // Same tile
    
    int oldIdx = oj * _nBlocksX + oi;
    int newIdx = nj * _nBlocksX + ni;
    
    // Remove from old tile
    if(object->PrevObject != NULL) {
        object->PrevObject->NextObject = object->NextObject;
    } else {
        _objects[oldIdx] = object->NextObject;
    }
    if(object->NextObject != NULL) {
        object->NextObject->PrevObject = object->PrevObject;
    }
    
    // Add to new tile (at head)
    object->PrevObject = NULL;
    object->NextObject = _objects[newIdx];
    if(object->NextObject != NULL) {
        object->NextObject->PrevObject = object;
    }
    _objects[newIdx] = object;
    
    // Update element reference
    object->SetTileElement(g_Globals->World.Elements->GetElement(_elements[newIdx]));
}
```

#### 6.2.4 Object Selection

**SelectObjects()** - Searches 3x3 tile area:
```cpp
void Map::SelectObjects(int x, int y, std::vector<Object*>* dest) {
    int ci = x / _nPixelsPerBlockX;  // Center tile
    int cj = y / _nPixelsPerBlockY;
    
    // 3x3 search area (clamped to bounds)
    int si = (ci - 1) >= 0 ? (ci - 1) : 0;
    int sj = (cj - 1) >= 0 ? (cj - 1) : 0;
    int di = (ci + 1) < _nBlocksX ? (ci + 1) : _nBlocksX - 1;
    int dj = (cj + 1) < _nBlocksY ? (cj + 1) : _nBlocksY - 1;
    
    // Search grid
    for(int j = sj; j <= dj; j++) {
        for(int i = si; i <= di; i++) {
            Object* obj = _objects[j * _nBlocksX + i];
            while(obj != NULL) {
                // Check team
                if(obj->GetTeam() == g_Globals->World.CurrentPlayer) {
                    if(obj->Contains(x, y)) {
                        dest->push_back(obj);
                    }
                }
                obj = obj->NextObject;
            }
        }
    }
}
```

#### 6.2.5 Coordinate Conversion

```cpp
void Map::ConvertTileToPosition(int i, int j, int *x, int *y) {
    *x = i * _nPixelsPerBlockX;
    *y = j * _nPixelsPerBlockY;
}

void Map::ConvertPositionToTile(int x, int y, int *i, int *j) {
    *i = x / _nPixelsPerBlockX;
    *j = y / _nPixelsPerBlockY;
    
    // Clamp to valid range
    if(*i < 0) *i = 0;
    if(*i >= _nBlocksX) *i = _nBlocksX - 1;
    if(*j < 0) *j = 0;
    if(*j >= _nBlocksY) *j = _nBlocksY - 1;
}
```

**Mega-tile conversion** (12x12 tiles = 120x120 pixels):
```cpp
void Map::ConvertMegaTileToPosition(int mi, int mj, int *x, int *y) {
    int megaSizeX = _nPixelsPerBlockX * 12;
    int megaSizeY = _nPixelsPerBlockY * 12;
    *x = mi * megaSizeX + (megaSizeX >> 1);  // Center of mega-tile
    *y = mj * megaSizeY + (megaSizeY >> 1);
}
```

### 6.3 Element Class (Terrain Types)

**Location**: `src/world/Element.h`, `src/world/Element.cpp`

#### 6.3.1 Element Definition

```cpp
class Element {
public:
    Element(void);
    ~Element(void);
    
    // Stance/height levels
    enum Level { 
        Prone = 0,   // On ground
        Low,         // Crawling/crouching low
        Medium,      // Crouching
        High,        // Standing
        Top          // Elevated position
    };
    
    int Index;                    // Database index
    int Height;                   // Visual height in meters
    std::string Name;             // Human-readable name
    bool BlocksHeight;            // Blocks LOS based on height?
    bool Passable;                // Can units walk through?
    
    // Indexed by Level enum
    unsigned char Cover[4];       // Cover percentage (0-100)
    unsigned char Hindrance[4];   // Movement hindrance
    unsigned short Protection[5]; // Damage protection (includes Top)
    float Movement[3];            // Speed multiplier (0=Prone, 1=Crouch, 2=Standing)
};
```

#### 6.3.2 Sample Elements

| Name | Height | Passable | Cover_Prone | Movement_Prone |
|------|--------|----------|-------------|----------------|
| Grass Field | 1 | true | 50 | 1.0 |
| Trench | 0 | true | 78 | 0.4 |
| Stone Wall | 8 | false | 93 | 0.5 |
| Deep Water | 0 | false | 0 | 0.0 |
| Wreck | 8 | false | 93 | 0.5 |

### 6.4 ElementManager

**Location**: `src/world/ElementManager.h`, `src/world/ElementManager.cpp`

```cpp
class ElementManager {
public:
    ElementManager(void);
    ~ElementManager(void);
    
    void Load(const std::filesystem::path& fileName);
    Element* GetElement(int index);
    
protected:
    std::vector<Element*> _elements;
};
```

**XML Format** (Elements.xml):
```xml
<Element>
    <Name>Grass Field</Name>
    <Height>1</Height>
    <Passable>true</Passable>
    <Blocks_Height>false</Blocks_Height>
    
    <Cover_Prone>50</Cover_Prone>
    <Cover_Low>22</Cover_Low>
    <Cover_Medium>0</Cover_Medium>
    <Cover_High>0</Cover_High>
    
    <Protection_Prone>25</Protection_Prone>
    <Protection_Low>10</Protection_Low>
    <Protection_Medium>0</Protection_Medium>
    <Protection_High>0</Protection_High>
    <Protection_Top>0</Protection_Top>
    
    <Hindrance_Prone>2</Hindrance_Prone>
    <Hindrance_Low>0</Hindrance_Low>
    <Hindrance_Medium>0</Hindrance_Medium>
    <Hindrance_High>0</Hindrance_High>
    
    <Soldier_Move_Prone>1</Soldier_Move_Prone>
    <Soldier_Move_Crouch>1.667</Soldier_Move_Crouch>
    <Soldier_Move_Standing>2</Soldier_Move_Standing>
</Element>
```

### 6.5 Building System

**Location**: `src/world/Building.h`, `src/world/Building.cpp`

#### 6.5.1 Building Class

```cpp
class Building {
public:
    Building(void);
    ~Building(void);
    
    Point Position;                    // Upper-left corner (world coords)
    std::vector<Point> BoundaryPoints; // Polygon vertices
    int* Tiles;                        // Array of tile indices
    int NumTiles;                      // Number of tiles
    
    TGA* GetExterior();
    TGA* GetInterior();
    void SetExterior(TGA* tga);
    void SetInterior(TGA* tga);
    
protected:
    TGA* _interiorGraphic;
    TGA* _exteriorGraphic;
};
```

#### 6.5.2 Building Loading (XML)

```xml
<Building>
    <Boundary>
        <Point><X>738</X><Y>631</Y></Point>
        <Point><X>738</X><Y>704</Y></Point>
        <Point><X>827</X><Y>704</Y></Point>
        <Point><X>829</X><Y>662</Y></Point>
        <Point><X>827</X><Y>632</Y></Point>
    </Boundary>
    <Position><X>738</X><Y>631</Y></Position>
    <ExteriorGraphic>Acqueville/building_graphics/exterior_000.tga</ExteriorGraphic>
    <InteriorGraphic>Acqueville/building_graphics/interior_000.tga</InteriorGraphic>
</Building>
```

#### 6.5.3 Building Index Population

```cpp
void Map::PopulateBuildingsIndices(std::vector<Building*>* buildings) {
    for(size_t b = 0; b < buildings->size(); b++) {
        Building* building = (*buildings)[b];
        
        // Iterate all tiles
        for(int j = 0; j < _nBlocksY; j++) {
            for(int i = 0; i < _nBlocksX; i++) {
                int tileX = i * _nPixelsPerBlockX + (_nPixelsPerBlockX >> 1);
                int tileY = j * _nPixelsPerBlockY + (_nPixelsPerBlockY >> 1);
                
                // Point-in-polygon test
                if(Screen::PointInRegion(tileX, tileY, &building->BoundaryPoints)) {
                    _buildingIndices[j * _nBlocksX + i] = b + 1;  // 1-based
                    
                    // Add to building's tile list
                    // ... resize and store tile index
                }
            }
        }
    }
}
```

#### 6.5.4 Building Rendering

```cpp
void World::RenderBuildings(Screen* screen, Rect* clip) {
    std::vector<Building*> buildingsToDraw;
    
    // First pass: collect buildings that need interior rendering
    for(int j = clip->y / 10; j < (clip->y + clip->h) / 10 + 1; j++) {
        for(int i = clip->x / 10; i < (clip->x + clip->w) / 10 + 1; i++) {
            if(j < 0 || j >= _currentMap->NumTiles.y ||
               i < 0 || i >= _currentMap->NumTiles.x) continue;
            
            int buildingIdx = _currentMap->_buildingIndices[j * _nBlocksX + i];
            if(buildingIdx > 0) {
                Building* building = _currentMap->_buildings[buildingIdx - 1];
                
                // Check if any object is in building or debug mode
                bool showInterior = g_Globals->World.bRenderBuildingInteriors;
                if(!showInterior) {
                    // Check for objects on building tiles
                    for(int k = 0; k < building->NumTiles; k++) {
                        int tileIdx = building->Tiles[k];
                        if(_currentMap->_objects[tileIdx] != NULL) {
                            showInterior = true;
                            break;
                        }
                    }
                }
                
                if(showInterior) {
                    if(std::find(buildingsToDraw.begin(), buildingsToDraw.end(),
                                building) == buildingsToDraw.end()) {
                        buildingsToDraw.push_back(building);
                    }
                }
            }
        }
    }
    
    // Second pass: render collected interiors
    for(auto* building : buildingsToDraw) {
        screen->Blit(building->GetInterior()->GetData(),
                     building->Position.x - screen->Origin.x,
                     building->Position.y - screen->Origin.y,
                     building->GetInterior()->GetWidth(),
                     building->GetInterior()->GetHeight(),
                     building->GetInterior()->GetWidth(),
                     building->GetInterior()->GetHeight(),
                     4);  // 4 bytes per pixel
    }
}
```

### 6.6 Line of Sight System

**Location**: `src/world/LineOfSight.h`, `src/world/LineOfSight.cpp`

#### 6.6.1 LOS Calculation

```cpp
class LineOfSight {
public:
    LineOfSight(void);
    ~LineOfSight(void);
    
    // Export LOS data to file (precomputation)
    void Export(Map* map, const std::filesystem::path& fileName);
    
    // Calculate LOS between two tiles
    bool CalculateLOSForTile(int x1, int y1, int x2, int y2,
                             int *ox, int *oy, int *oz, Map* map);

protected:
    void CalculateLOSForTile(int i, int j, int sx, int sy,
                            Map* map, unsigned char* buffer);
};
```

#### 6.6.2 3D LOS Algorithm

Uses 3D Bresenham line drawing:

```cpp
bool LineOfSight::CalculateLOSForTile(int x1, int y1, int x2, int y2,
                                      int *ox, int *oy, int *oz, Map* map) {
    // Calculate elevations (z)
    int z1 = (map->GetTileElevation(x1, y1) + 2) * HEIGHT_MODIFIER;  // +2m eye height
    int z2 = map->GetTileElevation(x2, y2) * HEIGHT_MODIFIER;
    
    // 3D Bresenham
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int dz = abs(z2 - z1);
    
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int sz = (z1 < z2) ? 1 : -1;
    
    int err1 = dx - dy;
    int err2 = dx - dz;
    
    int x = x1, y = y1, z = z1;
    
    while(true) {
        // Check if current position blocks LOS
        Element* e = map->GetTileElement(x, y);
        if(e->BlocksHeight) {
            int tileHeight = map->GetTileElevation(x, y) * HEIGHT_MODIFIER;
            
            // Calculate height of LOS line at this position
            int lineHeight = z1 + (z2 - z1) * distance / totalDistance;
            
            if(tileHeight > lineHeight) {
                // Blocked
                *ox = x;
                *oy = y;
                *oz = tileHeight;
                return false;
            }
        }
        
        if(x == x2 && y == y2 && z == z2) break;
        
        // Step along line
        int e2_1 = 2 * err1;
        int e2_2 = 2 * err2;
        
        if(e2_1 > -dy) {
            err1 -= dy;
            x += sx;
        }
        if(e2_1 < dx) {
            err1 += dx;
            y += sy;
        }
        if(e2_2 > -dz) {
            err2 -= dz;
            z += sz;
        }
    }
    
    return true;  // LOS clear
}
```

#### 6.6.3 LOS Export Format

Uses Run-Length Encoding (RLE):
- `0xFF AA BB` = RLE marker (AA = count, BB = value)
- `0x01-0xFE` = LOS percentage (1-254)
- `0x00` = End of scan line

### 6.7 VictoryLocation

**Location**: `src/world/VictoryLocation.cpp`

```cpp
class VictoryLocation {
public:
    std::string Name;
    int X, Y;                    // Mega-tile coordinates (12x12 blocks)
    int Value;                   // Strategic value
    int ControllingTeam;         // -1 = neutral, otherwise team index
    
    // Campaign linking
    std::string LinksToMapName;
    std::string LinksToVictoryLocationName;
};
```

**Rendering**:
- Displays national flag at position
- Smart text placement based on map edge proximity
- Links to other maps in campaign mode

---

*[Continue to Section 7: Configuration Files and Data Schemas]*
## 7. Configuration Files and Data Schemas

### 7.1 XML Parsing Infrastructure

**Parser**: TinyXML2 (version 11.0.0)
**Location**: `src/misc/tinyxml2.h`, `src/misc/tinyxml2.cpp`

**Common Pattern**:
```cpp
using namespace tinyxml2;

XMLDocument doc;
if(doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
    printf("Failed to load: %s\n", fileName.c_str());
    return;
}

XMLElement* root = doc.FirstChildElement("RootElement");
if(!root) return;

for(XMLElement* elem = root->FirstChildElement("ElementName");
    elem != nullptr;
    elem = elem->NextSiblingElement("ElementName")) {
    
    XMLElement* child = elem->FirstChildElement("ChildName");
    if(child && child->GetText()) {
        const char* value = child->GetText();
        // Process value
    }
}
```

### 7.2 Soldiers.xml

**Location**: `config/Soldiers.xml`

**Purpose**: Defines soldier types with animations, attributes, and capabilities.

#### Schema

```xml
<?xml version="1.0" encoding="utf-8"?>
<Soldiers>
  <Soldier>
    <Name>string</Name>                           <!-- Type identifier -->
    <PrimaryWeapon>string</PrimaryWeapon>         <!-- References Weapons.xml -->
    <PrimaryWeaponNumClips>int</PrimaryWeaponNumClips>
    <States>
      <State>
        <Name>string</Name>                       <!-- State name -->
        <Animation reverse="true|false">string</Animation>
      </State>
      <!-- 15 states minimum -->
    </States>
    <Attributes>
      <WalkingSpeed>float</WalkingSpeed>          <!-- m/s -->
      <WalkingAcceleration>float</WalkingAcceleration>
      <SneakingSpeed>float</SneakingSpeed>
      <SneakingAcceleration>float</SneakingAcceleration>
      <RunningSpeed>float</RunningSpeed>          <!-- 5.36 = historical max -->
      <RunningAcceleration>float</RunningAcceleration>
      <CanMove />      <!-- Boolean flag -->
      <CanMoveFast />
      <CanDefend />
      <CanSmoke />
      <CanFire />
      <CanSneak />
      <CanAmbush />
    </Attributes>
  </Soldier>
</Soldiers>
```

#### Soldier Types

| Name | Primary Weapon | Clips | Speeds (W/R/S) | Capabilities |
|------|----------------|-------|----------------|--------------|
| Garand | M1 Garand | 4 | 2.5/5.36/1.0 | All 7 flags |
| Carbine | M1A1 Carbine | 4 | 2.5/5.36/1.0 | All 7 flags |
| Thompson | Thompson M1A1 | 4 | 2.5/5.36/1.0 | All 7 flags |
| BAR | BAR | 6 | 2.5/5.36/1.0 | All 7 flags |
| Bazooka | Bazooka | 8 | 2.5/5.36/1.0 | All 7 flags |
| .30 Cal MG | .30 Cal MG | 8 | 2.5/5.36/1.0 | All 7 flags |

#### State-to-Animation Mapping

| State | Animation | Notes |
|-------|-----------|-------|
| Standing | Standing Rest | - |
| Prone | Prone Rest | - |
| Standing Firing | Standing Firing | - |
| Prone Firing | Prone Firing | - |
| Standing Reloading | Standing Reloading | 5 frames |
| Prone Reloading | Prone Reloading | 5 frames |
| Standing Up | Standing Up | 8 frames |
| Lying Down | Standing Up | reverse="true" |
| Sneaking | Crawling | - |
| Walking | Walking | 12 frames |
| Running | Running | 8 frames, 150ms |
| Dying Blown Up | Dying Blown Up | - |
| Dying Backward | Dying Backward | - |
| Dying Forward | Dying Forward | - |
| Dead | Dead | Static |

**Special Cases**:
- **Bazooka**: Prefixes animations with "Bazooka "
- **.30 Cal MG**: Prefixes animations with "Machine Gun "

### 7.3 Weapons.xml

**Location**: `config/Weapons.xml`

**Purpose**: Defines weapon statistics and behavior.

#### Schema

```xml
<Weapons>
  <!-- All times in milliseconds -->
  <!-- Ranges in meters, Weights in pounds -->
  <!-- BaseAccuracy is MOA (minutes of angle) -->
  <Weapon earthShaker="true|false">
    <Name>string</Name>              <!-- Weapon identifier -->
    <Icon>string</Icon>              <!-- UI icon name -->
    <Sound>string</Sound>            <!-- Sound effect key -->
    <Animation>string</Animation>    <!-- Effect type: Rifle/Machine Gun/Bazooka/Muzzle -->
    <RoundsPerClip>int</RoundsPerClip>
    <RoundsPerBurst>int</RoundsPerBurst>
    <ReloadTimeChamber>int</ReloadTimeChamber>  <!-- ms -->
    <ReloadTimeClip>int</ReloadTimeClip>        <!-- ms -->
    <TimeToFire>int</TimeToFire>                <!-- ms between shots -->
    <MaxEffectiveRange>int</MaxEffectiveRange>  <!-- meters -->
    <WeaponWeight>float</WeaponWeight>          <!-- pounds -->
    <ClipWeight>float</ClipWeight>             <!-- pounds -->
    <CoolRate>int</CoolRate>                    <!-- ms per cool tick -->
    <HeatRate>int</HeatRate>                    <!-- ms per heat tick -->
    <BaseAccuracy>float</BaseAccuracy>          <!-- MOA -->
  </Weapon>
</Weapons>
```

#### Weapon Definitions

| Name | Rounds/Clip | Burst | Reload(C/C) | Fire | Range | Weight | Heat/Cool | Acc | earthShaker |
|------|-------------|-------|-------------|------|-------|--------|-----------|-----|-------------|
| M1 Garand | 8 | 1 | 400/8000 | 400 | 460m | 10.0 | 300/500 | 1.00 | No |
| Blank | 0 | 0 | 0/0 | 0 | 0 | 0.0 | 0/0 | 0.00 | No |
| Thompson M1A1 | 30 | 5 | 500/10000 | 800 | 50m | 11.0 | 300/500 | 3.00 | No |
| M1A1 Carbine | 15 | 1 | 200/10000 | 500 | 200m | 5.2 | 300/500 | 1.00 | No |
| BAR | 20 | 3 | 200/10000 | 300 | 80m | 19.4 | 300/500 | 2.00 | No |
| Bazooka | 1 | 1 | 20000/10000 | 1000 | 80m | 9.0 | 1500/1000 | 2.00 | No |
| .30 Cal MG | 250 | 4 | 300/15000 | 300 | 80m | 45.0 | 300/700 | 2.00 | No |
| 7.5cm L48 | 1 | 1 | 8000/8000 | 500 | 80m | 0.0 | 1000/500 | 2.00 | **Yes** |

### 7.4 Effects.xml

**Location**: `config/Effects.xml`

**Purpose**: Defines visual effects with directional variants.

#### Schema

```xml
<Effects>
  <!-- Static: Fixed position (explosions) -->
  <Effect type="static">
    <Name>string</Name>
    <FrameHold>int</FrameHold>        <!-- ms per frame -->
    <Sound>string</Sound>             <!-- Optional -->
    <Graphic>path.x.y.tga</Graphic>   <!-- Explicit frame -->
    <!-- OR -->
    <Graphics>path/*.tga</Graphics>   <!-- Wildcard pattern -->
  </Effect>
  
  <!-- Dynamic: Follows entity (muzzle flashes) -->
  <Effect type="dynamic" place="turret|none">
    <Name>string</Name>
    <FrameHold>int</FrameHold>
    <Graphics>path/*.tga</Graphics>
  </Effect>
</Effects>
```

#### Effect Categories

| Category | Count | Description |
|----------|-------|-------------|
| Static | 2 | Explosion 60m, Dust Cloud |
| Rifle Directional | 8 | N, NE, E, SE, S, SW, W, NW muzzle flashes |
| Bazooka Directional | 8 | Rocket backblast directions |
| Machine Gun Directional | 8 | Uses rifle graphics |
| Muzzle Directional | 8 | Tank cannon flashes (place="turret") |

### 7.5 Squads.xml

**Location**: `config/Squads.xml`

**Purpose**: Defines squad compositions.

#### Schema

```xml
<Squads>
  <Squad>
    <Name>string</Name>
    <Icon>string</Icon>
    
    <!-- Infantry (0-32 per squad) -->
    <Soldier>
      <Title>string</Title>          <!-- Leader/Assistant Leader/Soldier -->
      <Rank>string</Rank>            <!-- Sergeant Major/Corporal/PFC/Private -->
      <Type>string</Type>            <!-- From Soldiers.xml -->
      <Camo>string</Camo>            <!-- Camouflage pattern -->
    </Soldier>
    
    <!-- Vehicle (0-1 per squad) -->
    <Vehicle>
      <Type>string</Type>            <!-- From Vehicles.xml -->
      <Soldier slot="int">           <!-- -1=driver, 0=gunner, 1=loader -->
        <Title>string</Title>
        <Rank>string</Rank>
        <Type>string</Type>
        <Camo>string</Camo>
      </Soldier>
    </Vehicle>
  </Squad>
</Squads>
```

#### Squad Types

| Name | Icon | Composition |
|------|------|-------------|
| BAR Rifle | Rifle Squad | 7 soldiers: Leader (Thompson), Asst (BAR), Soldier (BAR), 3x Garand, 1x Carbine |
| Bazooka | Bazooka Squad | 1 soldier: Leader (Thompson) |
| .30 Cal MG | MG Squad | 3 soldiers: Leader (Garand), Asst (Garand), Gunner (.30 Cal) |
| Panzer IVG | Tank Squad | 1 vehicle: Panzer IVG with 3 crew (gunner, loader, driver) |

### 7.6 Elements.xml

**Location**: `config/Elements.xml`

**Purpose**: Defines terrain element properties.

#### Schema

```xml
<Elements>
  <Element>
    <Name>string</Name>
    <Height>int</Height>
    <Passable>true|false</Passable>
    <Blocks_Height>true|false</Blocks_Height>
    
    <!-- Cover by stance (Prone/Low/Medium/High) -->
    <Cover_Prone>int</Cover_Prone>
    <Cover_Low>int</Cover_Low>
    <Cover_Medium>int</Cover_Medium>
    <Cover_High>int</Cover_High>
    
    <!-- Protection by level (Prone/Low/Medium/High/Top) -->
    <Protection_Prone>int</Protection_Prone>
    <Protection_Low>int</Protection_Low>
    <Protection_Medium>int</Protection_Medium>
    <Protection_High>int</Protection_High>
    <Protection_Top>int</Protection_Top>
    <Protection_Flag>Behind|Elevated|Sunken|InElement|None</Protection_Flag>
    
    <!-- Hindrance by stance -->
    <Hindrance_Prone>int</Hindrance_Prone>
    <Hindrance_Low>int</Hindrance_Low>
    <Hindrance_Medium>int</Hindrance_Medium>
    <Hindrance_High>int</Hindrance_High>
    
    <!-- Movement multipliers -->
    <Soldier_Move_Prone>float</Soldier_Move_Prone>
    <Soldier_Move_Crouch>float</Soldier_Move_Crouch>
    <Soldier_Move_Standing>float</Soldier_Move_Standing>
  </Element>
</Elements>
```

### 7.7 Nationalities.xml

**Location**: `config/Nationalities.xml`

```xml
<Nationalities>
  <Nationality>
    <Name>American</Name>
    <VictoryLocation>UI/Flags/Static/american.14.9.tga</VictoryLocation>
    <MiniMap>UI/Flags/Static/minimap_american.tga</MiniMap>
  </Nationality>
</Nationalities>
```

### 7.8 Terrain.xml

**Location**: `config/Terrain.xml`

```xml
<Widgets>
  <Widget>
    <Name>Small Tree 1</Name>
    <Graphic>Terrain/big_tree_3 (471).17.18.tga</Graphic>
    <Index>106</Index>
  </Widget>
</Widgets>
```

### 7.9 Animation XMLs

#### SoldierAnimations.xml

```xml
<Animations dir="Soldiers/Rifle" image="spr*" mask="msk*">
  <Animation>
    <Name>Standing Rest</Name>
    <Directions>8</Directions>
    <NumFrames>1</NumFrames>
    <Time>200</Time>
    <FirstDirection>North</FirstDirection>
    <TransparentColor>16777215</TransparentColor>  <!-- White -->
  </Animation>
</Animations>
```

**Attributes**:
- **dir**: Subdirectory in graphics/
- **image**: Sprite file pattern
- **mask**: Mask file pattern
- **Directions**: Number of directional variants (8)
- **NumFrames**: Frames per direction
- **Time**: Milliseconds per frame
- **FirstDirection**: Starting direction
- **TransparentColor**: 24-bit RGB color key

### 7.10 Additional XML Configuration Files

The following XML files exist but are not detailed above:

#### Vehicles.xml
**Location**: `config/Vehicles.xml`

Defines vehicle types (tanks, vehicles) with armor, speed, crew positions, and weapon mounts.

#### CombatUI.xml
**Location**: `config/CombatUI.xml`

Defines UI widget positions and graphics for the combat interface.

#### ContextMenuWidgets.xml
**Location**: `config/ContextMenuWidgets.xml`

Defines the context menu widget graphics (light/dark/negative states).

#### Animation XMLs
- **SoldierAnimations.xml**: Base soldier animation definitions
- **BazookaAnimations.xml**: Bazooka-specific animations  
- **MachineGunAnimations.xml**: .30 Cal MG specific animations
- **SoldierDead.xml**: Death pose definitions
- **SoldierDeaths.xml**: Death animation definitions

#### Icon and Widget XMLs
- **Icons.xml**: UI icon definitions
- **WeaponIcons.xml**: Weapon icon mappings
- **Colors.xml**: Color palette definitions
- **ColorModifiers.xml**: Color modification rules

#### Audio XMLs
- **SoundEffects.xml**: Sound effect file mappings
- **EnglishVoices.xml**: Voice line definitions

### 7.11 Text Configuration Files

#### SoldierStates.txt

**Location**: `config/SoldierStates.txt`

Line-number indexed (1-based):
```
1: Standing
2: Prone
3: Stopped
4: Moving
5: Firing
6: Walking
7: WalkingSlow
8: Crawling
9: Running
10: Reloading
11: DyingBlownUp
12: DyingBackward
13: DyingForward
14: Dead
15: Reloaded
16: OutOfAmmo
17: NoTarget
18: FindingCover
19: Following
20: FollowingInFormation
21: Defending
22: Ambushing
```

#### SoldierActions.txt

**Location**: `config/SoldierActions.txt`

Tab-separated format:
```
Name                    Group   Time    Requirements                    Changes
StandingFire            Fire    0       Stopped,Standing,Reloaded       +Firing,-NoTarget
ProneFire               Fire    0       Stopped,Prone,Reloaded          +Firing,-NoTarget
Run                     Move    0       Standing                        +Running,-Walking,-WalkingSlow,-Crawling,-Stopped,+Moving,-Firing
...
```

#### USNames.txt

**Location**: `config/USNames.txt`

Pool of 473 American surnames for random soldier naming. Read until `#` marker.

---

*[Continue to Section 8: Asset Structure and File Formats]*
## 8. Asset Structure and File Formats

### 8.1 Graphics Directory Structure

```
graphics/
├── CombatContextMenu/     # Context menu UI elements
├── Effects/               # Visual effects (26 subdirectories)
│   ├── rifle_n/           # Rifle muzzle flashes (5 frames)
│   ├── rifle_ne/
│   ├── ... (8 directions)
│   ├── bazooka_n/         # Rocket effects (12 frames)
│   ├── ... (8 directions)
│   ├── muzzle_n/          # Tank muzzle flashes (17 frames)
│   ├── ... (8 directions)
│   ├── Explosion/         # 33 explosion frames
│   └── Dustcloud/         # Vehicle dust effects
├── Resources/             # Application icon
├── Soldiers/              # Soldier animations (19 subdirectories)
│   ├── Rifle/             # ~2,600 files - Standard infantry
│   ├── MG/                # ~1,400 files - Machine gunner
│   ├── Bazooka/           # ~1,400 files - Anti-tank
│   ├── Dying/             # ~560 files - Death animations
│   ├── Dead 1/ ... Dead 6/ # Static dead poses
│   ├── Flame/             # ~1,400 files - Flamethrower
│   ├── Burned/            # Charred corpses
│   ├── Standing Burning/  # On fire animations
│   ├── Prone Burning/
│   ├── Surrendering/      # ~1,000 files
│   ├── Kneeling Mine/     # Mine placement
│   └── Prone Mine/
├── Terrain/               # Tree sprites (5 files)
├── UI/                    # User interface
│   ├── Actions/           # Action indicators
│   ├── Cursors/           # Mouse cursors (16 files)
│   ├── Flags/             # National flags
│   │   ├── Animated/      # 40 animated flags
│   │   └── Static/        # Static icons
│   ├── Ranks/             # Military rank icons
│   ├── Status/            # Status indicators
│   ├── Teams/             # Team panel graphics
│   ├── Weapons/           # Weapon icons
│   ├── DejaVuSans.ttf     # UI font
│   └── ui_game_*.tga      # UI elements
└── Vehicles/              # Tank graphics (3 files)
    ├── panzer_IVG_hull.12.21.tga
    ├── panzer_IVG_turret.8.30.tga
    └── panzer_IVG_wreck.11.21.tga
```

### 8.2 TGA File Naming Convention

#### 8.2.1 Format Specification

```
{name}.{optional_id}.{originX}.{originY}.tga
```

#### 8.2.2 Parsing Algorithm

**CRITICAL**: Strip `.tga` extension BEFORE parsing coordinates.

```cpp
// CORRECT parsing order
void ParseOriginFromFilename(TGA* tga, const std::string& filename) {
    std::string fName = filename;
    
    // Step 1: Strip .tga
    size_t extDot = fName.rfind('.');
    if(extDot != std::string::npos) {
        fName = fName.substr(0, extDot);
    }
    
    // Step 2: Find Y coordinate
    size_t yDot = fName.rfind('.');
    if(yDot == std::string::npos) return;
    
    std::string yStr = fName.substr(yDot + 1);
    fName = fName.substr(0, yDot);
    
    // Step 3: Find X coordinate
    size_t xDot = fName.rfind('.');
    if(xDot == std::string::npos) return;
    
    std::string xStr = fName.substr(xDot + 1);
    
    // Parse
    int x = atoi(xStr.c_str());
    int y = atoi(yStr.c_str());
    tga->SetOrigin(x, y);
}
```

#### 8.2.3 Examples

| Filename | X Origin | Y Origin | Purpose |
|----------|----------|----------|---------|
| `image001.-15.-3.tga` | -15 | -3 | Muzzle flash offset |
| `big_tree_3 (471).17.18.tga` | 17 | 18 | Tree with ID |
| `panzer_IVG_turret.8.30.tga` | 8 | 30 | Tank turret pivot |
| `ui_game_donut_fat_yellow.6.7.tga` | 6 | 7 | UI element origin |

#### 8.2.4 Color Depth and Format

**TGA Header**:
```cpp
typedef struct {
    char  idlength;              // ID field length
    char  colourmaptype;         // 0 = none
    char  datatypecode;          // 2 = uncompressed RGB
    short int colourmaporigin;
    short int colourmaplength;
    char  colourmapdepth;
    short int x_origin;
    short int y_origin;
    short width;
    short height;
    char  bitsperpixel;          // 16, 24, or 32
    char  imagedescriptor;
} TGA_HEADER;
```

**Supported Formats**:
- **Type 2** (Uncompressed RGB): Fully supported
- **Type 10** (RLE Compressed): Parsed but not fully implemented
- **32-bit BGRA**: Native format with alpha
- **24-bit BGR**: Converted to 32-bit with 0xFF alpha
- **16-bit RGB**: Converted to 32-bit with bit expansion

**Internal Storage**:
- All images converted to 32-bit ARGB
- Pixel data as `unsigned char*` array
- Row order flipped during load (TGA stores bottom-to-top)

**Pixel Layout** (BGRA order):
```
Byte 0: Blue
Byte 1: Green
Byte 2: Red
Byte 3: Alpha
```

### 8.3 Map Files

#### 8.3.1 Map XML Format

**Location**: `maps/{MapName}/{MapName}.xml`

```xml
<?xml version="1.0" encoding="utf-8"?>
<Map>
    <Name>Acqueville</Name>
    <Background>Acqueville/Acqueville.bgm.tga</Background>
    <Mini>Acqueville/Acqueville.mmm.tga</Mini>
    <Overland>Acqueville/Acqueville.ovm.tga</Overland>
    <Elements>Acqueville/Acqueville.txt</Elements>
    <Buildings>Acqueville/Acqueville.buildings.xml</Buildings>
    <VictoryLocations>
        <VL>
            <X>4</X>                    <!-- MegaTile X -->
            <Y>9</Y>                    <!-- MegaTile Y -->
            <Name>Hotel Atlantic</Name>
            <Value>3</Value>
            <LinksTo>
                <MapName>MapName</MapName>
                <VLName>VictoryLocationName</VLName>
            </LinksTo>
        </VL>
    </VictoryLocations>
</Map>
```

**Coordinate System**:
- MegaTile = 12x12 tiles = 120x120 pixels
- Used for victory location placement

#### 8.3.2 Map Text Format

**Location**: `maps/{MapName}/{MapName}.txt`

Legacy Close Combat format:
```
10                              <- Scale factor

60 X Max                        <- MegaTiles X
57 Y Max                        <- MegaTiles Y
0                               <- Unknown
0                               <- Unknown
Idx	E0	E1	E2	...	E15	Elev
&                               <- Section delimiter
0	25	25	25	25	6	6	6	...	0	0	0	0	<- Data row
```

**Data Row Format**:
```
{RowIndex}\t{E0}\t{E1}\t...\t{E15}\t{Elev0}\t{Elev1}\t...\t{Elev15}
```

- 16 element indices (E0-E15)
- 16 elevation values
- Tab-separated
- Each row represents one macroblock row

#### 8.3.3 Buildings XML Format

**Location**: `maps/{MapName}/{MapName}.buildings.xml`

```xml
<Buildings>
    <Building>
        <Boundary>
            <Point><X>738</X><Y>631</Y></Point>
            <Point><X>738</X><Y>704</Y></Point>
            <Point><X>827</X><Y>704</Y></Point>
            <Point><X>829</X><Y>662</Y></Point>
            <Point><X>827</X><Y>632</Y></Point>
        </Boundary>
        <Position><X>738</X><Y>631</Y></Position>
        <ExteriorGraphic>Acqueville/building_graphics/exterior_000.tga</ExteriorGraphic>
        <InteriorGraphic>Acqueville/building_graphics/interior_000.tga</InteriorGraphic>
    </Building>
</Buildings>
```

**Building Graphics**:
- Located in `maps/{MapName}/building_graphics/`
- Pairs: `exterior_XXX.tga` and `interior_XXX.tga`
- 31 buildings in Acqueville map = 62 TGA files

### 8.4 Audio Assets

#### 8.4.1 Directory Structure

```
Sounds/
├── Effects/              # Sound effects
│   ├── 30 cal MG-0010.wav
│   ├── bar-0008.wav
│   ├── bazooka-0011.wav
│   ├── dying-0061.wav
│   ├── explosion-0050.wav
│   ├── large tank gun-0014.wav
│   ├── rifle-0028.wav
│   └── thompson-0005.wav
└── English Voices/       # Unit voice responses
    ├── 0038 - move completed.wav
    ├── 0041 - awaiting orders.wav
    └── 0053 - no clear path.wav
```

#### 8.4.2 SoundEffects.xml

```xml
<Sounds>
    <Sound>
        <Name>explosion</Name>
        <File>Effects/explosion-0050.wav</File>
    </Sound>
    <Sound>
        <Name>Rifle</Name>
        <File>Effects/rifle-0028.wav</File>
    </Sound>
</Sounds>
```

#### 8.4.3 EnglishVoices.xml

```xml
<Sounds>
    <Sound>
        <Name>awaiting orders</Name>
        <File>English Voices/0041 - awaiting orders.wav</File>
    </Sound>
</Sounds>
```

### 8.5 Configuration Files Summary

| File | Purpose |
|------|---------|
| BazookaAnimations.xml | Bazooka animation definitions |
| ColorModifiers.xml | Uniform color modifications |
| Colors.xml | Color palette |
| CombatUI.xml | Combat interface layout |
| ContextMenuWidgets.xml | Context menu widgets |
| Effects.xml | Visual effect definitions |
| Elements.xml | Terrain elements (~100 types) |
| EnglishVoices.xml | Voice references |
| Icons.xml | UI icons |
| MachineGunAnimations.xml | MG animation definitions |
| Nationalities.xml | Faction definitions |
| SoldierActions.txt | Action definitions |
| SoldierAnimations.xml | Rifle animation definitions |
| SoldierDead.xml | Dead pose animations |
| SoldierDeaths.xml | Death animation sequences |
| SoldierStates.txt | State machine states |
| Soldiers.xml | Soldier type definitions |
| SoundEffects.xml | Sound effect references |
| Squads.xml | Squad compositions |
| Terrain.xml | Terrain sprite mappings |
| USNames.txt | Soldier name pool (473 names) |
| Vehicles.xml | Vehicle definitions |
| WeaponIcons.xml | Weapon icon mappings |
| Weapons.xml | Weapon statistics |

### 8.6 Asset Loading Pipeline

#### 8.6.1 Animation Loading Sequence

```cpp
SoldierAnimationManager::LoadAnimations(xmlFile) {
    1. Parse XML configuration
       - Read dir, image, mask attributes
       - Parse each <Animation> element
    
    2. Scan directory
       searchPattern = graphicsDir / directory / "spr*"
       for each .tga starting with "spr":
           add to files[]
           create mask filename (replace "spr" with "msk")
           add to masks[]
       sort(files)
       sort(masks)
    
    3. Load TGA files
       for each frame:
           load sprite: TGA::Create(fName)
           load mask: TGA::Create(mName)
           parse origin from filename
           create MaskFrame
    
    4. Create Animation objects
       group frames by direction (8 dirs)
       store in _animations vector
}
```

#### 8.6.2 Effect Loading Sequence

```cpp
EffectManager::LoadEffects(xmlFile) {
    1. Parse XML
       - type="static" or type="dynamic"
       - place="turret" attribute
    
    2. Static effects
       - Read explicit <Graphic> elements
       - Load each TGA
    
    3. Dynamic effects
       - Parse <Graphics> wildcard
       - GetFiles() to collect matches
       - Sort alphabetically
       - Load each TGA
    
    4. Parse origin from filenames
       - STRIP .tga FIRST
       - Then parse coordinates
}
```

#### 8.6.3 Soldier Creation Pipeline

```cpp
SoldierManager::CreateSoldier(type) {
    1. Find template by type name
    
    2. Create Soldier object
       - Assign random name from USNames.txt
       - Equip primary weapon from WeaponManager
    
    3. Assign animations
       for each state in template:
           anim = AnimationManager::GetAnimation(state.Animation)
           soldier->_animations[state] = anim
    
    4. Set capabilities from XML flags
    5. Set speed/acceleration values
}
```

### 8.7 Asset Creation Guidelines

#### 8.7.1 Soldier Animations

1. **File Naming**:
   - Format: `spr{index}.{originX}.{originY}.tga`
   - Matching mask: `msk{index}.{originX}.{originY}.tga`
   - Keep indices sequential

2. **Origin Points**:
   - Origin (0,0) = feet position
   - Negative Y = up (for muzzle flashes)
   - Positive X = right

3. **Color Key**:
   - White (255,255,255) = transparent for soldiers
   - Black (0,0,0) = transparent for effects

4. **Frame Counts**:
   - Must be multiples of 8 for 8-directional
   - Walking: 12 frames per direction
   - Running: 8 frames per direction
   - Firing: 1 frame per direction

#### 8.7.2 Effects

1. **Filename Format**:
   - `image{NNN}.{originX}.{originY}.tga`
   - Must strip .tga before parsing

2. **Origin Rules**:
   - Muzzle flashes: origin at barrel tip
   - Explosions: origin at center

---

*[Continue to Section 9: UI System and Combat Module]*
## 9. UI System and Combat Module

### 9.1 CombatModule

**Location**: `src/application/CombatModule.h`, `src/application/CombatModule.cpp`

The `CombatModule` is the main game mode, managing the combat UI, world simulation, and user interaction.

#### 9.1.1 Class Definition

```cpp
class CombatModule : public Module {
public:
    CombatModule();
    ~CombatModule(void);
    
    // Module interface
    virtual void Initialize(void *app);
    virtual void Simulate(long dt);
    virtual void Render(Screen *screen);
    
    // Mouse events
    virtual void LeftMouseDown(int x, int y);
    virtual void LeftMouseUp(int x, int y);
    virtual void LeftMouseDrag(int x, int y);
    virtual void RightMouseDown(int x, int y);
    virtual void RightMouseUp(int x, int y);
    virtual void RightMouseDrag(int x, int y);
    virtual void MiddleMouseDown(int x, int y);
    virtual void MiddleMouseUp(int x, int y);
    virtual void MiddleMouseDrag(int x, int y);
    
    // Keyboard events
    virtual void KeyUp(int key);
    virtual void KeyDown(int key);

protected:
    // UI Managers
    WidgetManager* _uiManager;           // Main UI widgets
    WidgetManager* _iconManager;         // Unit/team icons
    WidgetManager* _weaponIconManager;   // Weapon icons
    WidgetManager* _terrainManager;      // Terrain indicators
    
    // Background graphics
    TGA* _longBottomBackground;          // Bottom command bar
    TGA* _unitBackground;                // Squad panel background
    TGA* _teamBarBlank;                  // Empty team bar
    TGA* _airstrikeNeg;                  // Disabled buttons
    TGA* _artilleryNeg;
    TGA* _bombardNeg;
    TGA* _activeTeamPanel;               // Active squad panel
    
    // Core systems
    World* _currentWorld;
    MiniMap* _currentMiniMap;
    AnimationManager* _animationManager;
    SoldierManager* _soldierManager;
    SoundManager* _soundManager;
    SoundManager* _soundEffectsManager;
    ColorModifierManager* _colorModifierManager;
    
    // Marks
    Mark* _marks;
    
    // Panel visibility
    bool _showTeamPanel;                 // F6 toggle
    bool _showMiniMap;                   // F5 toggle
    bool _showUnitPanel;                 // F7 toggle
    
    // FPS tracking (added in SDL2 port)
    long _frameTimeAccumulator;          // Accumulated frame time
    int _frameCount;                     // Frame count for averaging
    float _currentFPS;                   // Current FPS display value
    float _currentFrameTime;             // Current frame time in ms
    
    // App reference
    void* _app;
};
```

#### 9.1.2 Initialization

```cpp
void CombatModule::Initialize(void *app) {
    _app = app;
    
    // Create and initialize world
    _currentWorld = new World();
    _currentWorld->Load("maps/Acqueville/Acqueville.xml",
                        _soldierManager, _animationManager);
    
    // Create minimap
    _currentMiniMap = new MiniMap();
    _currentMiniMap->SetWorld(_currentWorld);
    _currentWorld->SetMiniMap(_currentMiniMap);
    
    // Load UI graphics
    _longBottomBackground = TGA::Create(
        g_Globals->Application.GraphicsDirectory / 
        "UI/Teams/Bottom Background.tga");
    _unitBackground = TGA::Create(
        g_Globals->Application.GraphicsDirectory /
        "UI/Teams/Team Panel Background.tga");
    // ... etc
    
    // Load widget managers
    _uiManager = new WidgetManager();
    _uiManager->LoadWidgets(
        g_Globals->Application.ConfigDirectory / "CombatUI.xml");
    
    _iconManager = new WidgetManager();
    _iconManager->LoadWidgets(
        g_Globals->Application.ConfigDirectory / "Icons.xml");
    
    _weaponIconManager = new WidgetManager();
    _weaponIconManager->LoadWidgets(
        g_Globals->Application.ConfigDirectory / "WeaponIcons.xml");
    
    _terrainManager = new WidgetManager();
    _terrainManager->LoadWidgets(
        g_Globals->Application.ConfigDirectory / "Terrain.xml");
    
    // Initialize marks
    _marks = new Mark();
    g_Globals->World.Marks = _marks;
}
```

#### 9.1.3 Render Flow

```cpp
void CombatModule::Render(Screen *screen) {
    // Set clip rectangle for world rendering
    // (exclude UI panels)
    Rect worldClip;
    worldClip.x = 0;
    worldClip.y = 0;
    worldClip.w = screen->GetWidth();
    worldClip.h = screen->GetHeight() - _longBottomBackground->GetHeight();
    
    if(_showTeamPanel) {
        worldClip.w -= _activeTeamPanel->GetWidth();
    }
    
    // Render world
    screen->SetClippingRectangle(worldClip.x, worldClip.y,
                                  worldClip.w, worldClip.h);
    _currentWorld->Render(screen, &worldClip);
    
    // Reset clip for UI
    screen->SetClippingRectangle(0, 0,
                                  screen->GetWidth(),
                                  screen->GetHeight());
    
    // Render minimap
    if(_showMiniMap) {
        RenderMiniMap(screen);
    }
    
    // Render team panel
    if(_showTeamPanel) {
        RenderTeamPanel(screen);
    }
    
    // Render unit panel
    if(_showUnitPanel) {
        RenderUnitPanel(screen);
    }
    
    // Render bottom command bar
    RenderBottomBar(screen);
}
```

#### 9.1.4 UI Layout

```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│                    GAME WORLD                           │
│                    (clipped region)                     │
│                                                         │
│  ┌──────────────┐                                       │
│  │   MINIMAP    │  (if _showMiniMap)                    │
│  │  [=======]   │  - Position: top-left                 │
│  │  [ O--> ]    │  - Yellow viewport rect               │
│  │  [=======]   │  - Unit dots (blue/green/red)         │
│  └──────────────┘  - Victory locations                  │
│                                                         │
├─────────────────────────────────────────────────────────┤
│  SQUAD PANEL (if _showTeamPanel)                        │
│  ┌───┬───┬───┐                                          │
│  │ 0 │ 3 │ 6 │  Tiled background panel                  │
│  ├───┼───┼───┤  - Squad icons + names                   │
│  │ 1 │ 4 │ 7 │  - Action indicators                     │
│  ├───┼───┼───┤  - 3 squads per column                   │
│  │ 2 │ 5 │   │                                          │
│  └───┴───┴───┘                                          │
├─────────────────────────────────────────────────────────┤
│  BOTTOM COMMAND BAR                                     │
│  [Art][Air][Nav] [Team Bar]                             │
│   Disabled       - Selected squad icon                  │
│   buttons        - Squad name + rank                    │
│                  - Unit status heads                    │
│                  - Quality indicator                    │
└─────────────────────────────────────────────────────────┘
```

#### 9.1.5 Input Handling - F-Key Toggle System

The CombatModule provides comprehensive debug and UI toggle functionality via F-keys:

**Key Mappings** (updated for SDL2 port):

| Key | Code | Action | Global Variable |
|-----|------|--------|-----------------|
| F1 | 112 | Toggle help text overlay | `bRenderHelpText` |
| F2 | 113 | Toggle FPS/stats display | `bRenderStats` |
| F3 | 114 | Toggle path rendering | `bRenderPaths` |
| F4 | 115 | Toggle weapon fan/LOS | `bWeaponFan` |
| F5 | 116 | Toggle minimap | `_showMiniMap` |
| F6 | 117 | Toggle team panel | `_showTeamPanel` |
| F7 | 118 | Toggle unit panel | `_showUnitPanel` |
| F8 | 119 | Cycle building display mode | Multiple flags |
| F9 | 120 | Toggle terrain elements | `bRenderElements` |
| F10 | 121 | Toggle bounding boxes | `bRenderBoundingBoxes` |
| Left Arrow | 0x25 | Scroll left | - |
| Up Arrow | 0x26 | Scroll up | - |
| Right Arrow | 0x27 | Scroll right | - |
| Down Arrow | 0x28 | Scroll down | - |

**Implementation** (src/application/CombatModule.cpp:616-680):
```cpp
void CombatModule::KeyUp(int key) {
    switch(key) {
        case 112: /* F1 */
            g_Globals->World.bRenderHelpText = !g_Globals->World.bRenderHelpText;
            break;
        case 113: /* F2 */
            g_Globals->World.bRenderStats = !g_Globals->World.bRenderStats;
            break;
        case 114: /* F3 */
            g_Globals->World.bRenderPaths = !g_Globals->World.bRenderPaths;
            break;
        case 115: /* F4 */
            g_Globals->World.bWeaponFan = !g_Globals->World.bWeaponFan;
            break;
        case 116: /* F5 */
            _showMiniMap = !_showMiniMap;
            break;
        case 117: /* F6 */
            _showTeamPanel = !_showTeamPanel;
            break;
        case 118: /* F7 */
            _showUnitPanel = !_showUnitPanel;
            break;
        case 119: /* F8 */
            // Cycle: Interiors -> Outlines -> Elevation -> NULL
            if(g_Globals->World.bRenderBuildingInteriors) {
                g_Globals->World.bRenderBuildingOutlines = true;
                g_Globals->World.bRenderElevation = false;
                g_Globals->World.bRenderBuildingInteriors = false;
            } else if(g_Globals->World.bRenderBuildingOutlines) {
                g_Globals->World.bRenderBuildingOutlines = false;
                g_Globals->World.bRenderElevation = true;
            } else if(g_Globals->World.bRenderElevation) {
                g_Globals->World.bRenderElevation = false;
            } else {
                g_Globals->World.bRenderBuildingInteriors = true;
            }
            break;
        case 120: /* F9 */
            g_Globals->World.bRenderElements = !g_Globals->World.bRenderElements;
            break;
        case 121: /* F10 */
            g_Globals->World.bRenderBoundingBoxes = !g_Globals->World.bRenderBoundingBoxes;
            break;
        default:
            _currentWorld->KeyUp(key);
            break;
    }
}
```

#### 9.1.6 FPS and Frame Time Tracking

The CombatModule tracks performance metrics for display:

**Implementation** (src/application/CombatModule.cpp:207-223):
```cpp
void CombatModule::Simulate(long dt) {
    _currentWorld->Simulate(dt);

    // Track FPS and frame time
    _frameTimeAccumulator += dt;
    _frameCount++;
    
    // Update FPS display every 500ms
    if(_frameTimeAccumulator >= 500) {
        _currentFrameTime = (float)_frameTimeAccumulator / (float)_frameCount;
        _currentFPS = (float)_frameCount * 1000.0f / (float)_frameTimeAccumulator;
        _frameTimeAccumulator = 0;
        _frameCount = 0;
    }
}
```

**Rendering** (src/application/CombatModule.cpp:517-564):
```cpp
// Render FPS and frame time stats if enabled
if(g_Globals->World.bRenderStats) {
    Color yellow(255,255,0);
    char stats[64];
    snprintf(stats, sizeof(stats), "FPS: %.1f  Frame: %.2fms", 
             _currentFPS, _currentFrameTime);
    // Position in top-right corner
    g_Globals->World.Fonts->Render(screen, stats, screen->GetWidth() - 140, 10, &yellow);
}

// Render help text if enabled
if(g_Globals->World.bRenderHelpText) {
    Color yellow(255,255,0);
    int y = 10;
    int lineHeight = 14;
    g_Globals->World.Fonts->Render(screen, "=== CONTROLS ===", 10, y, &yellow);
    y += lineHeight;
    g_Globals->World.Fonts->Render(screen, "F1: Toggle this help", 10, y, &yellow);
    // ... (all F-key mappings listed)
}
```

**Key Mappings** (legacy reference):
        _currentWorld->KeyUp(key);
        break;
    }
}
```

### 9.2 CombatContextMenu

**Location**: `src/graphics/CombatContextMenu.h`

#### 9.2.1 Menu Choices

```cpp
enum ContextMenuChoice {
    Move,       // Move order
    MoveFast,   // Run order
    Fire,       // Attack order
    Ambush,     // Ambush order
    Smoke,      // Deploy smoke
    Sneak,      // Sneak order
    Defend,     // Defend order
    None        // No selection
};
```

#### 9.2.2 Widget States

Each choice has three states:
- **Light**: Hover/active (brighter)
- **Dark**: Normal state (darker)
- **Neg**: Disabled/unavailable (grayed)

| Choice | Light Widget | Dark Widget | Neg Widget |
|--------|--------------|-------------|------------|
| Move | Order Move Light | Order Move Dark | Order Move Neg |
| MoveFast | Order Move Fast Light | Order Move Fast Dark | Order Move Fast Neg |
| Fire | Order Fire Light | Order Fire Dark | Order Fire Neg |
| Sneak | Order Sneak Light | Order Sneak Dark | Order Sneak Neg |
| Smoke | Order Smoke Light | Order Smoke Dark | Order Smoke Neg |
| Defend | Order Defend Light | Order Defend Dark | Order Defend Neg |
| Ambush | Order Ambush Light | Order Ambush Dark | Order Ambush Neg |

#### 9.2.3 Menu Layout

```
┌──────────────────┐
│ [Move]           │  y+2 offset per item
│ [Move Fast]      │
│ [Fire]           │
│ [Sneak]          │
│ [Smoke]          │
│ [Defend]         │
│ [Ambush]         │
└──────────────────┘
```

Position adjusts to stay within screen bounds.

### 9.3 MiniMap

**Location**: `src/world/MiniMap.h`, `src/world/MiniMap.cpp`

#### 9.3.1 Core Properties

```cpp
class MiniMap {
public:
    Point Position;                    // Screen position
    int _zoomWidth, _zoomHeight;       // Viewport rect size
    float _widthPct, _heightPct;       // Scale percentages
    int _x, _y;                        // Rect position within minimap
    int _visibleWidth, _visibleHeight;
    TGA* _tga;                         // Minimap image
    World* _parentWorld;
};
```

#### 9.3.2 Render Elements

1. **Minimap Image**: Scaled-down terrain overview
2. **Border**: Black outer (2px) + white inner (1px)
3. **Victory Locations**: Flag icons
4. **Unit Markers**:
   - Blue: Player units
   - Green: Allied units
   - Red: Enemy units
5. **Yellow Rectangle**: Current viewport indicator

#### 9.3.3 Interaction

```cpp
bool Contains(int x, int y);           // Hit test
void LeftMouseDown(int x, int y);      // Start drag
void LeftMouseUp(int x, int y);        // Click to center
void LeftMouseDrag(int x, int y);      // Drag to pan
```

Click/drag updates world origin:
```cpp
void MiniMap::LeftMouseDrag(int x, int y) {
    // Convert minimap coords to world coords
    int worldX = (x - Position.x) / _widthPct;
    int worldY = (y - Position.y) / _heightPct;
    
    // Center viewport on click
    int ox = worldX - (_parentWorld->GetWidth() / 2);
    int oy = worldY - (_parentWorld->GetHeight() / 2);
    
    _parentWorld->SetOrigin(ox, oy);
}
```

### 9.4 Mark System

**Location**: `src/graphics/Mark.h`, `src/graphics/Mark.cpp`

#### 9.4.1 Colors

```cpp
enum class Color {
    Blue = 0,    // Waypoints, selection
    Purple,      // Special markers
    Red,         // Enemy targeting
    Yellow,      // Caution/warning
    Orange,      // Highlight
    Green,       // Allied/friendly
    Brown,       // Terrain markers
    NumColors
};
```

#### 9.4.2 Cursor Files

| Color | Widget Name | File |
|-------|-------------|------|
| Blue | Mark Blue | UI/Cursors/Mark Blue.tga |
| Purple | Mark Purple | UI/Cursors/Mark Purple.tga |
| Red | Mark Red | UI/Cursors/Mark Red.tga |
| Yellow | Mark Yellow | UI/Cursors/Mark Yellow.tga |
| Orange | Mark Orange | UI/Cursors/Mark Orange.tga |
| Green | Mark Green | UI/Cursors/Mark Green.tga |
| Brown | Mark Brown | UI/Cursors/Mark Brown.tga |

### 9.5 WidgetManager

**Location**: `src/graphics/WidgetManager.h`

```cpp
class WidgetManager {
public:
    void LoadWidgets(const std::filesystem::path& fileName);
    Widget* GetWidget(const std::string& widgetName);  // Returns clone
    Widget* GetWidget(int index);
    Widget* GetWidget(int index, bool clone);

protected:
    std::vector<Widget*> _widgets;
    std::vector<TGA*> _sourceImages;
};
```

#### 9.5.1 XML Format

```xml
<Widgets>
    <Widget>
        <Name>Widget Name</Name>
        <Graphic>path/to/image.tga</Graphic>
        <Index>0</Index>  <!-- Optional -->
    </Widget>
</Widgets>
```

#### 9.5.2 Widget Class

```cpp
class Widget {
public:
    Widget(const std::string &name, TGA *tga);
    
    void Render(Screen *screen, int x, int y, Rect *clip);
    void Render(Screen *screen, int x, int y);
    void Render(Screen *screen, int x, int y, int w, int h);
    void Render(Screen *screen, int x, int y, Color *transparentColor);
    
    const std::string& GetName() const;
    int GetWidth();
    int GetHeight();
    TGA* GetImage();
    Widget* Clone();
};
```

### 9.6 Cursor System

**Location**: `src/application/CursorInterface.h`

```cpp
class CursorInterface {
public:
    enum class CursorType {
        // Markers (7 colors)
        MarkBlue = 0, MarkPurple, MarkRed, MarkYellow,
        MarkOrange, MarkBrown, MarkGreen, MarkGrey,
        
        // Crosshairs (filled = over target)
        CrosshairsBlack, CrosshairsRed, CrosshairsYellow, CrosshairsGreen,
        
        // Empty crosshairs (no target)
        CrosshairsEmptyBlack, CrosshairsEmptyRed,
        CrosshairsEmptyYellow, CrosshairsEmptyGreen,
        
        Regular,           // System default
        NumCursorTypes
    };
    
    virtual void ShowCursor(bool bShow, CursorType type) = 0;
};
```

#### 9.6.1 Cursor Files

| Type | File |
|------|------|
| MarkBlue | UI/Cursors/Mark Blue.tga |
| CrosshairsBlack | UI/Cursors/Crosshairs Black.tga |
| CrosshairsEmptyRed | UI/Cursors/Empty Crosshairs Red.tga |
| ... | ... |

### 9.7 ColorModifierManager

**Location**: `src/graphics/ColorModifierManager.h`

```cpp
struct ColorModifiers {
    std::string Name;
    Color Body;
    Color Legs;
    Color Head;
    Color Belt;
    Color Boots;
    Color Weapon;
};

// Modern C++: Using std::vector instead of fixed-size C-style array
extern std::vector<ColorModifiers> g_ColorModifiers;
```

**XML Format** (ColorModifiers.xml):
```xml
<ColorModifiers>
    <ColorModifier>
        <Name>American Infantry</Name>
        <Body><Red>-10</Red><Green>-6</Green><Blue>-15</Blue></Body>
        <Legs><Red>-6</Red><Green>-4</Green><Blue>-5</Blue></Legs>
        <Head><Red>-1</Red><Green>-3</Green><Blue>-3</Blue></Head>
        <Belt><Red>0</Red><Green>0</Green><Blue>0</Blue></Belt>
        <Boots><Red>0</Red><Green>0</Green><Blue>0</Blue></Boots>
        <Weapon><Red>-8</Red><Green>-8</Green><Blue>-8</Blue></Weapon>
    </ColorModifier>
</ColorModifiers>
```

Values are multiplied by 8 (range: -32 to +31 * 8 = -248 to +248).

---

*[Continue to Section 10: Build System and Dependencies]*
## 10. Build System and Dependencies

### 10.1 Dependencies

#### 10.1.1 Required Libraries

| Library | Version | Purpose | Installation |
|---------|---------|---------|--------------|
| SDL2 | 2.0+ | Windowing, events | `libsdl2-dev` |
| SDL2_mixer | 2.0+ | Audio playback | `libsdl2-mixer-dev` |
| SDL2_ttf | 2.0+ | TrueType fonts | `libsdl2-ttf-dev` |
| tinyxml2 | 11.0.0 | XML parsing | Bundled in src/misc/ |

#### 10.1.2 Ubuntu/Debian Installation

```bash
sudo apt-get install -y build-essential
sudo apt-get install -y libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev
```

#### 10.1.3 Build Tools

- **Compiler**: GCC with C++17 support (g++ 7+)
- **Build System**: GNU Make
- **Debugger**: GDB (optional)

### 10.2 Makefile

**Location**: `Makefile`

#### 10.2.1 Targets

```makefile
# Default: Release build
make                # Builds ./opencombat

# Debug build
make debug          # Builds ./opencombat-debug with -g flag

# Clean
make clean          # Removes all build artifacts

# Check dependencies
make check-deps     # Verifies SDL2 libraries installed
```

#### 10.2.2 Build Configuration

```makefile
# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
CXXFLAGS_DEBUG = -std=c++17 -Wall -Wextra -g -O0 -DDEBUG

# SDL2 flags
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs) -lSDL2_mixer -lSDL2_ttf

# Include paths
INCLUDES = -Isrc -Isrc/misc

# Source files
SOURCES = $(wildcard src/*.cpp src/**/*.cpp src/**/**/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
```

#### 10.2.3 Build Rules

```makefile
# Release build
opencombat: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(SDL_LIBS)

# Debug build
opencombat-debug: CXXFLAGS = $(CXXFLAGS_DEBUG)
opencombat-debug: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(SDL_LIBS)

# Object compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $(INCLUDES) -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) opencombat opencombat-debug
```

### 10.3 Project Structure

```
OpenCombat-SDL/
├── src/                       # Source code
│   ├── main.cpp              # Entry point
│   ├── main.h                # Main header
│   ├── ai/                   # A* pathfinding
│   │   ├── AStar.cpp
│   │   ├── AStar.h
│   │   ├── Path.cpp
│   │   └── Path.h
│   ├── application/          # Application layer
│   │   ├── CombatModule.cpp
│   │   ├── CombatModule.h
│   │   ├── CursorInterface.h
│   │   ├── GameApplication.cpp
│   │   ├── GameApplication.h
│   │   ├── Globals.h
│   │   ├── Module.h
│   │   ├── Modules.h
│   │   └── SoundInterface.h
│   ├── graphics/             # Rendering
│   │   ├── Animation.cpp
│   │   ├── Animation.h
│   │   ├── CombatContextMenu.cpp
│   │   ├── CombatContextMenu.h
│   │   ├── ColorModifierManager.cpp
│   │   ├── ColorModifierManager.h
│   │   ├── Effect.cpp
│   │   ├── Effect.h
│   │   ├── EffectManager.cpp
│   │   ├── EffectManager.h
│   │   ├── FontManager.cpp
│   │   ├── FontManager.h
│   │   ├── Frame.cpp
│   │   ├── Frame.h
│   │   ├── Mark.cpp
│   │   ├── Mark.h
│   │   ├── MaskFrame.cpp
│   │   ├── MaskFrame.h
│   │   ├── Screen.cpp
│   │   ├── Screen.h
│   │   ├── SoldierAnimationManager.cpp
│   │   ├── SoldierAnimationManager.h
│   │   ├── WidgetManager.cpp
│   │   └── WidgetManager.h
│   ├── misc/                 # Utilities
│   │   ├── Array.h
│   │   ├── Color.cpp
│   │   ├── Color.h
│   │   ├── GameConstants.h
│   │   ├── StatusCallback.h
│   │   ├── Structs.h
│   │   ├── TGA.cpp
│   │   ├── TGA.h
│   │   ├── tinyxml2.cpp
│   │   └── tinyxml2.h
│   ├── objects/              # Game objects
│   │   ├── Formation.cpp
│   │   ├── Formation.h
│   │   ├── Object.cpp
│   │   ├── Object.h
│   │   ├── Soldier.cpp
│   │   ├── Soldier.h
│   │   ├── SoldierActionHandlers.cpp
│   │   ├── SoldierActionHandlers.h
│   │   ├── SoldierManager.cpp
│   │   ├── SoldierManager.h
│   │   ├── Squad.cpp
│   │   ├── Squad.h
│   │   ├── SquadManager.cpp
│   │   ├── SquadManager.h
│   │   ├── Status.h
│   │   ├── Target.h
│   │   ├── Vehicle.cpp
│   │   ├── Vehicle.h
│   │   ├── VehicleManager.cpp
│   │   ├── VehicleManager.h
│   │   ├── Weapon.cpp
│   │   ├── Weapon.h
│   │   ├── WeaponManager.cpp
│   │   └── WeaponManager.h
│   ├── orders/               # Orders
│   │   ├── AmbushOrder.cpp
│   │   ├── AmbushOrder.h
│   │   ├── DefendOrder.cpp
│   │   ├── DefendOrder.h
│   │   ├── FireOrder.cpp
│   │   ├── FireOrder.h
│   │   ├── MoveOrder.cpp
│   │   ├── MoveOrder.h
│   │   ├── Order.cpp
│   │   ├── Order.h
│   │   ├── Orders.h
│   │   ├── PauseOrder.cpp
│   │   ├── PauseOrder.h
│   │   ├── StopOrder.cpp
│   │   └── StopOrder.h
│   ├── sound/                # Audio
│   │   ├── Sound.cpp
│   │   ├── Sound.h
│   │   ├── SoundManager.cpp
│   │   └── SoundManager.h
│   ├── states/               # State machine
│   │   ├── Action.cpp
│   │   ├── Action.h
│   │   ├── ActionQueue.h
│   │   ├── ObjectActions.cpp
│   │   ├── ObjectActions.h
│   │   ├── ObjectStates.h
│   │   ├── SoldierActionLoader.cpp
│   │   ├── SoldierActionLoader.h
│   │   ├── SoldierStateLoader.cpp
│   │   ├── SoldierStateLoader.h
│   │   ├── SoldierStateTransitions.h
│   │   ├── SoldierStateTransitionLoader.cpp
│   │   ├── SoldierStateTransitionLoader.h
│   │   ├── State.cpp
│   │   └── State.h
│   └── world/                # World systems
│       ├── Building.cpp
│       ├── Building.h
│       ├── BuildingManager.cpp
│       ├── BuildingManager.h
│       ├── Element.cpp
│       ├── Element.h
│       ├── ElementManager.cpp
│       ├── ElementManager.h
│       ├── LegacyMapLoader.cpp
│       ├── LegacyMapLoader.h
│       ├── LineOfSight.cpp
│       ├── LineOfSight.h
│       ├── Map.cpp
│       ├── Map.h
│       ├── MapManager.cpp
│       ├── MapManager.h
│       ├── MiniMap.cpp
│       ├── MiniMap.h
│       ├── Nationality.cpp
│       ├── Nationality.h
│       ├── VictoryLocation.cpp
│       ├── VictoryLocation.h
│       ├── World.cpp
│       └── World.h
├── config/                    # Configuration files (24 XML/TXT files)
├── graphics/                  # Visual assets (~8,000 TGA files)
├── maps/                      # Map data
│   └── Acqueville/
│       ├── Acqueville.xml
│       ├── Acqueville.txt
│       ├── Acqueville.buildings.xml
│       ├── *.tga
│       └── building_graphics/
├── sounds/                    # Audio assets
│   ├── Effects/
│   └── English Voices/
├── Makefile                   # Build system
├── README.md                  # Project readme
└── LICENSE                    # License file
```

### 10.4 Compilation Units

Total: **138 source files** (~21,420 lines of code)

#### 10.4.1 By Directory

| Directory | Files | Purpose |
|-----------|-------|---------|
| src/ | 2 | Entry point |
| src/ai/ | 4 | Pathfinding |
| src/application/ | 9 | App layer |
| src/graphics/ | 24 | Rendering |
| src/misc/ | 9 | Utilities |
| src/objects/ | 18 | Game objects |
| src/orders/ | 14 | Orders |
| src/sound/ | 4 | Audio |
| src/states/ | 14 | State machine |
| src/world/ | 20 | World systems |

#### 10.4.2 Key Compilation Dependencies

```
main.cpp
  └── GameApplication.cpp
       └── CombatModule.cpp
            ├── World.cpp
            │   ├── Map.cpp
            │   ├── BuildingManager.cpp
            │   └── LineOfSight.cpp
            ├── SoldierManager.cpp
            │   └── Soldier.cpp
            │       └── SoldierActionHandlers.cpp
            ├── SquadManager.cpp
            │   └── Squad.cpp
            └── VehicleManager.cpp
                └── Vehicle.cpp
```

### 10.5 Runtime Dependencies

#### 10.5.1 Required Data Files

```
./config/          # Must exist, 24 configuration files
./graphics/        # Must exist, ~8,000 TGA files
./maps/            # Must exist, at least one map
./sounds/          # Optional but recommended
```

#### 10.5.2 Dynamic Loading

- **TGA files**: Loaded on demand during gameplay
- **XML configs**: Loaded at initialization
- **Sounds**: Loaded when first played
- **Maps**: Loaded when selected

### 10.6 Platform Support

#### 10.6.1 Supported Platforms

- **Linux**: Primary development platform (Ubuntu/Debian tested)
- **macOS**: Should work with SDL2 installed via Homebrew
- **Windows**: Requires MinGW or Visual Studio port

#### 10.6.2 Known Issues

- **Case sensitivity**: Linux filesystem is case-sensitive
  - Use `config/` not `Config/`
  - Use `graphics/` not `Graphics/`
  
- **Path separators**: Code uses forward slashes `/`
  - Portable across all platforms

### 10.7 Debugging

#### 10.7.1 Debug Build

```bash
make debug
./opencombat-debug
```

Includes:
- `-g` flag for debug symbols
- `-O0` for no optimization
- `DEBUG` preprocessor define

#### 10.7.2 Self-Tests

```cpp
// In main.cpp (line 59)
Screen::SelfTest();       // Test blitting
```

Run automatically at startup. Note: `ActionQueue::SelfTest()` exists but is not currently called.

#### 10.7.3 Debug Flags (Globals.h)

```cpp
struct WorldGlobals {
    bool bRenderElevation;           // Show elevation numbers
    bool bRenderElements;            // Show element indices
    bool bRenderStats;               // Show performance stats
    bool bWeaponFan;                 // Show weapon range fans
    bool bRenderPaths;               // Show AI paths
    bool bRenderHelpText;            // Show help text
    bool bRenderBuildingOutlines;    // Show building boundaries
    bool bRenderBuildingInteriors;   // Force interior rendering
};
```

Toggle with F2-F9 keys.

### 10.8 Performance Considerations

#### 10.8.1 Optimizations

- **Software rendering**: No GPU dependency
- **Dirty rectangles**: Only redraw changed regions
- **Object culling**: Only render visible objects
- **Tile-based culling**: Skip off-screen tiles

#### 10.8.2 Memory Usage

- **TGA images**: Loaded as needed, cached
- **Animations**: Cloned from templates
- **Pathfinding**: Pre-allocated node pool (60,000 nodes)
- **Sounds**: Loaded once, reused

#### 10.8.3 Target Specifications

- **Resolution**: 800x600 minimum, up to map size
- **Framerate**: 30-60 FPS target
- **Simulation**: 50ms timestep (20Hz)

### 10.9 Distribution

#### 10.9.1 Release Build

```bash
make clean
make
strip opencombat  # Remove symbols for smaller binary
```

#### 10.9.2 Required Files for Distribution

```
opencombat              # Binary executable
config/                 # Configuration (24 files)
graphics/               # Assets (~8,000 files)
maps/                   # Maps (Acqueville/)
sounds/                 # Audio (optional but recommended)
README.md
LICENSE
```

#### 10.9.3 Installation

No installation required - runs from any directory:

```bash
./opencombat
```

Ensure the working directory contains `config/`, `graphics/`, `maps/` subdirectories.

---

## Appendix A: Glossary

- **Action**: Low-level executable unit (WalkTo, ProneFire, etc.)
- **Element**: Terrain type (grass, wall, water, etc.)
- **Formation**: Squad arrangement (Column, File, Line)
- **LOS**: Line of Sight
- **MegaTile**: 12x12 tile region (120x120 pixels)
- **Order**: High-level command (Move, Fire, Ambush)
- **Path**: Linked list of tile waypoints
- **Squad**: Group of soldiers/vehicles
- **State**: Behavioral condition (Standing, Prone, Firing)
- **TGA**: Targa image format
- **Tile**: 10x10 pixel map unit

## Appendix B: File Extensions

| Extension | Purpose |
|-----------|---------|
| .cpp | C++ source |
| .h | C++ header |
| .tga | Targa image |
| .xml | XML configuration |
| .txt | Text data |
| .wav | Audio (PCM) |
| .ttf | TrueType font |
| .md | Markdown documentation |

## Appendix C: Coordinate Quick Reference

| System | Units | Conversion |
|--------|-------|------------|
| Screen | pixels | world - origin |
| World | pixels | tile * 10 |
| Tile | 10px | world / 10 |
| MegaTile | 120px | tile / 12 |

## Appendix D: Timing Quick Reference

| Value | Description |
|-------|-------------|
| 50ms | Simulation timestep |
| 33ms | Effect frame duration (~30fps) |
| 150ms | Running animation frame |
| 200ms | Standard animation frame |
| 100ms | Scroll initial delay |
| 720px/s | Scroll speed |

---

**End of Design Document**

This document provides comprehensive specifications for recreating the OpenCombat SDL game engine and assets. All technical details, file formats, algorithms, and data schemas are documented to enable full reconstruction of the project.

For questions or issues, refer to the source code at `/home/juk/git/OpenCombat-SDL/`.

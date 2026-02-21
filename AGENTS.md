# AGENTS.md - OpenCombat SDL Coding Guidelines

## Project Overview

OpenCombat SDL is a C++17 tactical wargame being ported from DirectX/Windows to SDL2/cross-platform.
- **Language**: C++17
- **Lines**: ~21,500 lines across 139 source files
- **Status**: Active SDL2 port complete, testing phase

---

## Build Commands

### Setup
```bash
# Ubuntu/Debian
sudo apt-get install -y build-essential pkg-config libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev libtinyxml2-dev

# Check dependencies
make check-deps
```

### Build
```bash
# Release build (parallel recommended)
make -j$(nproc)

# Debug build
make debug -j$(nproc)

# Clean
make clean
make distclean  # Full cleanup including backups
```

### Run
```bash
./opencombat          # Release
./opencombat-debug    # Debug
```

---

## Test Commands

### Run Individual Tests
```bash
./opencombat --test-screen        # Test screen/blitting only
./opencombat --test-actionqueue   # Test action queue only
./opencombat --test-all           # Run all tests
```

Tests run and then exit without starting the game.

### Available Self-Tests
- `Screen::SelfTest()` - src/graphics/Screen.cpp:812
- `ActionQueue::SelfTest()` - src/states/ActionQueue.h:88

---

## Code Style

### Naming Conventions
| Type | Convention | Example |
|------|------------|---------|
| Classes | PascalCase | `GameApplication`, `Screen` |
| Methods | PascalCase | `Initialize()`, `Render()` |
| Members | underscore prefix | `_width`, `_device` |
| Locals | camelCase | `localVar`, `tempValue` |
| Constants | UPPER_SNAKE_CASE | `MAX_WEAPONS_PER_SOLDIER` |
| Files | PascalCase | `Screen.cpp`, `GameApplication.h` |

### Formatting
- **Indentation**: Tabs
- **Braces**: Allman style (opening brace on new line)
- **Line length**: ~100 characters
- **Comments**: `//` single line, `/* */` multi-line

### Example
```cpp
class MyClass
{
public:
    MyClass(void);
    virtual ~MyClass(void);
    void Initialize(void);
    
protected:
    int _value;
};

void
MyClass::Initialize(void)
{
    _value = 0;
}
```

---

## Import/Include Guidelines

### Include Order
```cpp
// 1. Header for this .cpp
#include "./GameApplication.h"

// 2. System headers
#include <stdio.h>
#include <vector>
#include <string>

// 3. Third-party (SDL2)
#include <SDL2/SDL.h>

// 4. Project headers (forward slashes)
#include "graphics/Screen.h"
#include "misc/Structs.h"
```

### Header Guards
Use `#pragma once` (not #ifdef guards)

---

## Types & Conventions

### Memory Management
- Use standard containers: `std::vector<T>`, `std::array<T, N>`, `std::string`
- Use `std::unique_ptr<T>` for owned heap objects
- Raw pointers for non-owning references (legacy compatibility)
- Use `calloc`/`free` for C-style arrays only when necessary

### Error Handling
```cpp
// Use assert for debug checks
assert(device != NULL);

// Return false on failure
if (!Initialize()) return false;

// Use Error.h macros for logging
LOG_INFO("Message");
LOG_ERROR("Error message");
```

### Booleans
- Use `bool` (not BOOL)
- Boolean methods: prefix with "Is" (e.g., `IsDead()`)

---

## SDL2 Port Notes

### Completed
- Graphics: Direct3D → SDL2 (Screen.cpp, FontManager.cpp)
- Input: Win32 messages → SDL2 events
- Audio: DirectSound → SDL2_mixer
- XML: MSXML4 → tinyxml2 (included in src/misc/)

### Path Handling
```cpp
// Use forward slashes (portable)
snprintf(path, 256, "%s/config", dir);
```

### Case Sensitivity
Linux filesystem is case-sensitive:
- `config/` not `Config/`
- `graphics/` not `Graphics/`

---

## Key Files

| File | Purpose |
|------|---------|
| src/main.cpp | Entry point |
| src/graphics/Screen.cpp | Rendering/blitting |
| src/graphics/FontManager.cpp | Text rendering |
| src/application/GameApplication.cpp | Game state management |
| src/world/World.cpp | Game world simulation |
| src/misc/tinyxml2.cpp | XML parsing |
| src/misc/Structs.h | Common structs (Point, Rect, Region) |

---

## Agent Notes

1. **Preserve existing style** - Match surrounding code conventions
2. **Minimize changes** - Focus only on the specific task
3. **Run SelfTest()** after modifications to graphics/states
4. **Build frequently** - Run `make` after each change
5. **Forward slashes** - Use `/` in all paths (not `\`)
6. **Use STL containers** - Prefer `std::vector`, `std::array` over raw arrays
7. **Do not modify tinyxml2** - `src/misc/tinyxml2.cpp` and `src/misc/tinyxml2.h` are third-party library files
8. **C-style conversions** - Currently using `atoi`, `atof` for string conversions; plan to migrate to C++ exception-based versions later

---

## Critical Design Patterns

See [docs/CRITICAL_DESIGN_PATTERNS.md](docs/CRITICAL_DESIGN_PATTERNS.md) for detailed information on common pitfalls including:
- Object ownership and order reference counting
- Filename parsing conventions

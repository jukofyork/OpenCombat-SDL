# AGENTS.md - OpenCombat SDL Coding Guidelines

## Project Overview

OpenCombat SDL is a C++ tactical wargame being ported from DirectX/Windows to SDL2/cross-platform.
- **Language**: C++11
- **Lines**: ~14,000 lines across 158 source files
- **Status**: Active SDL2 port (graphics/input complete, audio/XML pending)

---

## Build Commands

### Setup
```bash
# Ubuntu/Debian
sudo apt-get install -y build-essential libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev

# Check dependencies
make check-deps
```

### Build
```bash
# Release build
make

# Debug build
make debug

# Clean
make clean
```

### Run
```bash
./opencombat          # Release
./opencombat-debug    # Debug
```

---

## Test Commands

### Run Self-Tests
Self-tests run automatically at startup:
```bash
./opencombat
```

### Run Single Test
Add to `src/main.cpp` and rebuild:
```cpp
ActionQueue::SelfTest();  // Example single test
```

### Available Self-Tests
- `Screen::SelfTest()` - src/graphics/Screen.cpp:702
- `ActionQueue::SelfTest()` - src/states/ActionQueue.h:92

---

## Code Style

### Naming Conventions
| Type | Convention | Example |
|------|------------|---------|
| Classes | PascalCase | `GameApplication`, `Screen` |
| Methods | PascalCase | `Initialize()`, `Render()` |
| Members | underscore prefix | `_width`, `_device` |
| Locals | camelCase | `localVar`, `tempValue` |
| Constants | UPPER_SNAKE_CASE | `MAX_WEAPONS` |
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

// 3. Third-party (SDL2)
#include <SDL2/SDL.h>

// 4. Project headers (use forward slashes)
#include "graphics/Screen.h"
#include "misc/Array.h"
```

### Header Guards
Use `#pragma once` (not #ifdef guards)

---

## Types & Conventions

### Memory Management
- Use raw pointers, manual `new`/`delete` (legacy codebase)
- Use `Array<T>` template for dynamic arrays
- Use `calloc`/`free` for C-style arrays

### Error Handling
```cpp
// Use assert for debug checks
assert(device != NULL);

// Return false on failure
if (!Initialize()) return false;
```

### Booleans
- Use `bool` (not BOOL)
- Boolean methods: prefix with "Is" (e.g., `IsDead()`)

---

## SDL2 Port Notes

### Completed
- Graphics: Direct3D → SDL2 (Screen.cpp, FontManager.cpp)
- Input: Win32 messages → SDL2 events
- Audio: DirectSound → SDL2_mixer (Phase 3)
- XML: MSXML4 → tinyxml2 (Phase 4)

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
| src/misc/Array.h | Dynamic array template |

---

## Image Asset Handling

### TGA File Reading
This project uses TGA image files extensively (e.g., `graphics/Vehicles/panzer_IVG_wreck.11.21.tga`). These cannot be read directly but can be viewed after converting to PNG:

```bash
convert graphics/Vehicles/panzer_IVG_wreck.11.21.tga /tmp/view.png
```

The `convert` command from ImageMagick converts TGA to PNG format for viewing.

---

## Agent Notes

1. **Preserve existing style** - Match surrounding code conventions
2. **Minimize changes** - Focus only on the specific task
3. **Run SelfTest()** after modifications to graphics/states
4. **Build frequently** - Run `make` after each change
5. **No smart pointers** - Keep raw pointer style for consistency
6. **Forward slashes** - Use `/` in all paths (not `\`)

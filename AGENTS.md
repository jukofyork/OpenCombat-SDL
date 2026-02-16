# AGENTS.md - OpenCombat SDL Coding Guidelines

## Project Overview

OpenCombat SDL is a C++ tactical wargame being ported from DirectX/Windows to SDL2/cross-platform.
- **Language**: C++11
- **Lines**: ~14,000 lines across 158 source files
- **Status**: Active SDL2 port complete, testing phase

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

### Run All Self-Tests
Self-tests run automatically at startup:
```bash
./opencombat
```

### Run Single Test
Edit `src/main.cpp` to call specific test, then rebuild:
```cpp
// In main(), before app.Initialize():
Screen::SelfTest();       // Test screen/blitting
ActionQueue::SelfTest();  // Test action queue
```
Then run:
```bash
make && ./opencombat
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

// 4. Project headers (forward slashes)
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
- Audio: DirectSound → SDL2_mixer
- XML: MSXML4 → tinyxml2

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

### TGA Files
This project uses TGA images (e.g., `graphics/Vehicles/panzer_IVG_wreck.11.21.tga`). View them by converting to PNG:
```bash
convert graphics/Vehicles/panzer_IVG_wreck.11.21.tga /tmp/view.png
```

---

## Agent Notes

1. **Preserve existing style** - Match surrounding code conventions
2. **Minimize changes** - Focus only on the specific task
3. **Run SelfTest()** after modifications to graphics/states
4. **Build frequently** - Run `make` after each change
5. **No smart pointers** - Keep raw pointer style for consistency
6. **Forward slashes** - Use `/` in all paths (not `\`)

---

## Important Design Patterns

### Object Ownership and Selection

**CRITICAL: Never add the same Order pointer to multiple objects**

The codebase uses reference counting for orders (`Order::IncrementRefCount()` / `Order::Release()`). Adding the same order to multiple objects causes double-free crashes when all objects process and release the order.

**The Infantry Firing Bug (Commit 996b9cb)**

This bug occurred when adding vehicle squads (tanks) to the game:

**What went wrong:**
1. Tank squad was added to both `_mobileObjects` (via `AddObject()`) AND team objects list
2. Selection code iterated both `_currentMap->SelectObjects()` AND `_mobileObjects` loop
3. When clicking near both infantry squad and tank, BOTH got selected
4. `IssueOrder()` added the SAME order pointer to both selected objects
5. Both objects processed the order and called `Release()` → **double-free crash**

**The fix:**
```cpp
// In World::Select() - only iterate mobile objects if map finds nothing
_currentMap->SelectObjects(x+_originX, y+_originY, &_selectedObjects);

if(_selectedObjects.empty()) {  // <-- GUARD: skip if already found
    for(size_t i = 0; i < _mobileObjects.size(); ++i) {
        if(_mobileObjects[i]->Select(x+_originX,y+_originY)) {
            _selectedObjects.push_back(_mobileObjects[i]);
            break;  // <-- Only select one object
        }
    }
}
```

**Lessons learned:**
- Always check if selection already found something before iterating alternatives
- Vehicle squads should only be in `_mobileObjects`, NOT in team objects list
- Team objects list (`g_Globals->World.Teams[...].Objects`) is for UI display only
- The original author intentionally kept tanks out of team objects (see `#if 0` block)
- When you see `#if 0` in original code, investigate WHY before enabling it

**The Gun Flash Filename Parsing Bug (Commit fd3039e)**

This bug caused muzzle flashes to appear out of soldiers' legs and facing completely wrong directions (e.g., northeast flash appearing southwest).

**What went wrong:**
Effect files are named with format: `imageNNN.x.y.tga` where x,y are origin/hotspot coordinates for positioning the muzzle flash.

The parsing code in `EffectManager.cpp` had a critical bug:
```cpp
// OLD (buggy):
size_t lastDot = fName.rfind('.');
std::string yStr = fName.substr(lastDot + 1);  // Gets "tga" - WRONG!
fName = fName.substr(0, lastDot);
size_t secondDot = fName.rfind('.');
std::string xStr = fName.substr(secondDot + 1);  // Gets "-3" - should be Y!
int x = atoi(xStr.c_str());  // x = -3 (wrong!)
int y = atoi(yStr.c_str());  // y = 0 from "tga" (wrong!)
```

For `image001.-15.-3.tga`:
- OLD: x=-3, y=0 (flashes at leg level, wrong horizontal position)
- CORRECT: x=-15, y=-3

**The fix:**
```cpp
// NEW (fixed):
size_t extDot = fName.rfind('.');
if (extDot != std::string::npos) {
    fName = fName.substr(0, extDot);  // Strip .tga first
    size_t yDot = fName.rfind('.');
    if (yDot != std::string::npos) {
        std::string yStr = fName.substr(yDot + 1);  // Y = "-3"
        fName = fName.substr(0, yDot);
        size_t xDot = fName.rfind('.');
        if (xDot != std::string::npos) {
            std::string xStr = fName.substr(xDot + 1);  // X = "-15"
            int x = atoi(xStr.c_str());
            int y = atoi(yStr.c_str());
            tga->SetOrigin(x,y);
        }
    }
}
```

**Lessons learned:**
- Always strip the file extension first when parsing structured filenames
- Test parsing logic with actual example filenames
- When debugging visual bugs, add printf() statements to verify what's being parsed
- Negative numbers in filenames can confuse string parsing - be explicit about dot positions

# Critical Design Patterns

This document describes critical design patterns and common pitfalls in the OpenCombat SDL codebase. These are lessons learned from actual bugs.

---

## Object Ownership - NEVER add the same Order pointer to multiple objects

The codebase uses reference counting for orders (`Order::IncrementRefCount()` / `Order::Release()`). Adding the same order to multiple objects causes double-free crashes when all objects process and release the order.

### The Infantry Firing Bug (Commit 996b9cb)

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

---

## Filename Parsing - Strip extension first

Effect files use format: `imageNNN.x.y.tga` where x,y are origin/hotspot coordinates for positioning the muzzle flash.

### The Gun Flash Filename Parsing Bug (Commit fd3039e)

This bug caused muzzle flashes to appear out of soldiers' legs and facing completely wrong directions (e.g., northeast flash appearing southwest).

**What went wrong:**
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

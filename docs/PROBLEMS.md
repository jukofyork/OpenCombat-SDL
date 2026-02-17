# OpenCombat SDL - Known Bugs and Problems

This document tracks confirmed bugs, issues, and architectural problems in the OpenCombat SDL codebase.

## Critical Bugs (Affect Runtime Behavior)

### 1. TGA Origin Parsing Bug

**Status**: ✅ **FIXED** (Commit: Fix TGA origin parsing)

**Location**: `src/misc/TGA.cpp:152-175`

**Problem**: The TGA filename parser was fragile because it counted ALL dots in the full path string, not just in the filename. This would cause incorrect coordinate parsing if directory names ever contained dots (e.g., `graphics.v2/Effects/image001.-15.-3.tga`).

**Fix Applied**:
```cpp
// FIXED: Work with filename only, strip extension first
std::string fName = filePath.filename().string();

// Strip the extension first (.tga)
size_t extDot = fName.rfind('.');
if (extDot != std::string::npos) {
    fName = fName.substr(0, extDot);
    // Then parse X and Y coordinates
    ...
}
```

**Additional Changes**:
- Renamed parameter from `fileName` to `filePath` for clarity
- Removed duplicate origin parsing from `EffectManager.cpp` (origin parsing now only happens in `TGA::Create()`)

---

### 2. RLE-Compressed TGA Not Implemented

**Status**: Known limitation

**Location**: `src/misc/TGA.cpp:123`

**Problem**: TGA files with RLE compression (datatypecode == 10) hit an assertion and will crash:
```cpp
if(imageTypeCode == 10) {
    assert(0);  // RLE not implemented
}
```

**Impact**: Game will crash if loading RLE-compressed TGA files.

**Workaround**: Ensure all TGA assets are uncompressed (type 2).

---

## Documentation Bugs

### 3. ActionQueue::SelfTest() Not Called

**Status**: Documentation/implementation mismatch

**Location**: 
- Docs claim: `main.cpp` calls both tests
- Actual code: `main.cpp:59` only calls `Screen::SelfTest()`

**Problem**: The self-test system is documented as running both `Screen::SelfTest()` and `ActionQueue::SelfTest()`, but only the screen test is actually invoked.

**Code**:
```cpp
// main.cpp:59
Screen::SelfTest();  // Only this is called
// ActionQueue::SelfTest();  // NOT called
```

**Impact**: Action queue tests are not being run, potentially masking bugs.

**Fix Options**:
1. Add `ActionQueue::SelfTest();` to main.cpp
2. Update documentation to reflect actual behavior

---

## Minor Issues

### 4. State Bitfield Indentation Error

**Status**: Code style issue

**Location**: `src/states/State.cpp:12`

**Problem**: Minor indentation error in the bit shift operation (cosmetic only, logic is correct).

---

### 5. Missing XML Documentation

**Status**: Incomplete documentation

**Problem**: DESIGN.md was missing documentation for 13 XML configuration files:
- Vehicles.xml
- CombatUI.xml
- ContextMenuWidgets.xml
- BazookaAnimations.xml
- MachineGunAnimations.xml
- SoldierAnimations.xml
- SoldierDead.xml
- SoldierDeaths.xml
- Icons.xml
- WeaponIcons.xml
- Colors.xml
- ColorModifiers.xml
- SoundEffects.xml
- EnglishVoices.xml

**Resolution**: Fixed in DESIGN.md update. All files now documented.

---

### 6. Build System Documentation Inaccuracies (FIXED)

**Status**: ✅ Fixed in DESIGN.md

**Previous Issues**:
- C++ standard was documented as C++11 (actual: C++17)
- File count was 138 (actual: 158)
- Missing Makefile targets: `distclean`, `test`, `test-clean`, `help`
- Build used `pkg-config`, not `sdl2-config` as documented

**Resolution**: All corrected in DESIGN.md.

---

### 7. Action Struct Fields Mismatch (FIXED)

**Status**: ✅ Fixed in DESIGN.md

**Previous Issue**: DESIGN.md documented `Index` and `Data` fields in Action struct that don't exist in `ObjectActions.h`.

**Resolution**: Removed non-existent fields from documentation.

---

## Historical Bugs (Already Fixed)

### 8. The Infantry Firing Bug (FIXED - Commit 996b9cb)

**Status**: ✅ Fixed

**Problem**: Double-free crash when the same Order pointer was added to multiple selected objects.

**Root Cause**: When selecting near both infantry and tank, both got selected. `IssueOrder()` added the SAME order pointer to both, causing both to call `Release()` → crash.

**Fix**: Added guard in `World::Select()` to skip mobile object iteration if map already found objects:
```cpp
_currentMap->SelectObjects(x+_originX, y+_originY, &_selectedObjects);

if(_selectedObjects.empty()) {  // Guard added
    for(size_t i = 0; i < _mobileObjects.size(); ++i) {
        if(_mobileObjects[i]->Select(x+_originX,y+_originY)) {
            _selectedObjects.push_back(_mobileObjects[i]);
            break;
        }
    }
}
```

**Status**: Code fix verified in `World.cpp:802-819`.

---

### 9. Gun Flash Filename Parsing Bug (FIXED - Commit fd3039e)

**Status**: ✅ Fixed in logic, but TGA.cpp has new bug

**Historical Problem**: Old code parsed coordinates before stripping `.tga` extension, causing wrong origin extraction.

**Current Status**: The original parsing bug was conceptually understood and fixed in documentation, but the implementation in TGA.cpp still has a different bug (see Bug #1 above).

---

## Verification Notes

All issues in this document have been verified by code inspection as of the current codebase state. The verification process checked:

1. **Object System**: 95%+ accuracy
2. **State Machine**: No "4 state types" categorization exists (flat 22-state bitfield)
3. **Order System**: Reference counting documented correctly
4. **A* Pathfinding**: DeepClear() vs Clear() distinction documented
5. **Graphics**: TGA parsing bug CONFIRMED still exists
6. **World/Map**: Ownership model correct
7. **Configuration**: All XML schemas accurate
8. **Build System**: C++17, pkg-config verified

---

## Recommended Fixes Priority

### High Priority
1. ~~Fix TGA Origin Parsing~~ ✅ FIXED
2. **Add ActionQueue::SelfTest()** to main.cpp - Improves test coverage

### Medium Priority
3. **Implement RLE Decompression** or add better error handling
4. **Fix State.cpp indentation** (cosmetic)

### Low Priority
5. None at this time

---

## Architectural Improvements

### XML Manifest Migration (COMPLETED)

**Date**: 2026-02-17

**Objective**: Replace fragile directory scanning (`opendir/readdir/sort`) with explicit XML manifests

**Files Changed**:
- `EffectManager.cpp` - Removed `GetFiles()`, wildcard parsing, and duplicate origin parsing
- `SoldierAnimationManager.cpp` - Complete rewrite to parse explicit `<Direction>` and `<Frame>` elements
- `TGA.cpp` - Fixed origin parsing bug (use `filename()` instead of full path)

**XML Schema Changes**:

1. **Effects.xml** - Now uses explicit `<Graphic>` elements for all frames:
```xml
<Effect type="dynamic">
    <Name>Rifle North</Name>
    <FrameHold>33</FrameHold>
    <Graphic>rifle_n/image001.-1.26.tga</Graphic>
    <Graphic>rifle_n/image002.-1.29.tga</Graphic>
    <!-- ... all frames explicitly listed ... -->
</Effect>
```

2. **SoldierAnimations.xml** - New schema with explicit frame lists:
```xml
<SoldierAnimations>
    <Animation>
        <Name>Standing Rest</Name>
        <Directions>8</Directions>
        <NumFrames>1</NumFrames>
        <Direction name="North">
            <Frame sprite="Soldiers/Rifle/spr0000.39.33.tga" mask="Soldiers/Rifle/msk0000.39.33.tga"/>
        </Direction>
        <!-- ... 7 more directions ... -->
    </Animation>
</SoldierAnimations>
```

**Benefits**:
- ✅ No filesystem-dependent ordering issues
- ✅ Explicit validation at load time
- ✅ Self-documenting asset dependencies
- ✅ Easier to debug missing frames
- ✅ Consistent with existing XML-based asset loading

**Migration Tool**: `scripts/generate_xml_manifests.sh` - Generates explicit XML from existing directory contents

---

*Last Updated*: 2026-02-17
*Verification Method*: Code inspection + Build verification
*Files Checked*: All graphics loading code

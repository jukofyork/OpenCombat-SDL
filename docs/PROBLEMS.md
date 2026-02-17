# OpenCombat SDL - Known Bugs and Problems

This document tracks confirmed bugs, issues, and architectural problems that still need to be addressed in the OpenCombat SDL codebase.

For detailed implementation plans, see:
- [Soldier Animation Migration Plan](SOLDIER_ANIMATION_MIGRATION_PLAN.md) - Migration from directory scanning to explicit XML manifests
- [Vehicle Combat Implementation Plans](VEHICLE_COMBAT_IMPLEMENTATION_PLANS.md) - Design options for implementing vehicle combat
- [Zoom Implementation Notes](ZOOM_IMPLEMENTATION_NOTES.md) - Analysis of attempted zoom feature and path forward

---

## Current Issues

### 1. RLE-Compressed TGA Not Implemented

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

### 2. Vehicle Combat Not Implemented

**Status**: Not implemented (was not in original 2005-2007 scope)

**Problem**: Vehicles can fire weapons and show effects, but cannot deal or receive damage. See VEHICLE_COMBAT_IMPLEMENTATION_PLANS.md for detailed analysis and implementation options.

---

### 3. Zoom Feature Not Implemented

**Status**: Attempted and reverted

**Problem**: Multiple approaches tried (full-screen zoom, split rendering, selective texture copying) all failed due to UI/world overlap and SDL2 limitations. See ZOOM_IMPLEMENTATION_NOTES.md for detailed analysis of what was tried and recommended approaches.

---

### 4. Soldier Animation Loading Uses Directory Scanning

**Status**: Legacy code - needs migration

**Location**: `src/graphics/SoldierAnimationManager.cpp:103-161`

**Problem**: Still uses `opendir/readdir` and `std::sort` to load animation frames. All other asset loading uses explicit XML manifests.

**Solution**: See SOLDIER_ANIMATION_MIGRATION_PLAN.md for detailed migration plan from directory scanning to explicit XML manifests.

---

*Last Updated*: 2026-02-17

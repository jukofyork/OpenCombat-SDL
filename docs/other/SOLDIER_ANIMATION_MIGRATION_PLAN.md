# SOLDIER_ANIMATION_MIGRATION_PLAN.md

## Executive Summary

This document details the architecture of the soldier animation system in OpenCombat-SDL and provides a comprehensive migration plan to move from directory-scanning-based loading to explicit XML manifests. This migration was attempted once and failed; this document exists to preserve the knowledge gained and provide a roadmap for future attempts.

**Current Status**: Only `SoldierAnimationManager.cpp` still uses directory scanning (`opendir/readdir`). The Effects system has been successfully migrated (commit e1897f4).

---

## 1. System Architecture Overview

### 1.1 The Five XML Configuration Files

The soldier animation system uses **5 separate XML files** that are loaded sequentially into a single `SoldierAnimationManager`:

| File | Directory | Frame Range | Total Files | Animation Count |
|------|-----------|-------------|-------------|-----------------|
| `SoldierAnimations.xml` | `Soldiers/Rifle` | spr0000-0759 | 1,520 (760×2) | 16 |
| `BazookaAnimations.xml` | `Soldiers/Bazooka` | spr1648-1975 | 656 (328×2) | 12 |
| `MachineGunAnimations.xml` | `Soldiers/MG` | spr1320-1647 | 656 (328×2) | 12 |
| `SoldierDeaths.xml` | `Soldiers/Dying` | spr2296-2535 | 480 (240×2) | 3 |
| `SoldierDead.xml` | `Soldiers/Dead 1` | spr2624-2631 | 16 (8×2) | 1 |

**Total**: 3,328 files (1,664 sprites + 1,664 masks) across 44 animations

### 1.2 File Naming Convention

All animation files follow this pattern:
- **Sprites**: `sprXXXX.YY.ZZ.tga`
- **Masks**: `mskXXXX.YY.ZZ.tga`

Where:
- `XXXX` = 4-digit sequential frame number (zero-padded)
- `YY.ZZ` = hotspot/origin coordinates (X.Y)
- All current assets use `39.33` as the hotspot

### 1.3 Animation Structure

Each animation consists of:
- **Name**: Human-readable identifier
- **Directions**: Always 8 (cardinal and ordinal directions)
- **NumFrames**: Frames per direction (varies by animation type)
- **FirstDirection**: Starting direction index (usually "North")
- **Time**: Frame duration in milliseconds
- **TransparentColor**: Usually 16777215 (white)

### 1.4 Direction Mapping

```cpp
enum Direction {
    South=0,
    SouthWest=1,
    West=2,
    NorthWest=3,
    North=4,
    NorthEast=5,
    East=6,
    SouthEast=7,
    NumDirections=8
};
```

The `FirstDirection` shifts the frame assignment:
- Formula: `((FirstDirectionIndex + k) % 8)` where k = 0..7
- Example: FirstDirection=North (4), so frames are assigned:
  - North (4), NorthEast (5), East (6), SouthEast (7), South (0), SouthWest (1), West (2), NorthWest (3)

---

## 2. Current Loading Mechanism

### 2.1 Directory Scanning Algorithm

Located in `src/graphics/SoldierAnimationManager.cpp` (lines 103-161):

```cpp
// 1. Parse XML attributes
std::string directory = root->Attribute("dir");  // e.g., "Soldiers/Rifle"
std::string image = root->Attribute("image");    // e.g., "spr*"
std::string mask = root->Attribute("mask");      // e.g., "msk*"

// 2. Open directory and scan
DIR* dir = opendir(searchDir.c_str());
while ((entry = readdir(dir)) != NULL) {
    // Skip . and ..
    // Check for .tga extension
    // Check if filename starts with "spr"
    if (entryName.substr(0, 3) == "spr") {
        files.push_back(filePath);
        
        // Derive mask filename by replacing "spr" with "msk"
        maskFile[0] = 'm'; maskFile[1] = 's'; maskFile[2] = 'k';
        masks.push_back(maskPath);
    }
}

// 3. Sort both arrays
std::sort(files.begin(), files.end());
std::sort(masks.begin(), masks.end());
```

### 2.2 Frame Assignment Algorithm

For each animation definition in XML:

```cpp
for each animation in dest:
    create new Animation object
    
    for j = 0 to NumFrames-1:           // Frames per direction
        for k = 0 to 7:                  // 8 directions
            file = files[numFiles]       // Take next file in sorted list
            mask = masks[numFiles]
            numFiles++
            
            // Parse hotspot from filename
            // Create MaskFrame with sprite + mask
            // Add to Animation at direction ((FirstDirection + k) % 8)
```

**Critical**: Files are consumed sequentially across ALL animations in the XML. The order of `<Animation>` elements in the XML determines which files get assigned to which animation.

---

## 3. Frame Inventory by Animation Type

### 3.1 Rifle Animations (SoldierAnimations.xml)

| Animation | Directions | Frames/Dir | Total Frames | File Range |
|-----------|------------|------------|--------------|------------|
| Standing Rest | 8 | 1 | 8 | 0000-0007 |
| Standing Firing | 8 | 1 | 8 | 0008-0015 |
| Kneeling Rest | 8 | 1 | 8 | 0016-0023 |
| Kneeling Firing | 8 | 1 | 8 | 0024-0031 |
| Prone Rest | 8 | 1 | 8 | 0032-0039 |
| Prone Firing | 8 | 1 | 8 | 0040-0047 |
| Standing Reloading | 8 | 5 | 40 | 0048-0087 |
| Kneeling Reloading | 8 | 5 | 40 | 0088-0127 |
| Prone Reloading | 8 | 5 | 40 | 0128-0167 |
| Standing Up | 8 | 8 | 64 | 0168-0231 |
| Grenade | 8 | 8 | 64 | 0232-0295 |
| Walking | 8 | 12 | 96 | 0296-0391 |
| Running | 8 | 8 | 64 | 0392-0455 |
| Walking Cover | 8 | 14 | 112 | 0456-0567 |
| Running Cover | 8 | 8 | 64 | 0568-0631 |
| Crawling | 8 | 16 | 128 | 0632-0759 |

**Total**: 760 frames (1,520 files with masks)

### 3.2 Bazooka Animations (BazookaAnimations.xml)

| Animation | Directions | Frames/Dir | Total Frames | File Range |
|-----------|------------|------------|--------------|------------|
| Bazooka Standing Rest | 8 | 1 | 8 | 1648-1655 |
| Bazooka Standing Firing | 8 | 1 | 8 | 1656-1663 |
| Bazooka Kneeling Rest | 8 | 1 | 8 | 1664-1671 |
| Bazooka Kneeling Firing | 8 | 1 | 8 | 1672-1679 |
| Bazooka Prone Rest | 8 | 1 | 8 | 1680-1687 |
| Bazooka Prone Firing | 8 | 1 | 8 | 1688-1695 |
| Bazooka Standing Up | 8 | 6 | 48 | 1696-1743 |
| Bazooka Walking | 8 | 6 | 48 | 1744-1791 |
| Bazooka Running | 8 | 4 | 32 | 1792-1823 |
| Bazooka Walking Cover | 8 | 7 | 56 | 1824-1879 |
| Bazooka Running Cover | 8 | 4 | 32 | 1880-1911 |
| Bazooka Crawling | 8 | 8 | 64 | 1912-1975 |

**Total**: 328 frames (656 files with masks)

### 3.3 Machine Gun Animations (MachineGunAnimations.xml)

Same structure as Bazooka, but different file numbers:
- **File Range**: 1320-1647
- **Total**: 328 frames (656 files with masks)

### 3.4 Death Animations (SoldierDeaths.xml)

| Animation | Directions | Frames/Dir | Total Frames | File Range |
|-----------|------------|------------|--------------|------------|
| Dying Blown Up | 8 | 10 | 80 | 2296-2375 |
| Dying Forward | 8 | 10 | 80 | 2376-2455 |
| Dying Backward | 8 | 10 | 80 | 2456-2535 |

**Total**: 240 frames (480 files with masks)

### 3.5 Dead Animations (SoldierDead.xml)

| Animation | Directions | Frames/Dir | Total Frames | File Range |
|-----------|------------|------------|--------------|------------|
| Dead | 8 | 1 | 8 | 2624-2631 |

**Total**: 8 frames (16 files with masks)

---

## 4. Why The Previous Migration Failed

Based on analysis and the AGENTS.md notes about the Gun Flash bug (commit fd3039e), the previous attempt likely failed due to:

### 4.1 Filename Numbering Assumptions

**Mistake**: Assuming all files start at 0
- Rifle does start at 0
- **MG starts at 1320**
- **Bazooka starts at 1648**
- **Dying starts at 2296**
- **Dead starts at 2624**

**Result**: Generated filenames like `spr0000` for Bazooka when actual files are `spr1648`

### 4.2 Wrong Coordinate Parsing

**Mistake**: Similar to the Gun Flash bug, not stripping extension before parsing
```cpp
// WRONG:
size_t lastDot = fName.rfind('.');
std::string yStr = fName.substr(lastDot + 1);  // Gets "tga" not the Y coord!

// CORRECT (from commit fd3039e):
size_t extDot = fName.rfind('.');
fName = fName.substr(0, extDot);  // Strip .tga FIRST
size_t yDot = fName.rfind('.');
std::string yStr = fName.substr(yDot + 1);  // Now gets Y coord
```

### 4.3 Missing File Verification

**Mistake**: Not verifying generated filenames against actual files
**Result**: XML referenced files that didn't exist → segfault on startup

### 4.4 spr/msk Pairing Errors

**Mistake**: Not ensuring every spr file has a corresponding msk file
**Result**: Animation frames without collision masks

### 4.5 Frame Count Mismatches

**Mistake**: XML said 12 frames for Walking, but generated wrong number
**Result**: Animation corruption or crashes

---

## 5. Migration Options

### Option A: Fully Explicit XML (Maximum Verifiability)

**Structure**: List every single file explicitly

```xml
<?xml version="1.0" encoding="utf-8"?>
<Animations baseDir="Soldiers/Rifle">
    <Animation name="Standing Rest" time="200" firstDir="North" transColor="16777215">
        <Direction name="North"     sprite="spr0000.39.33.tga" mask="msk0000.39.33.tga"/>
        <Direction name="NorthEast" sprite="spr0001.39.33.tga" mask="msk0001.39.33.tga"/>
        <Direction name="East"      sprite="spr0002.39.33.tga" mask="msk0002.39.33.tga"/>
        <!-- ... 5 more directions ... -->
    </Animation>
    <Animation name="Walking" time="200" firstDir="North" transColor="16777215">
        <Direction name="North">
            <Frame sprite="spr0296.39.33.tga" mask="msk0296.39.33.tga"/>
            <Frame sprite="spr0304.39.33.tga" mask="msk0304.39.33.tga"/>
            <!-- ... 10 more frames ... -->
        </Direction>
        <!-- ... 7 more directions ... -->
    </Animation>
</Animations>
```

**Pros**:
- Absolute clarity - every file is explicitly stated
- Easy to verify correctness
- No assumptions about file ordering
- Self-documenting

**Cons**:
- Extremely verbose (~2,600 lines for Rifle alone)
- Difficult to edit manually
- High risk of copy-paste errors
- XML files become very large

**Verdict**: Overkill for this use case. Better for smaller systems.

---

### Option B: Compact Range Notation (Maximum Compactness)

**Structure**: Specify file ranges with start/count

```xml
<?xml version="1.0" encoding="utf-8"?>
<Animations baseDir="Soldiers/Rifle">
    <Animation name="Standing Rest" time="200" firstDir="North" transColor="16777215">
        <FrameRange start="0" count="8"/>  <!-- 8 files: 0000-0007 -->
    </Animation>
    <Animation name="Standing Reloading" time="200" firstDir="North" transColor="16777215">
        <FrameRange start="48" count="40"/>  <!-- 40 files: 0048-0087 -->
    </Animation>
    <Animation name="Walking" time="200" firstDir="North" transColor="16777215">
        <FrameRange start="296" count="96"/>  <!-- 96 files: 0296-0391 -->
    </Animation>
</Animations>
```

**Pros**:
- Very compact
- Easy to read and understand
- Close to current mental model

**Cons**:
- Still relies on implicit file naming convention
- Hard to verify without running the game
- Frame count mismatches only caught at runtime
- Numbering errors (e.g., off-by-one) hard to spot

**Verdict**: Too implicit. We need more verifiability given the complexity.

---

### Option C: Hybrid with Verification (RECOMMENDED)

**Structure**: Compact notation with explicit verification data

```xml
<?xml version="1.0" encoding="utf-8"?>
<!-- 
  Soldier Rifle Animations Manifest
  Generated: 2026-02-17 from graphics/Soldiers/Rifle/
  Total frames: 760 (1520 files with masks)
  
  Verification: 
    Expected files: spr0000-0759.tga, msk0000-0759.tga
    Actual files found: 1520 ✓
    Missing: 0
-->
<Animations baseDir="Soldiers/Rifle">
    <!-- Standing Rest: 8 dirs × 1 frame = 8 files (0000-0007) -->
    <Animation name="Standing Rest" time="200" firstDir="North" transColor="16777215">
        <Sequence pattern="spr{0000..0007}.39.33.tga" maskPattern="msk{0000..0007}.39.33.tga"/>
    </Animation>
    
    <!-- Walking: 8 dirs × 12 frames = 96 files (0296-0391) -->
    <Animation name="Walking" time="200" firstDir="North" transColor="16777215">
        <Sequence pattern="spr{0296..0391}.39.33.tga" maskPattern="msk{0296..0391}.39.33.tga"/>
    </Animation>
</Animations>
```

**Pros**:
- Compact and readable
- Pattern notation is unambiguous
- Can be programmatically expanded and verified
- Generation tool can validate files exist
- Comments document the verification status

**Cons**:
- Requires generation tool (but we need one anyway)
- Slightly more complex parsing

**Verdict**: Best balance of readability, verifiability, and maintainability.

---

### Option D: Animation-Centric with Frame Lists

**Structure**: Group by animation, list all frames

```xml
<?xml version="1.0" encoding="utf-8"?>
<Animations baseDir="Soldiers/Rifle">
    <Animation name="Standing Rest" time="200" firstDir="North" transColor="16777215" 
               frames="8" files="spr0000-0007.tga"/>
    <Animation name="Standing Reloading" time="200" firstDir="North" transColor="16777215"
               frames="40" files="spr0048-0087.tga"/>
    <Animation name="Walking" time="200" firstDir="North" transColor="16777215"
               frames="96" files="spr0296-0391.tga"/>
</Animations>
```

**Pros**:
- Very compact
- Similar to current XML structure
- Easy to verify frame counts match

**Cons**:
- Too implicit - doesn't show the pattern
- Hard to debug if frame count doesn't match files
- Doesn't show direction breakdown

**Verdict**: Too terse. Doesn't provide enough debugging information.

---

## 6. Recommended Implementation Plan

### Phase 1: Create Generation Tool

**File**: `scripts/generate_soldier_xml.sh` (bash, like Effects script)

**Steps**:
1. Parse current XML files to extract animation definitions
2. Calculate expected file ranges for each animation
3. Scan actual directories to find real files
4. Verify spr/msk pairing for each file
5. Generate new XML with pattern notation
6. Output verification report

**Verification Report Example**:
```
SoldierAnimations.xml (Rifle):
  Expected files: 1520 (760 spr + 760 msk)
  Found files: 1520 ✓
  Animation coverage:
    - Standing Rest: 8 files ✓ (0000-0007)
    - Standing Reloading: 40 files ✓ (0048-0087)
    - Walking: 96 files ✓ (0296-0391)
    ...
  Missing files: 0
  Unpaired masks: 0
  
BazookaAnimations.xml:
  Expected files: 656
  Found files: 656 ✓
  ...
```

### Phase 2: New XML Schema

**Attributes per Animation**:
- `name`: Animation name (matches current)
- `time`: Frame duration
- `firstDir`: Starting direction (North/South/etc.)
- `transColor`: Transparent color
- `frameCount`: Total frames (for verification)

**Child Elements**:
- `<Sequence pattern="..." maskPattern="..."/>`: File range in bash brace expansion notation

**Pattern Syntax**:
- `spr{0000..0007}.39.33.tga` expands to: spr0000.39.33.tga, spr0001.39.33.tga, ... spr0007.39.33.tga
- `msk{0000..0007}.39.33.tga` expands similarly for masks

### Phase 3: Dual-Mode Loader

**Strategy**: Support both old and new formats temporarily

```cpp
void SoldierAnimationManager::LoadAnimations(const std::filesystem::path& fileName) {
    XMLDocument doc;
    doc.LoadFile(fileName.c_str());
    
    XMLElement* root = doc.FirstChildElement("Animations");
    if (!root) return;
    
    // Check for new format (has <Sequence> elements)
    if (root->FirstChildElement("Animation") && 
        root->FirstChildElement("Animation")->FirstChildElement("Sequence")) {
        LoadNewFormat(root);  // Explicit manifest
    } else {
        LoadOldFormat(root);  // Directory scanning
    }
}
```

**Benefits**:
- Can test new format without breaking old
- Easy to compare: load both, verify frame counts match
- Gradual migration: convert one XML file at a time

### Phase 4: Incremental Migration

**Order of conversion**:
1. `SoldierDead.xml` (simplest: 1 animation, 8 frames)
2. `SoldierDeaths.xml` (3 animations, 240 frames)
3. `BazookaAnimations.xml` (12 animations, 328 frames)
4. `MachineGunAnimations.xml` (12 animations, 328 frames)
5. `SoldierAnimations.xml` (16 animations, 760 frames - most complex)

**After each conversion**:
- Generate new XML
- Run game with `--validate-soldier-xml` flag
- Compare old vs new loading
- Visual test: watch animations in game

### Phase 5: Remove Old Code

Once all 5 files are migrated and tested:
- Remove `opendir/readdir` code
- Remove `std::sort` calls
- Remove old format loader
- Update documentation

---

## 7. Critical Implementation Details

### 7.1 File Ordering

The current code relies on `std::sort()` for alphabetical ordering. This works because:
- All filenames are zero-padded to 4 digits
- "spr0100" sorts after "spr0099" correctly
- No files should ever be unpadded (e.g., "spr100.tga")

**Verification**: Generation tool must check all files match `spr[0-9]{4}\.[0-9]+\.[0-9]+\.tga`

### 7.2 Hotspot Parsing

When implementing the new loader, use the corrected parsing logic from TGA.cpp:

```cpp
void ParseFilename(const std::string& filePath, int& outX, int& outY) {
    std::string fName = filePath.filename().string();
    
    // Strip extension first
    size_t extDot = fName.rfind('.');
    if (extDot == std::string::npos) return;
    fName = fName.substr(0, extDot);
    
    // Find Y coordinate
    size_t yDot = fName.rfind('.');
    if (yDot == std::string::npos) return;
    std::string yStr = fName.substr(yDot + 1);
    
    // Find X coordinate
    fName = fName.substr(0, yDot);
    size_t xDot = fName.rfind('.');
    if (xDot == std::string::npos) return;
    std::string xStr = fName.substr(xDot + 1);
    
    outX = atoi(xStr.c_str());
    outY = atoi(yStr.c_str());
}
```

### 7.3 spr/msk Pairing Verification

Every sprite MUST have a corresponding mask:

```cpp
bool VerifyPairing(const std::string& sprFile, const std::string& mskFile) {
    // Extract number from spr file
    std::string sprNum = ExtractNumber(sprFile);  // "spr0000.39.33.tga" -> "0000"
    
    // Extract number from msk file  
    std::string mskNum = ExtractNumber(mskFile);  // "msk0000.39.33.tga" -> "0000"
    
    return sprNum == mskNum;
}
```

### 7.4 Direction Assignment

Must preserve exact current logic:

```cpp
int firstDirIndex = DirectionFromString(animation->FirstDirection);
// DirectionFromString: "North"=4, "South"=0, etc.

for (int frame = 0; frame < numFrames; frame++) {
    for (int dir = 0; dir < 8; dir++) {
        int actualDir = (firstDirIndex + dir) % 8;
        // Assign file at index (currentFileIndex++) to direction actualDir
    }
}
```

### 7.5 Path Construction

Files in XML are relative to baseDir:

```cpp
std::filesystem::path baseDir = root->Attribute("baseDir");
std::string pattern = sequence->Attribute("pattern");

// Expand pattern: "spr{0000..0007}.39.33.tga"
// To: ["spr0000.39.33.tga", "spr0001.39.33.tga", ...]

for each expanded filename:
    std::filesystem::path fullPath = g_Globals->Application.GraphicsDirectory / baseDir / filename;
```

---

## 8. Testing Checklist

After migration, verify:

### 8.1 File Coverage
- [ ] All 3,328 files are referenced in new XML
- [ ] No duplicate file references
- [ ] No orphaned files (files not referenced)
- [ ] spr/msk pairing is 1:1 for all files

### 8.2 Animation Integrity
- [ ] Each animation has correct frame count
- [ ] Direction counts are all 8
- [ ] Frame order matches old loading exactly
- [ ] Hotspot coordinates parsed correctly

### 8.3 Visual Testing
- [ ] Standing animations look correct (all 8 directions)
- [ ] Walking animation plays smoothly
- [ ] Reloading animation shows all frames
- [ ] Death animations play to completion
- [ ] Dead poses display correctly
- [ ] Direction changes work (rotate soldier)

### 8.4 Edge Cases
- [ ] Bazooka animations (different file numbers)
- [ ] MG animations (different file numbers)
- [ ] Dead animation (only 1 frame per direction)
- [ ] Soldiers facing all 8 directions

---

## 9. Files to Modify

### 9.1 New Files
- `scripts/generate_soldier_xml.sh` - Generation script
- `SOLDIER_ANIMATION_MIGRATION_PLAN.md` - This document

### 9.2 Modified Files
- `config/SoldierAnimations.xml` - Rifle animations
- `config/BazookaAnimations.xml` - Bazooka animations
- `config/MachineGunAnimations.xml` - MG animations
- `config/SoldierDeaths.xml` - Death animations
- `config/SoldierDead.xml` - Dead poses
- `src/graphics/SoldierAnimationManager.cpp` - New loader
- `src/graphics/SoldierAnimationManager.h` - New method declarations

### 9.3 Eventually Removed (after full migration)
- `opendir/readdir` code in SoldierAnimationManager.cpp
- `std::sort` calls
- Old XML attributes: `dir`, `image`, `mask`

---

## 10. References

### 10.1 Related Code Locations
- `src/graphics/SoldierAnimationManager.cpp:103-161` - Current directory scanning
- `src/graphics/EffectManager.cpp` - Successfully migrated Effects system
- `src/misc/TGA.cpp:45-175` - Correct filename parsing (fixed in commit fd3039e)

### 10.2 Related Documentation
- `AGENTS.md` - Contains notes on the Gun Flash bug and double-free issues
- `PROBLEMS.md` - Bug tracking and resolution log
- `config/Effects.xml` - Example of explicit manifest (post-migration)

### 10.3 Git History
- Commit `e1897f4` - Successful Effects migration
- Commit `fd3039e` - Gun Flash filename parsing fix

---

## 11. Conclusion

The soldier animation system migration is significantly more complex than the Effects migration due to:

1. **Scale**: 3,328 files vs 365 files
2. **Complexity**: 5 XML files with interdependent loading
3. **spr/msk pairing**: Every frame needs collision mask
4. **Direction logic**: Non-trivial 8-direction assignment
5. **Non-zero indexing**: Files don't start at 0 for non-Rifle types

The **recommended approach (Option C)** provides the best balance:
- Compact enough to be maintainable
- Verifiable through generation tool
- Clear pattern syntax
- Can be implemented incrementally

**Next Steps**:
1. Create `scripts/generate_soldier_xml.sh`
2. Start with `SoldierDead.xml` (simplest test case)
3. Implement dual-mode loader
4. Test incrementally
5. Remove old code after full validation

**Risk Mitigation**:
- Keep both loaders during migration
- Generate verification reports
- Test each XML file independently
- Visual verification in-game is mandatory

---

*Document Version: 1.0*
*Created: 2026-02-17*
*Status: Planning complete, ready for implementation*

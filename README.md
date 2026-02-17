# OpenCombat SDL2 Port

## Overview

OpenCombat is an open-source clone of the classic Close Combat tactical wargame series. This version is being ported from DirectX/Windows to SDL2/Linux.

**Original**: Windows/DirectX (2005)  
**Target**: Cross-platform (Linux, Windows, macOS) with SDL2

---

## Project Structure

```
opencombat-sdl/
├── src/                    # Source code (~14,000 lines)
│   ├── ai/                # A* pathfinding and AI
│   ├── application/       # Main application and game modules
│   ├── graphics/          # Rendering, animations, UI (SDL2)
│   ├── misc/              # Utilities (TGA loader, Array, tinyxml2)
│   ├── objects/           # Soldiers, squads, vehicles, weapons
│   ├── orders/            # Unit orders (move, fire, defend)
│   ├── sound/             # Audio management (SDL2_mixer)
│   ├── states/            # State machines for units
│   └── world/             # Maps, buildings, terrain, line-of-sight
├── config/                # XML configuration files
├── graphics/              # Game assets (TGA images, ~85MB)
├── maps/                  # Map files (~25MB)
├── sounds/                # Audio files (~9.5MB)
└── Makefile               # Build configuration
```

---

## Current Status

### Completed
- [x] Graphics: Direct3D → SDL2 (Screen.cpp, FontManager.cpp)
- [x] Input: Win32 messages → SDL2 events
- [x] Audio: DirectSound → SDL2_mixer (Phase 3)
- [x] XML: MSXML4 → tinyxml2 (Phase 4)
- [x] Custom cursor system with team-colored markers
- [x] Font rendering and victory location text
- [x] Keyboard controls (arrow keys, F-keys, context menu)
- [x] Minimap with team/nationality support
- [x] Core game loop and rendering functional
- [x] Self-tests for Screen and ActionQueue

### Pending
- [ ] Testing and optimization (Phase 5)

---

## Dependencies

### Required
- SDL2 (core)
- SDL2_mixer (audio)
- SDL2_ttf (fonts)
- tinyxml2 (XML parsing - bundled in src/misc/)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential
sudo apt-get install -y libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev
```

**Fedora:**
```bash
sudo dnf install -y gcc-c++ SDL2-devel SDL2_mixer-devel SDL2_ttf-devel
```

**macOS (Homebrew):**
```bash
brew install sdl2 sdl2_mixer sdl2_ttf
```

---

## Building

### Using Make

```bash
# Check dependencies
make check-deps

# Build release version
make

# Build debug version
make debug

# Clean build files
make clean
```

---

## Running

```bash
./opencombat          # Release build
./opencombat-debug    # Debug build
```

The game expects to find `config/`, `graphics/`, `maps/`, and `sounds/` directories in the working directory.

---

## Controls

### Mouse Controls

| Action | Control | Description |
|--------|---------|-------------|
| **Select Unit** | Left Click | Select a soldier or vehicle |
| **Context Menu** | Right Click | Open action menu for selected unit |
| **Pan View** | Middle Click + Drag | Click and drag to pan the camera |
| **Minimap Click** | Left Click | Jump to location on minimap |

### Context Menu Actions (Right-Click)

When a unit is selected, right-click to open the context menu:

- **Move** - Order unit to move to location (blue cursor)
- **Move Fast** - Order unit to run (purple cursor)
- **Fire** - Attack target (red cursor)
- **Sneak** - Move stealthily (yellow cursor)
- **Smoke** - Deploy smoke grenade (grey cursor)
- **Defend** - Set up defensive position
- **Ambush** - Set up ambush facing direction

### Keyboard Controls

#### View Controls
| Key | Action |
|-----|--------|
| **Arrow Keys** | Scroll/Pan the view (hold for continuous scroll) |
| **K** | Kill selected unit(s) (debug/cheat) |
| **F** | Cycle through formations for selected unit(s) |

#### Display Toggles (F-Keys)
| Key | Action |
|-----|--------|
| **F1** | Toggle help display (shows controls) |
| **F2** | Toggle FPS/stats display |
| **F3** | Toggle path rendering (show unit paths) |
| **F4** | Toggle weapon fan/LOS display (shows field of view for selected unit) |
| **F5** | Toggle minimap visibility |
| **F6** | Toggle team panel visibility |
| **F7** | Toggle unit panel visibility |
| **F8** | Cycle building display (Interiors → Outlines → Elevation → Off) |
| **F9** | Toggle terrain elements visibility |
| **F10** | Toggle bounding box display (cyan=animations, magenta=collision) |

---

## Key Files

### Entry Point
- `src/main.cpp` - Application entry and main loop

### Core Systems
- `src/graphics/Screen.cpp` - Rendering and blitting
- `src/graphics/FontManager.cpp` - Text rendering
- `src/application/GameApplication.cpp` - Game state management
- `src/application/CombatModule.cpp` - Main combat gameplay
- `src/world/World.cpp` - Game world simulation

### Configuration Files
- `config/Soldiers.xml` - Unit definitions
- `config/Squads.xml` - Squad configurations
- `config/Weapons.xml` - Weapon definitions
- `config/Elements.xml` - Terrain elements
- `config/Animations.xml` - Animation definitions
- `config/Maps/*.xml` - Map configurations

---

## Port Phases

| Phase | Component | Status | Effort |
|-------|-----------|--------|--------|
| 0 | Preparation | Complete | Done |
| 1 | Graphics (D3D→SDL2) | Complete | Done |
| 2 | Input (Win32→SDL2) | Complete | Done |
| 3 | Audio (DSound→SDL2_mixer) | Complete | Done |
| 4 | XML (MSXML→tinyxml2) | Complete | Done |
| 5 | Testing & Polish | In Progress | Ongoing |

---

## Development Notes

### Code Style
- Tabs for indentation
- Allman brace style
- PascalCase for classes/methods
- Underscore prefix for member variables (e.g., `_width`)
- See `AGENTS.md` for full coding guidelines

### Self-Tests
Some classes have `SelfTest()` static methods:
```cpp
Screen::SelfTest();       // src/graphics/Screen.cpp:702
ActionQueue::SelfTest();  // src/states/ActionQueue.h:92
```

Run these to verify functionality during development.

### Path Handling
Original code used Windows path separators (`\`). These have been updated for portability:
```cpp
// Old (Windows)
sprintf(path, "%s\\Config", dir);

// New (Portable)
snprintf(path, 256, "%s/config", dir);
```

### Case Sensitivity
Linux filesystem is case-sensitive. Ensure all file references match exactly:
- `config/` not `Config/`
- `graphics/` not `Graphics/`

---

## License

This is a reverse-engineered open-source clone of Close Combat. Original Close Combat is property of Atomic Games/Matrix Games. This project is for educational purposes.

---

## Original Source Code

The `original/` folder contains the legacy codebase from the original SourceForge project:

- **adv-warfare.zip** - Complete CVS repository export from the original project
- **build-20051201.zip** - Original "snapshot 0.1a" release from December 2005
- **screenshots/** - 4 original screenshots from the SourceForge project page

**Original Project**: https://sourceforge.net/projects/adv-warfare/

The original was written for Windows/DirectX and could not be built from the CVS repository due to missing files. This SDL2 port makes the game cross-platform and playable.

---

**Status**: SDL2 port complete - Game is playable! 🎮

Last Updated: 2026-02-13

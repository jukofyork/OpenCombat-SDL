# Zoom Feature Implementation Notes

## Overview
This document summarizes attempts to implement a zoom feature for the OpenCombat game view, what approaches were tried, why they failed, and what would be needed for a successful implementation.

## Target Behavior
- Zoom levels: 1x (default), 2x, 3x (integer multiples only)
- Only the game world (map, units, effects) should zoom
- UI panels (bottom bar, right panels, minimap) should remain at 1x size
- Mouse wheel and +/- keys control zoom
- Minimap yellow viewport rectangle should adjust to show visible area

## Attempted Approaches

### 1. Simple Full-Screen Zoom (Failed)
**Approach:**
- Render everything to a single texture at 1x
- Use `SDL_RenderCopy()` with a scaled destination rectangle
- Scale the entire screen by zoom level

**Result:**
- Zoom effect worked technically
- **Critical Problem:** Zoomed the ENTIRE screen including UI panels
- UI elements became too large and went off-screen

**Why it failed:**
- `SDL_RenderCopy()` scales the entire source texture uniformly
- Cannot selectively zoom only part of the screen with this approach
- UI panels (bottom bar ~68px, right panel ~200px) scale to unusable sizes

---

### 2. Split Rendering - Separate World and UI Surfaces (Failed)
**Approach:**
- Create `m_worldSurface` for game world rendering
- Keep `m_screenSurface` for UI rendering
- Render world to world surface at 1x
- Render UI to screen surface at 1x
- Present: Scale world texture, overlay UI texture at 1x

**Result:**
- Multiple crashes and black screens
- Partial rendering with visual artifacts
- Never achieved working compositing

**Problems encountered:**

1. **Initialization order bugs:**
   - `CreateRenderer()` created `m_worldSurface` successfully
   - `Initialize()` later reset `m_worldSurface = NULL` 
   - Caused segfaults when `Render()` tried to lock NULL surface
   - Fixed by removing redundant NULL assignments

2. **Surface lifecycle issues:**
   - Forgot to initialize `m_worldSurface` and `m_worldTexture` in constructor
   - Missing cleanup in `Shutdown()`
   - `RecreateRendererResources()` didn't recreate world resources on window resize

3. **Compositing problems:**
   - UI surface was full screen and covered world when rendered on top
   - SDL doesn't have built-in alpha blending for texture compositing
   - Clearing UI surface to black covered underlying world content

4. **Complex coordinate management:**
   - Two separate surfaces with different coordinate systems
   - Clipping rectangles had to be calculated separately for each
   - Hard to keep world and UI in sync

---

### 3. Selective Texture Copying (Failed)
**Approach:**
- Render everything once to main surface at 1x
- Use `SDL_RenderCopy()` with source/destination rectangles
- Copy world area scaled up, copy UI areas at 1x

**Result:**
- Ghosting artifacts from previous frames
- Black areas where content should be
- Visual glitches at UI/world boundaries

**Problems:**

1. **Screen clearing issues:**
   - Couldn't clear only parts of the screen between frames
   - Previous frame content bleeding through
   - Artifacts at the edges of scaled regions

2. **Overlapping UI elements:**
   - Right panel overlaps world view area
   - Minimap is in bottom-left which is in the UI panel area
   - Difficult to determine which pixels are "world" vs "UI"

3. **Positioning complexity:**
   - Math for positioning scaled vs unscaled regions was error-prone
   - Hardcoded panel sizes (68px bottom, 200px right) were approximate
   - Different panel visibility states (F5-F7 toggles) changed sizes

---

## Key Technical Challenges

### 1. Rendering Architecture
The game's rendering pipeline:
```
main.cpp::Render() → GameApplication::Render() → CombatModule::Render()
                                              ↓
                                    World::Render(screen, clipRect)
                                    [renders map + units + effects]
                                    
                                    [renders UI panels on top]
```

**Challenge:** All rendering goes through a single `Screen` object writing to one surface. The world and UI are interleaved in the render call, not separated.

### 2. UI Overlap
The UI layout:
```
+------------------------+
| World View             |  ← ~532px high (at 600px window)
|                        |    (minus ~68px bottom panel)
|                        |
|  +---------------+     |
|  | Minimap       |     |  ← Bottom-left corner
|  +---------------+     |
|                        |
+------------------------+  ← ~68px bottom panel
| Bottom Panel           |
+------------------------+

Right side: Unit panel (~200px wide) overlaps world view
```

**Challenge:** UI panels overlap the world view area, making it impossible to cleanly separate "world pixels" from "UI pixels" after rendering.

### 3. Coordinate Systems
Current mouse handling:
```cpp
// In Update():
_game->LeftMouseDown(mouseX, mouseY);  // Screen coordinates

// In CombatModule::LeftMouseDown():
_world->LeftMouseDown(x, y);  // Same screen coordinates
```

**Challenge:** When zoomed 2x, screen pixel (100, 100) = world pixel (50, 50). All mouse input needs coordinate transformation.

### 4. SDL2 Limitations
- `SDL_RenderCopy()` can scale, but scales the entire source rectangle
- No built-in support for "zoom this region, don't zoom that region"
- Texture compositing requires manual blending

---

## What Would Be Needed for Success

### Option A: Refactor Rendering Pipeline (Recommended)

1. **Separate World and UI render methods:**
   ```cpp
   class CombatModule {
       void RenderWorld(Screen* screen, Rect* clip);  // Map + units + effects
       void RenderUI(Screen* screen);                  // Panels only
   };
   ```

2. **Double-buffered offscreen rendering:**
   - Create `m_worldTexture` as render target (SDL_TEXTUREACCESS_TARGET)
   - Render world to texture at 1x
   - Render UI to main screen at 1x
   - Present: Copy world texture scaled, UI unchanged

3. **Proper depth management:**
   - UI should have transparent background where world shows through
   - Or use stencil buffer to mask world vs UI areas

4. **Mouse coordinate transformation:**
   ```cpp
   // Transform screen to world coordinates
   worldX = (screenX / zoomLevel) + originX;
   worldY = (screenY / zoomLevel) + originY;
   ```

5. **Minimap viewport updates:**
   - Adjust yellow rectangle size: `width = screenWidth / zoomLevel`
   - Position based on current origin

### Option B: Hardware-Accelerated Approach

Use OpenGL or SDL2's render targets:
1. Render world to FBO (Frame Buffer Object) at 1x
2. Render UI to default framebuffer at 1x  
3. Draw world FBO as textured quad with scaling
4. Draw UI on top

### Option C: Software Rendering with Scaling

1. Render everything to large buffer at zoom level
2. Scale down UI elements manually before rendering
3. Copy to screen

---

## Working Features (Unaffected by Zoom Attempts)

- Arrow key scroll repeat (hold arrows to scroll)
- Middle-click drag panning
- Minimap yellow viewport rectangle
- Context menu (F-keys)
- All existing gameplay

## Files Modified During Zoom Attempt

If re-implementing, these files were touched:
- `src/main.cpp` - Render loop, mouse handling, zoom constants
- `src/main.h` - Zoom level member variables
- `src/application/CombatModule.h` - RenderWorldOnly/RenderUIOnly declarations
- `src/application/CombatModule.cpp` - Split render implementation
- `src/world/MiniMap.h` - SetZoomLevel method
- `src/world/MiniMap.cpp` - Viewport rectangle scaling
- `src/misc/GameConstants.h` - ZOOM_MIN, ZOOM_MAX, ZOOM_DEFAULT

## Constants Used

```cpp
#define ZOOM_MIN 1
#define ZOOM_MAX 3
#define ZOOM_DEFAULT 1
```

## Current State

Code has been reverted to commit `74d392b` ("Add middle-click drag scrolling to pan the view").
Zoom feature is NOT implemented.

To retry implementation:
1. Choose an approach from "What Would Be Needed for Success"
2. Implement in small, testable steps
3. Test zoom at 1x first (should be identical to no zoom)
4. Then test 2x, 3x
5. Verify mouse coordinates work correctly at each zoom level

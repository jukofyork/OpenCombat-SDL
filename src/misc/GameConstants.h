#pragma once

// Game timing constants
// These are shared between the SDL application and the game logic
// to ensure consistent behavior regardless of rendering speed

// Simulation timestep in milliseconds (33ms = ~30 FPS simulation rate)
// The game simulates at a fixed timestep independent of rendering
#define SIMULATION_TIMESTEP_MS 33

// Scroll repeat configuration
// Scroll speed in pixels per second when holding arrow keys
// The actual scroll per frame is calculated as: (SCROLL_SPEED_PPS * dt) / 1000
#define SCROLL_SPEED_PPS 480

// Initial delay in milliseconds before scroll repeat starts
// Set to 0 to start scrolling immediately when key is held
#define SCROLL_INITIAL_DELAY_MS 200

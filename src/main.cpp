//-----------------------------------------------------------------------------
// File: main.cpp
//
// Desc: SDL2 port of the OpenCombat main application
//-----------------------------------------------------------------------------
#include "main.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <string>

#include "application/GameApplication.h"
#include "graphics/Screen.h"
#include "misc/Color.h"
#include "misc/TGA.h"
#include "graphics/FontManager.h"
#include "application/Globals.h"
#include "misc/GameConstants.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
ApplicationType* g_pApp = NULL;
Globals* g_Globals;

//-----------------------------------------------------------------------------
// Helper function to get current time in milliseconds
//-----------------------------------------------------------------------------
long GetTickCount()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point to the program
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	Globals globals;
	g_Globals = &globals;

	// Run our SelfTests
	Screen::SelfTest();

	// Get the current directory and store it
	char tempDir[256];
	if(getcwd(tempDir, sizeof(tempDir)) == NULL) {
		globals.Application.CurrentDirectory = ".";
	} else {
		globals.Application.CurrentDirectory = tempDir;
	}
	
	// Use portable path separators
	globals.Application.ConfigDirectory = globals.Application.CurrentDirectory + "/config";
	globals.Application.GraphicsDirectory = globals.Application.CurrentDirectory + "/graphics";
	globals.Application.MapsDirectory = globals.Application.CurrentDirectory + "/maps";
	globals.Application.SoundsDirectory = globals.Application.CurrentDirectory + "/sounds";

	// Create and initialize the application
	CSDLApplication app;
	g_pApp = &app;
	globals.Application.Status = (StatusCallback *)&app;
	globals.Application.Cursor = (CursorInterface *)&app;

	if(!app.Initialize()) {
		fprintf(stderr, "Failed to initialize application\n");
		return 1;
	}

	// Run the main loop
	app.Run();

	// Shutdown
	app.Shutdown();

	return 0;
}

//-----------------------------------------------------------------------------
// Name: CSDLApplication()
// Desc: Application constructor
//-----------------------------------------------------------------------------
CSDLApplication::CSDLApplication()
{
	m_bLoadingApp = true;
	m_pFont = NULL;
	m_pSoundManager = NULL;

	m_window = NULL;
	m_renderer = NULL;
	m_screenSurface = NULL;
	m_screenTexture = NULL;

	m_windowWidth = 800;
	m_windowHeight = 600;

	_game = NULL;
	_screen = NULL;
	_fontManager = NULL;
	_millis = 0;

	memset(&_oldMouseState, 0, sizeof(MouseState));
	memset(&_currentMouseState, 0, sizeof(MouseState));
	strcpy(_statusText, "Loading... Please wait");

	// Initialize cursor array to NULL
	for(int i = 0; i < CursorInterface::NumCursorTypes; i++) {
		_cursors[i] = NULL;
	}
	_defaultCursor = NULL;
}

//-----------------------------------------------------------------------------
// Name: ~CSDLApplication()
// Desc: Application destructor
//-----------------------------------------------------------------------------
CSDLApplication::~CSDLApplication()
{
}

//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initialize SDL and create window
//-----------------------------------------------------------------------------
bool CSDLApplication::Initialize()
{
	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		return false;
	}

	// Initialize SDL_ttf
	if(TTF_Init() < 0) {
		fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
		// Continue anyway, we'll handle missing fonts gracefully
	}

	// Create window
	if(!CreateWindow()) {
		return false;
	}

	// Create renderer
	if(!CreateRenderer()) {
		return false;
	}

	// Initialize audio
	InitAudio();

	// Create game objects
	_screen = new Screen();
	_fontManager = new FontManager();
	g_Globals->World.Fonts = _fontManager;

	// Initialize the game
	_game = new GameApplication();
	_game->Initialize(this);
	_game->ChooseModule(GameApplication::AvailableModules::Combat);

	// Initialize font manager
	_fontManager->Initialize(NULL);

	// Load custom cursors
	if(!LoadCursors()) {
		fprintf(stderr, "Warning: Failed to load some cursors\n");
	}

	m_bLoadingApp = false;

	return true;
}

//-----------------------------------------------------------------------------
// Name: CreateWindow()
// Desc: Create the main SDL window
//-----------------------------------------------------------------------------
bool CSDLApplication::CreateWindow()
{
	m_window = SDL_CreateWindow(
		"Open Combat",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		m_windowWidth,
		m_windowHeight,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if(m_window == NULL) {
		fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
		return false;
	}

	// Load and set window icon
	std::string iconPath = g_Globals->Application.CurrentDirectory + "/graphics/Resources/app_icon.tga";
	
	TGA* iconTga = TGA::Create(iconPath.c_str());
	if(iconTga != NULL) {
		int width = iconTga->GetWidth();
		int height = iconTga->GetHeight();
		int depth = iconTga->GetDepth();
		unsigned char* data = iconTga->GetData();
		
		if(width > 0 && height > 0 && data != NULL) {
			SDL_Surface* iconSurface = SDL_CreateRGBSurfaceWithFormat(
				0, width, height, 32, SDL_PIXELFORMAT_RGBA32
			);
			
			if(iconSurface != NULL) {
				// Lock surface
				if(SDL_MUSTLOCK(iconSurface)) {
					SDL_LockSurface(iconSurface);
				}
				
				// Convert BGR/BGRA to RGBA
				unsigned char* pixels = (unsigned char*)iconSurface->pixels;
				int pitch = iconSurface->pitch;
				
				for(int y = 0; y < height; y++) {
					for(int x = 0; x < width; x++) {
						int srcIdx = (y * width + x) * 4;
						int dstIdx = y * pitch + x * 4;
						
						pixels[dstIdx + 0] = data[srcIdx + 2];  // R
						pixels[dstIdx + 1] = data[srcIdx + 1];  // G
						pixels[dstIdx + 2] = data[srcIdx + 0];  // B
						pixels[dstIdx + 3] = data[srcIdx + 3];  // A
					}
				}
				
				// Unlock surface
				if(SDL_MUSTLOCK(iconSurface)) {
					SDL_UnlockSurface(iconSurface);
				}
				
				// Set window icon
				SDL_SetWindowIcon(m_window, iconSurface);
				SDL_FreeSurface(iconSurface);
			}
		}
		delete iconTga;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: CreateRenderer()
// Desc: Create the SDL renderer and screen surface
//-----------------------------------------------------------------------------
bool CSDLApplication::CreateRenderer()
{
	// Create renderer
	m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
	if(m_renderer == NULL) {
		// Try software renderer
		m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
		if(m_renderer == NULL) {
			fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
			return false;
		}
	}

	// Create a surface for software rendering (compatible with original code)
	// We use a 32-bit ARGB format to match the original DirectX format
	m_screenSurface = SDL_CreateRGBSurfaceWithFormat(
		0,
		m_windowWidth,
		m_windowHeight,
		32,
		SDL_PIXELFORMAT_ARGB8888
	);

	if(m_screenSurface == NULL) {
		fprintf(stderr, "SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
		return false;
	}

	// Create texture for presenting to screen
	m_screenTexture = SDL_CreateTexture(
		m_renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		m_windowWidth,
		m_windowHeight
	);

	if(m_screenTexture == NULL) {
		fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: RecreateRendererResources()
// Desc: Recreate surface and texture on window resize
//-----------------------------------------------------------------------------
bool CSDLApplication::RecreateRendererResources(int newWidth, int newHeight)
{
	// Update dimensions
	m_windowWidth = newWidth;
	m_windowHeight = newHeight;

	// Destroy old resources
	if(m_screenTexture) {
		SDL_DestroyTexture(m_screenTexture);
		m_screenTexture = NULL;
	}

	if(m_screenSurface) {
		if(SDL_MUSTLOCK(m_screenSurface) && m_screenSurface->locked > 0) {
			SDL_UnlockSurface(m_screenSurface);
		}
		SDL_FreeSurface(m_screenSurface);
		m_screenSurface = NULL;
	}

	// Create new surface
	m_screenSurface = SDL_CreateRGBSurfaceWithFormat(
		0,
		m_windowWidth,
		m_windowHeight,
		32,
		SDL_PIXELFORMAT_ARGB8888
	);

	if(m_screenSurface == NULL) {
		fprintf(stderr, "SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
		return false;
	}

	// Create new texture
	m_screenTexture = SDL_CreateTexture(
		m_renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		m_windowWidth,
		m_windowHeight
	);

	if(m_screenTexture == NULL) {
		fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
		return false;
	}

	// Update screen capabilities with new surface
	if(_screen) {
		unsigned char* pixels = (unsigned char*)m_screenSurface->pixels;
		_screen->SetCapabilities(
			pixels,
			m_windowWidth,
			m_windowHeight,
			SDL_PIXELFORMAT_ARGB8888,
			m_screenSurface->pitch
		);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: SetMaxWindowSize()
// Desc: Set maximum window size to prevent exceeding map dimensions
//-----------------------------------------------------------------------------
void CSDLApplication::SetMaxWindowSize(int maxWidth, int maxHeight)
{
	if(m_window) {
		// Get current window position
		int currentX, currentY;
		SDL_GetWindowPosition(m_window, &currentX, &currentY);
		
		// If window is currently larger than max, resize it
		if(m_windowWidth > maxWidth || m_windowHeight > maxHeight) {
			int newWidth = (m_windowWidth > maxWidth) ? maxWidth : m_windowWidth;
			int newHeight = (m_windowHeight > maxHeight) ? maxHeight : m_windowHeight;
			
			// Ensure minimum size
			if(newWidth < 800) newWidth = 800;
			if(newHeight < 600) newHeight = 600;
			
			SDL_SetWindowSize(m_window, newWidth, newHeight);
			RecreateRendererResources(newWidth, newHeight);
		}
		
		// Set the maximum size constraint
		SDL_SetWindowMaximumSize(m_window, maxWidth, maxHeight);
	}
}

//-----------------------------------------------------------------------------
// Name: InitAudio()
// Desc: Initialize SDL_mixer audio
//-----------------------------------------------------------------------------
bool CSDLApplication::InitAudio()
{
	// Initialize SDL_mixer
	if(Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 2048) < 0) {
		fprintf(stderr, "Mix_OpenAudio failed: %s\n", Mix_GetError());
		return false;
	}
	
	// Allocate 8 mixing channels (default is usually sufficient)
	Mix_AllocateChannels(8);
	
	return true;
}

//-----------------------------------------------------------------------------
// Name: CleanupAudio()
// Desc: Cleanup SDL_mixer audio resources
//-----------------------------------------------------------------------------
void CSDLApplication::CleanupAudio()
{
	Mix_CloseAudio();
}

//-----------------------------------------------------------------------------
// Name: Shutdown()
// Desc: Cleanup and shutdown
//-----------------------------------------------------------------------------
void CSDLApplication::Shutdown()
{
	// Cleanup cursors
	FreeCursors();

	// Cleanup game objects
	if(_fontManager) {
		_fontManager->Cleanup();
		delete _fontManager;
		_fontManager = NULL;
	}

	if(_screen) {
		_screen->Cleanup();
		delete _screen;
		_screen = NULL;
	}

	if(_game) {
		delete _game;
		_game = NULL;
	}

	// Cleanup SDL resources in proper order
	// First, ensure renderer is not holding references
	if(m_renderer) {
		SDL_RenderClear(m_renderer);
	}

	if(m_screenTexture) {
		SDL_DestroyTexture(m_screenTexture);
		m_screenTexture = NULL;
	}

	// Unlock surface if it was left locked (prevents crash on FreeSurface)
	if(m_screenSurface) {
		// Check if surface is locked by trying to unlock it
		if(SDL_MUSTLOCK(m_screenSurface) && m_screenSurface->locked > 0) {
			SDL_UnlockSurface(m_screenSurface);
		}
		SDL_FreeSurface(m_screenSurface);
		m_screenSurface = NULL;
	}

	// Destroy renderer after texture and surface are gone
	if(m_renderer) {
		SDL_DestroyRenderer(m_renderer);
		m_renderer = NULL;
	}

	// Destroy window last
	if(m_window) {
		SDL_DestroyWindow(m_window);
		m_window = NULL;
	}

	// Cleanup audio
	CleanupAudio();

	// Cleanup SDL_ttf
	TTF_Quit();

	// Cleanup SDL
	SDL_Quit();
}

//-----------------------------------------------------------------------------
// Name: Run()
// Desc: Main application loop
//-----------------------------------------------------------------------------
void CSDLApplication::Run()
{
	bool running = true;
	SDL_Event event;

	while(running) {
		// Process events
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					running = false;
					break;

			case SDL_KEYDOWN:
				HandleKeyDown(event.key.keysym.sym);
				break;

			case SDL_KEYUP:
				HandleKeyUp(event.key.keysym.sym);
				break;

				case SDL_MOUSEBUTTONDOWN:
					HandleMouseButtonDown(event.button.button, event.button.x, event.button.y);
					break;

				case SDL_MOUSEBUTTONUP:
					HandleMouseButtonUp(event.button.button, event.button.x, event.button.y);
					break;

			case SDL_MOUSEMOTION:
				HandleMouseMotion(event.motion.x, event.motion.y);
				break;

			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
					int newWidth = event.window.data1;
					int newHeight = event.window.data2;
					if(newWidth != m_windowWidth || newHeight != m_windowHeight) {
						RecreateRendererResources(newWidth, newHeight);
					}
				} else if(event.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
					// Block maximize - restore to previous size
					SDL_RestoreWindow(m_window);
				}
				break;
			}
		}

		// Update game state
		Update();

		// Render frame
		Render();

		// Small delay to prevent hogging CPU
		SDL_Delay(1);
	}
}

//-----------------------------------------------------------------------------
// Name: Update()
// Desc: Update game state (called once per frame)
//-----------------------------------------------------------------------------
void CSDLApplication::Update()
{
	// Update the game state
	long oldMillis = _millis;
	long currentMillis = GetTickCount();
	if(oldMillis != 0 && (currentMillis - oldMillis) >= SIMULATION_TIMESTEP_MS) {
		_millis = currentMillis;
		_game->Simulate(SIMULATION_TIMESTEP_MS);
	} else if(oldMillis == 0) {
		_millis = currentMillis;
	}

	// Check mouse states
	if(_currentMouseState.bLeftDown && !_oldMouseState.bLeftDown) {
		// Mouse was up, now is down
		int mouseX = _currentMouseState.X;
		int mouseY = _currentMouseState.Y;
		_game->LeftMouseDown(mouseX, mouseY);
	} else if(!_currentMouseState.bLeftDown && _oldMouseState.bLeftDown) {
		int mouseX = _currentMouseState.X;
		int mouseY = _currentMouseState.Y;
		_game->LeftMouseUp(mouseX, mouseY);
	} else if(_currentMouseState.bLeftDown && _oldMouseState.bLeftDown) {
		// Mouse is dragged
		int mouseX = _currentMouseState.X;
		int mouseY = _currentMouseState.Y;
		_game->LeftMouseDrag(mouseX, mouseY);
	}
	_oldMouseState.bLeftDown = _currentMouseState.bLeftDown;

	if(_currentMouseState.bRightDown && !_oldMouseState.bRightDown) {
		// Mouse was up, now is down
		int mouseX = _currentMouseState.X;
		int mouseY = _currentMouseState.Y;
		_game->RightMouseDown(mouseX, mouseY);
	} else if(!_currentMouseState.bRightDown && _oldMouseState.bRightDown) {
		int mouseX = _currentMouseState.X;
		int mouseY = _currentMouseState.Y;
		_game->RightMouseUp(mouseX, mouseY);
	} else if(_currentMouseState.bRightDown && _oldMouseState.bRightDown) {
		// Mouse is dragged
		int mouseX = _currentMouseState.X;
		int mouseY = _currentMouseState.Y;
		_game->RightMouseDrag(mouseX, mouseY);
	}
	_oldMouseState.bRightDown = _currentMouseState.bRightDown;

	if(_currentMouseState.bMiddleDown && !_oldMouseState.bMiddleDown) {
		// Middle mouse was up, now is down
		int mouseX = _currentMouseState.X;
		int mouseY = _currentMouseState.Y;
		_game->MiddleMouseDown(mouseX, mouseY);
	} else if(!_currentMouseState.bMiddleDown && _oldMouseState.bMiddleDown) {
		int mouseX = _currentMouseState.X;
		int mouseY = _currentMouseState.Y;
		_game->MiddleMouseUp(mouseX, mouseY);
	} else if(_currentMouseState.bMiddleDown && _oldMouseState.bMiddleDown) {
		// Middle mouse is dragged
		int mouseX = _currentMouseState.X;
		int mouseY = _currentMouseState.Y;
		_game->MiddleMouseDrag(mouseX, mouseY);
	}
	_oldMouseState.bMiddleDown = _currentMouseState.bMiddleDown;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Render the scene
//-----------------------------------------------------------------------------
void CSDLApplication::Render()
{
	// Lock the surface for direct pixel access
	if(SDL_LockSurface(m_screenSurface) < 0) {
		return;
	}

	// Set up the screen object with the surface pixel buffer
	unsigned char* pixels = (unsigned char*)m_screenSurface->pixels;
	int pitch = m_screenSurface->pitch;
	int width = m_screenSurface->w;
	int height = m_screenSurface->h;
	Uint32 format = m_screenSurface->format->format;

	_screen->SetSurface(m_screenSurface);
	_screen->SetRenderer(m_renderer);
	_screen->SetCapabilities(pixels, width, height, format, pitch);
	_screen->SetCursorPosition(_currentMouseState.X, _currentMouseState.Y);

	// Clear the screen
	SDL_FillRect(m_screenSurface, NULL, SDL_MapRGB(m_screenSurface->format, 0, 0, 0));

	// Render the game
	_game->Render(_screen);

	// Unlock the surface
	SDL_UnlockSurface(m_screenSurface);

	// Update the texture with the surface content
	SDL_UpdateTexture(m_screenTexture, NULL, m_screenSurface->pixels, m_screenSurface->pitch);

	// Clear the renderer and copy the texture
	SDL_RenderClear(m_renderer);
	SDL_RenderCopy(m_renderer, m_screenTexture, NULL, NULL);
	SDL_RenderPresent(m_renderer);
}

//-----------------------------------------------------------------------------
// Name: HandleKeyUp()
// Desc: Handle key release events
//-----------------------------------------------------------------------------
void CSDLApplication::HandleKeyUp(SDL_Keycode key)
{
	int gameKey = 0;
	
	// Map SDL keycodes to game key codes (Windows virtual key codes)
	switch(key) {
		// Arrow keys (Windows VK codes)
		case SDLK_LEFT:   gameKey = 0x25; break;
		case SDLK_UP:     gameKey = 0x26; break;
		case SDLK_RIGHT:  gameKey = 0x27; break;
		case SDLK_DOWN:   gameKey = 0x28; break;
		
		// Function keys (mapped to ASCII codes as expected by CombatModule)
		case SDLK_F2:     gameKey = 113; break;  // 'q'
		case SDLK_F3:     gameKey = 114; break;  // 'r'
		case SDLK_F5:     gameKey = 116; break;  // 't'
		case SDLK_F6:     gameKey = 117; break;  // 'u'
		case SDLK_F7:     gameKey = 118; break;  // 'v'
		case SDLK_F8:     gameKey = 119; break;  // 'w'
		case SDLK_F9:     gameKey = 120; break;  // 'x'
		
		// Letter keys (pass through ASCII values)
		// Note: SDL2 keycodes for letters are lowercase, so we check both cases
		case SDLK_k:      gameKey = 'k'; break;
		case SDLK_f:      gameKey = 'f'; break;
		
		default:
			// For other keys, try to use the ASCII value if it's in range
			if(key < 128) {
				gameKey = key;
			} else {
				return;  // Unknown key, ignore
			}
	}
	
	// Pass the mapped key to the game
	if(_game) {
		_game->KeyUp(gameKey);
	}
}

//-----------------------------------------------------------------------------
// Name: HandleKeyDown()
// Desc: Handle key press events (for key repeat)
//-----------------------------------------------------------------------------
void CSDLApplication::HandleKeyDown(SDL_Keycode key)
{
	int gameKey = 0;
	
	// Map SDL keycodes to game key codes (Windows virtual key codes)
	switch(key) {
		// Arrow keys (Windows VK codes)
		case SDLK_LEFT:   gameKey = 0x25; break;
		case SDLK_UP:     gameKey = 0x26; break;
		case SDLK_RIGHT:  gameKey = 0x27; break;
		case SDLK_DOWN:   gameKey = 0x28; break;
		
		// Function keys (mapped to ASCII codes as expected by CombatModule)
		case SDLK_F2:     gameKey = 113; break;  // 'q'
		case SDLK_F3:     gameKey = 114; break;  // 'r'
		case SDLK_F5:     gameKey = 116; break;  // 't'
		case SDLK_F6:     gameKey = 117; break;  // 'u'
		case SDLK_F7:     gameKey = 118; break;  // 'v'
		case SDLK_F8:     gameKey = 119; break;  // 'w'
		case SDLK_F9:     gameKey = 120; break;  // 'x'
		
		// Letter keys (pass through ASCII values)
		case SDLK_k:      gameKey = 'k'; break;
		case SDLK_f:      gameKey = 'f'; break;
		
		default:
			// For other keys, try to use the ASCII value if it's in range
			if(key < 128) {
				gameKey = key;
			} else {
				return;  // Unknown key, ignore
			}
	}
	
	// Pass the mapped key to the game
	if(_game) {
		_game->KeyDown(gameKey);
	}
}

//-----------------------------------------------------------------------------
// Name: HandleMouseButtonDown()
// Desc: Handle mouse button press
//-----------------------------------------------------------------------------
void CSDLApplication::HandleMouseButtonDown(Uint8 button, int x, int y)
{
	if(button == SDL_BUTTON_LEFT) {
		_currentMouseState.bLeftDown = true;
	} else if(button == SDL_BUTTON_RIGHT) {
		_currentMouseState.bRightDown = true;
	} else if(button == SDL_BUTTON_MIDDLE) {
		_currentMouseState.bMiddleDown = true;
	}
	_currentMouseState.X = x;
	_currentMouseState.Y = y;
}

//-----------------------------------------------------------------------------
// Name: HandleMouseButtonUp()
// Desc: Handle mouse button release
//-----------------------------------------------------------------------------
void CSDLApplication::HandleMouseButtonUp(Uint8 button, int x, int y)
{
	if(button == SDL_BUTTON_LEFT) {
		_currentMouseState.bLeftDown = false;
	} else if(button == SDL_BUTTON_RIGHT) {
		_currentMouseState.bRightDown = false;
	} else if(button == SDL_BUTTON_MIDDLE) {
		_currentMouseState.bMiddleDown = false;
	}
	_currentMouseState.X = x;
	_currentMouseState.Y = y;
}

//-----------------------------------------------------------------------------
// Name: HandleMouseMotion()
// Desc: Handle mouse movement
//-----------------------------------------------------------------------------
void CSDLApplication::HandleMouseMotion(int x, int y)
{
	_currentMouseState.X = x;
	_currentMouseState.Y = y;
}

//-----------------------------------------------------------------------------
// Name: Status()
// Desc: Update status text
//-----------------------------------------------------------------------------
void CSDLApplication::Status(char *msg)
{
	if(msg) {
		strncpy(_statusText, msg, 255);
		_statusText[255] = '\0';
	}
}

//-----------------------------------------------------------------------------
// Name: SetGameCursor()
// Desc: Set the game cursor
//-----------------------------------------------------------------------------
void CSDLApplication::SetGameCursor(int cursorType)
{
	// TODO: Implement custom cursors using SDL
	UNREFERENCED_PARAMETER(cursorType);
}

//-----------------------------------------------------------------------------
// Name: ShowCursor()
// Desc: Show/hide cursor with specific type (from CursorInterface)
//-----------------------------------------------------------------------------
void CSDLApplication::ShowCursor(bool bShow, CursorType type)
{
	if(bShow) {
		// Set the appropriate cursor
		if(type == CursorInterface::CursorType::Regular) {
			// Use system default cursor
			SDL_SetCursor(_defaultCursor);
		} else if(type >= 0 && type < CursorInterface::NumCursorTypes && _cursors[type] != NULL) {
			SDL_SetCursor(_cursors[type]);
		}
	}
	
	SDL_ShowCursor(bShow ? SDL_ENABLE : SDL_DISABLE);
}

//-----------------------------------------------------------------------------
// Name: LoadCursors()
// Desc: Load all custom cursor TGA files
//-----------------------------------------------------------------------------
bool CSDLApplication::LoadCursors()
{
	// Save the default cursor for later use
	_defaultCursor = SDL_GetCursor();
	
	// Map cursor types to TGA file names (matching original DirectX version)
	const char* cursorFiles[CursorInterface::NumCursorTypes] = {
		"graphics/UI/Cursors/Mark Blue.tga",              // MarkBlue
		"graphics/UI/Cursors/Mark Purple.tga",            // MarkPurple
		"graphics/UI/Cursors/Mark Red.tga",               // MarkRed
		"graphics/UI/Cursors/Mark Yellow.tga",            // MarkYellow
		"graphics/UI/Cursors/Mark Orange.tga",            // MarkOrange
		"graphics/UI/Cursors/Mark Brown.tga",             // MarkBrown
		"graphics/UI/Cursors/Mark Green.tga",             // MarkGreen
		"graphics/UI/Cursors/Mark Grey.tga",              // MarkGrey
		"graphics/UI/Cursors/Crosshairs Black.tga",       // CrosshairsBlack
		"graphics/UI/Cursors/Crosshairs Red.tga",         // CrosshairsRed
		"graphics/UI/Cursors/Crosshairs Yellow.tga",      // CrosshairsYellow
		"graphics/UI/Cursors/Crosshairs Green.tga",       // CrosshairsGreen
		"graphics/UI/Cursors/Empty Crosshairs Black.tga", // CrosshairsEmptyBlack
		"graphics/UI/Cursors/Empty Crosshairs Red.tga",   // CrosshairsEmptyRed
		"graphics/UI/Cursors/Empty Crosshairs Yellow.tga", // CrosshairsEmptyYellow
		"graphics/UI/Cursors/Empty Crosshairs Green.tga", // CrosshairsEmptyGreen
		NULL                                              // Regular (use system default)
	};
	
	// Load each cursor
	for(int i = 0; i < CursorInterface::NumCursorTypes; i++) {
		if(cursorFiles[i] == NULL) {
			// Skip Regular cursor - use system default
			_cursors[i] = NULL;
			continue;
		}
		
		// Build full path
		std::string path = g_Globals->Application.CurrentDirectory + "/" + cursorFiles[i];
		
		// Load TGA file
		TGA* tga = TGA::Create(path.c_str());
		if(tga == NULL) {
			fprintf(stderr, "Failed to load cursor: %s\n", path.c_str());
			_cursors[i] = NULL;
			continue;
		}
		
		// Get TGA properties
		int width = tga->GetWidth();
		int height = tga->GetHeight();
		unsigned char* data = tga->GetData();
		
		if(width <= 0 || height <= 0 || data == NULL) {
			fprintf(stderr, "Invalid cursor data: %s\n", path.c_str());
			delete tga;
			_cursors[i] = NULL;
			continue;
		}
		
		// Create SDL surface in RGBA format
		SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
			0, width, height, 32, SDL_PIXELFORMAT_RGBA32
		);
		
		if(surface == NULL) {
			fprintf(stderr, "Failed to create surface for cursor: %s\n", SDL_GetError());
			delete tga;
			_cursors[i] = NULL;
			continue;
		}
		
		// Lock surface before accessing pixels
		if(SDL_MUSTLOCK(surface)) {
			SDL_LockSurface(surface);
		}
		
		// Copy and convert TGA data (BGRA) to RGBA
		unsigned char* pixels = (unsigned char*)surface->pixels;
		int pitch = surface->pitch;
		
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				int srcIdx = (y * width + x) * 4;
				int dstIdx = y * pitch + x * 4;
				// BGRA to RGBA
				pixels[dstIdx + 0] = data[srcIdx + 2];  // R
				pixels[dstIdx + 1] = data[srcIdx + 1];  // G
				pixels[dstIdx + 2] = data[srcIdx + 0];  // B
				pixels[dstIdx + 3] = data[srcIdx + 3];  // A
			}
		}
		
		// Unlock surface
		if(SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}
		
		// Create cursor with centered hotspot (16,16 for 32x32 cursors)
		int hotspotX = width / 2;
		int hotspotY = height / 2;
		_cursors[i] = SDL_CreateColorCursor(surface, hotspotX, hotspotY);
		
		if(_cursors[i] == NULL) {
			fprintf(stderr, "Failed to create cursor from surface: %s\n", SDL_GetError());
		}
		
		// Clean up
		SDL_FreeSurface(surface);
		delete tga;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Name: FreeCursors()
// Desc: Free all custom cursor resources
//-----------------------------------------------------------------------------
void CSDLApplication::FreeCursors()
{
	// Free all loaded cursors
	for(int i = 0; i < CursorInterface::NumCursorTypes; i++) {
		if(_cursors[i] != NULL) {
			SDL_FreeCursor(_cursors[i]);
			_cursors[i] = NULL;
		}
	}
}

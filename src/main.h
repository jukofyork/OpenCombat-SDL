//-----------------------------------------------------------------------------
// File: main.h
//
// Desc: SDL2 version of the main application header
//-----------------------------------------------------------------------------
#pragma once

#include <SDL2/SDL.h>
#include "misc/StatusCallback.h"
#include "application/CursorInterface.h"

// Forward declaration for cursor system
struct SDL_Cursor;

// Class forward declarations
class GameApplication;
class Screen;
class FontManager;

//-----------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------
#define SDLAPP_KEY "OpenCombat"

// Keeps track of previous mouse states
struct MouseState
{
	int X;
	int Y;
	bool bLeftDown;
	bool bRightDown;
};

//-----------------------------------------------------------------
// Name: class CSDLApplication
// Desc: SDL2 Application class replacing the Direct3D version
//-----------------------------------------------------------------
class CSDLApplication : public StatusCallback, public CursorInterface
{
protected:
	bool m_bLoadingApp;          // TRUE, if the app is loading
	FontManager* m_pFont;        // Font for drawing text
	void* m_pSoundManager;       // Sound manager (placeholder for now)

	// SDL objects
	SDL_Window* m_window;
	SDL_Renderer* m_renderer;
	SDL_Surface* m_screenSurface;
	SDL_Texture* m_screenTexture;

	// Window dimensions
	int m_windowWidth;
	int m_windowHeight;

	// Game objects
	GameApplication* _game;
	Screen* _screen;
	FontManager* _fontManager;
	long _millis;
	MouseState _oldMouseState;
	MouseState _currentMouseState;
	char _statusText[256];

	// Custom cursors
	SDL_Cursor* _cursors[CursorInterface::NumCursorTypes];
	SDL_Cursor* _defaultCursor;

public:
	CSDLApplication();
	virtual ~CSDLApplication();

	// Application lifecycle
	bool Initialize();
	void Run();
	void Shutdown();

	// Main loop functions
	void ProcessEvents();
	void Update();
	void Render();

	// Input handling
	void HandleKeyUp(SDL_Keycode key);
	void HandleKeyDown(SDL_Keycode key);
	void HandleMouseButtonDown(Uint8 button, int x, int y);
	void HandleMouseButtonUp(Uint8 button, int x, int y);
	void HandleMouseMotion(int x, int y);

	// From StatusCallback
	virtual void Status(char *msg);

	// Cursor handling
	void SetGameCursor(int cursorType);
	
	// From CursorInterface
	virtual void ShowCursor(bool bShow, CursorType type);

	// Set maximum window size to prevent exceeding map dimensions
	void SetMaxWindowSize(int maxWidth, int maxHeight);

	// Get the screen object
	Screen* GetScreen() { return _screen; }

protected:
	bool CreateWindow();
	bool CreateRenderer();
	bool InitAudio();
	void CleanupAudio();
	bool LoadCursors();
	void FreeCursors();
	bool RecreateRendererResources(int newWidth, int newHeight);
};

// Global access to the app
typedef CSDLApplication ApplicationType;

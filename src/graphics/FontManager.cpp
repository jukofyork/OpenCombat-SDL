#include "FontManager.h"
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <filesystem>
#include "../misc/Color.h"
#include "Screen.h"
#include "../application/Globals.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif

FontManager::FontManager(void)
{
	for(int i = 0; i < FontSize_Count; ++i) {
		_fonts[i] = NULL;
	}
}

FontManager::~FontManager(void)
{
	Cleanup();
}

TTF_Font*
FontManager::LoadFont(int pointSize)
{
	// Load DejaVu Sans from local project directory
	std::filesystem::path fontPath = g_Globals->Application.GraphicsDirectory / "UI/DejaVuSans.ttf";
	return TTF_OpenFont(fontPath.c_str(), pointSize);
}

void 
FontManager::Initialize(void *data)
{
	UNREFERENCED_PARAMETER(data);
	
	// Initialize SDL_ttf if not already done
	if(TTF_WasInit() == 0) {
		TTF_Init();
	}
	
	// Load small font (9pt) for UI panels
	_fonts[FontSize_Small] = LoadFont(9);
	
	// Load large font (14pt) for victory locations and headers
	_fonts[FontSize_Large] = LoadFont(14);
}

void 
FontManager::Cleanup()
{
	for(int i = 0; i < FontSize_Count; ++i) {
		if(_fonts[i] != NULL) {
			TTF_CloseFont(_fonts[i]);
			_fonts[i] = NULL;
		}
	}
}

void
FontManager::Restore()
{
	// Not needed for SDL_ttf
}

void
FontManager::Invalidate()
{
	// Not needed for SDL_ttf
}

void
FontManager::GetTextSize(const char *msg, int *w, int *h, FontSize size)
{
	if(size < 0 || size >= FontSize_Count) {
		size = FontSize_Small;
	}
	
	if(_fonts[size] == NULL || msg == NULL) {
		*w = 0;
		*h = 0;
		return;
	}
	
	TTF_SizeText(_fonts[size], msg, w, h);
}

void 
FontManager::Render(Screen *screen, const char *msg, int x, int y, Color *c, FontSize size)
{
	if(size < 0 || size >= FontSize_Count) {
		size = FontSize_Small;
	}
	
	if(_fonts[size] == NULL || msg == NULL || screen == NULL) {
		return;
	}
	
	// Create color for SDL_ttf
	SDL_Color color;
	color.r = c->red;
	color.g = c->green;
	color.b = c->blue;
	color.a = c->alpha;
	
	// Render text to surface
	SDL_Surface *textSurface = TTF_RenderText_Blended(_fonts[size], msg, color);
	if(textSurface == NULL) {
		return;
	}
	
	// Blit text surface to screen
	screen->BlitSurface(textSurface, x, y);
	
	// Clean up
	SDL_FreeSurface(textSurface);
}

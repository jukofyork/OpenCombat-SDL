#pragma once

#include <string>
#include <SDL2/SDL_ttf.h>

class Color;
class Screen;

// Font sizes
enum class FontSize {
	Small = 0,   // 9pt - UI panels
	Large = 1,   // 14pt - Victory locations, headers
	Count = 2
};

class FontManager
{
public:
	FontManager(void);
	virtual ~FontManager(void);

	// First time initialization
	void Initialize(void *data);
	// Final cleanup
	void Cleanup();
	// Reset (not needed for SDL_ttf)
	void Restore();
	// Invalidate (not needed for SDL_ttf)
	void Invalidate();

	// Render text with specified font size
	void Render(Screen *screen, const std::string &msg, int x, int y, Color *c, FontSize size = FontSize::Small);
	
	// Get text dimensions with specified font size
	void GetTextSize(const std::string &msg, int *w, int *h, FontSize size = FontSize::Small);

protected:
    TTF_Font* _fonts[static_cast<int>(FontSize::Count)]; // Fonts at different sizes
	
	// Load a font at a specific size
	TTF_Font* LoadFont(int pointSize);
};

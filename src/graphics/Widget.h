#pragma once

#include <string>
#include <misc/Structs.h>
#include <misc/TGA.h>
#include <misc/Color.h>

class Screen;

class Widget
{
public:
	Widget(const std::string &name, TGA *tga);
	virtual ~Widget(void);

	// Renders this widget
	virtual void Render(Screen *screen, int x, int y, Rect *clip);
	virtual void Render(Screen *screen, int x, int y);
	virtual void Render(Screen *screen, int x, int y, int w, int h);
	virtual void Render(Screen *screen, int x, int y, int w, int h, bool useAlpha);
	virtual void Render(Screen *screen, int x, int y, Color *transparentColor);

	// Retrieves the name of this widget
	inline const char *GetName() const { return _name.c_str(); }

	// Clone's this widget
	Widget *Clone();

	// Gets the width and height of this widget
	inline int GetWidth() { return _tga->GetWidth(); }
	inline int GetHeight() { return _tga->GetHeight(); }

	// Gets the underlying image
	inline TGA *GetImage() { return _tga; }

	// Sets the index
	inline void SetIndex(int i) { _index = i; }
	inline int GetIndex() { return _index; }

protected:
	std::string _name;
	int _index;
	TGA *_tga;
};

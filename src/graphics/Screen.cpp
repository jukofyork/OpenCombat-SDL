#include "Screen.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include "../misc/Color.h"
#include "../misc/Structs.h"
#include "./SoldierMasks.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif

Screen::Screen(void)
{
	_surface = nullptr;
	_renderer = nullptr;
	Origin.x = 0;
	Origin.y = 0;
}

Screen::~Screen(void)
{
	Cleanup();
}

void
Screen::Cleanup()
{
	// SDL resources are managed externally
	_surface = nullptr;
	_renderer = nullptr;
}

void
Screen::SetSurface(SDL_Surface *surface)
{
	_surface = surface;
}

void
Screen::SetRenderer(SDL_Renderer *renderer)
{
	_renderer = renderer;
}

void
Screen::SetCapabilities(unsigned char *bits, int width, int height, int format, int pitch)
{
	_bits = bits;
	_width = width;
	_height = height;
	_format = format;
	_pitch = pitch;

	// SDL pixel format handling
	switch(_format) {
		case SDL_PIXELFORMAT_ARGB8888:
		case SDL_PIXELFORMAT_RGBA8888:
		case SDL_PIXELFORMAT_ABGR8888:
		case SDL_PIXELFORMAT_BGRA8888:
			_bytes_per_pixel = 4;
			break;
		case SDL_PIXELFORMAT_RGB565:
		case SDL_PIXELFORMAT_BGR565:
			_bytes_per_pixel = 2;
			break;
		default:
			// Default to 4 bytes per pixel for 32-bit formats
			_bytes_per_pixel = 4;
	}
}

void
Screen::Clear(Color *c)
{
	// This is going to be really slow, but whatever
	for(int j = 0; j < _height; ++j) {
		for(int i = 0; i < _width; ++i) {
			_bits[j*_pitch + i*_bytes_per_pixel + 0] = 0;
			_bits[j*_pitch + i*_bytes_per_pixel + 1] = c->red;
			_bits[j*_pitch + i*_bytes_per_pixel + 2] = c->green;
			_bits[j*_pitch + i*_bytes_per_pixel + 3] = c->blue;
		}
	}
}

// XXX/GWS: All of the blitting routines in here need to be optimized!!!
void
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sw, int sh, int sbytes_per_pixel)
{
	UNREFERENCED_PARAMETER(sh);
	assert(_bytes_per_pixel == sbytes_per_pixel);

	// Bounds checking - skip if completely off-screen
	if (dx >= _width || dy >= _height || dx + dw <= 0 || dy + dh <= 0)
		return;

	// Clamp to screen bounds
	int srcX = 0, srcY = 0;
	if (dx < 0) {
		srcX = -dx;
		dw += dx;
		dx = 0;
	}
	if (dy < 0) {
		srcY = -dy;
		dh += dy;
		dy = 0;
	}
	if (dx + dw > _width)
		dw = _width - dx;
	if (dy + dh > _height)
		dh = _height - dy;

	for(int j = 0; j < dh; ++j) {
		memcpy(&(_bits[(j+dy)*_pitch+dx*_bytes_per_pixel]), &(src[(j+srcY)*sw*sbytes_per_pixel + srcX*sbytes_per_pixel]), dw*_bytes_per_pixel);
	}
}

void 
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sw, int sh, int sbytes_per_pixel, Color *transparentColor)
{
	UNREFERENCED_PARAMETER(sh);
	unsigned char r,g,b;
	assert(_bytes_per_pixel == sbytes_per_pixel);
	int sx=0,sy=0;

	if(dx < _clip.x) 
	{ 
		sx += (_clip.x-dx);
		dw -= (_clip.x-dx);
		dx = _clip.x;
	}
	else if(dx >= (_clip.x+_clip.w))
	{
		return;
	}
	
	if((dx+dw) > (_clip.x+_clip.w))
	{
		dw = (_clip.x+_clip.w) - dx;
	}

	if(dy < _clip.y)
	{
		sy += (_clip.y-dy);
		dh -= (_clip.y-dy);
		dy = _clip.y;
	}
	else if(dy >= (_clip.y+_clip.h))
	{
		return;
	}

	if((dy+dh) > (_clip.y+_clip.h))
	{
		dh = (_clip.y+_clip.h) - dy;
	}

	for(int j = 0; j < dh; ++j) {
		for(int i = 0; i < dw; ++i) {
			r = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 2];
			g =	src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 1];
			b = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 0];
			
			// First look at transparency
			if(r != transparentColor->red || g != transparentColor->green || b != transparentColor->blue) {
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = b;
			}
		}
	}
}

// Blits with rotation
// The basic rotation matrix is given by:
//
// [x', y'] = [ x*cos(theta) + y*sin(theta) , y*cos(theta) - x*sin(theta) ]
void
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sw, int sh, int sbytes_per_pixel, Color *transparentColor, int rotx, int roty, double angle)
{
	int xp, yp;
	double cosT = cos(angle);
	double sinT = sin(angle);
	unsigned char r,g,b;
	assert(_bytes_per_pixel == sbytes_per_pixel);
	UNREFERENCED_PARAMETER(sh);

	for(int j = 0; j < dh; ++j) {
		for(int i = 0; i < dw; ++i) {
			xp = (int)((i-rotx)*cosT + (j-roty)*sinT);
			yp = (int)((j-roty)*cosT - (i-rotx)*sinT);
			r = src[(j)*sw*sbytes_per_pixel + (i)*sbytes_per_pixel + 2];
			g =	src[(j)*sw*sbytes_per_pixel + (i)*sbytes_per_pixel + 1];
			b = src[(j)*sw*sbytes_per_pixel + (i)*sbytes_per_pixel + 0];

			if(r != transparentColor->red || g != transparentColor->green || b != transparentColor->blue) {
				// Bounds check to prevent buffer overflow
				int destX = xp + dx + rotx;
				int destY = yp + dy + roty;
				if(destX >= 0 && destX < _width && destY >= 0 && destY < _height) {
					_bits[(destY)*_pitch+(destX)*_bytes_per_pixel + 2] = r;
					_bits[(destY)*_pitch+(destX)*_bytes_per_pixel + 1] = g;
					_bits[(destY)*_pitch+(destX)*_bytes_per_pixel + 0] = b;
				}
			}
		}
	}
}

void
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, int sbytes_per_pixel)
{
	UNREFERENCED_PARAMETER(sh);
	assert(_bytes_per_pixel == sbytes_per_pixel);

	// Bounds checking - skip if completely off-screen
	if (dx >= _width || dy >= _height || dx + dw <= 0 || dy + dh <= 0)
		return;

	// Clamp to screen bounds
	if (dx < 0) {
		dw += dx;
		sx -= dx;
		dx = 0;
	}
	if (dy < 0) {
		dh += dy;
		sy -= dy;
		dy = 0;
	}
	if (dx + dw > _width)
		dw = _width - dx;
	if (dy + dh > _height)
		dh = _height - dy;

	for(int j = 0; j < dh; ++j) {
		memcpy(&(_bits[(j+dy)*_pitch+dx*_bytes_per_pixel]), &(src[(j+sy)*sw*sbytes_per_pixel + sx*sbytes_per_pixel]), dw*_bytes_per_pixel);
	}
}

void
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, int sbytes_per_pixel, bool useAlpha)
{
	UNREFERENCED_PARAMETER(sh);
	if(!useAlpha) {
		Blit(src, dx,dy,dw,dh,sx,sy,sw,sh,sbytes_per_pixel);
	} else {
		unsigned char r,g,b,a,origR,origG,origB;

		// Bounds checking - skip if completely off-screen
		if (dx >= _width || dy >= _height || dx + dw <= 0 || dy + dh <= 0)
			return;

		// Calculate actual draw region
		int startX = 0, startY = 0;
		int endX = dw, endY = dh;
		
		if (dx < 0) {
			startX = -dx;
			sx += startX;
		}
		if (dy < 0) {
			startY = -dy;
			sy += startY;
		}
		if (dx + dw > _width)
			endX = _width - dx;
		if (dy + dh > _height)
			endY = _height - dy;

		for(int j = startY; j < endY; ++j) {
			for(int i = startX; i < endX; ++i) {
				// new pixel = (alpha)(pixel A color) + (1 - alpha)(pixel B color)
				a = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 3];
				r = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 2];
				g =	src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 1];
				b = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 0];
				 
				origR = _bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2];
				origG = _bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1];
				origB = _bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0];
	
				// XXX/GWS: Need to speed this up!!!
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (a*(origR-r) >> 8) + r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (a*(origG-g) >> 8) + g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (a*(origB-b) >> 8) + b;
			}
		}
	}
}

// XXX/GWS: We can speed this one up a lot
void
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh,
			 int sx, int sy, int sw, int sh,
			 Color *transparentColor, Color *shadowColor, Color *hilitColor, Color *hilitShadowColor,
			 bool bHilit, int sbytes_per_pixel)
{
	UNREFERENCED_PARAMETER(sh);
	unsigned char r,g,b;

	// Bounds checking - skip if completely off-screen
	if (dx >= _width || dy >= _height || dx + dw <= 0 || dy + dh <= 0)
		return;

	// Calculate actual draw region
	int startX = 0, startY = 0;
	int endX = dw, endY = dh;

	if (dx < 0) {
		startX = -dx;
		sx += startX;
	}
	if (dy < 0) {
		startY = -dy;
		sy += startY;
	}
	if (dx + dw > _width)
		endX = _width - dx;
	if (dy + dh > _height)
		endY = _height - dy;

	for(int j = startY; j < endY; ++j) {
		for(int i = startX; i < endX; ++i) {
			r = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 2];
			g =	src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 1];
			b = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 0];

			// First look at transparency
			if(r != transparentColor->red || g != transparentColor->green || b != transparentColor->blue) {
				// Next look at shadow
				if(r == shadowColor->red && g == shadowColor->green && b == shadowColor->blue) {
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] >>= 2) *= 3;
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] >>= 2) *= 3;
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] >>= 2) *= 3;
				} else {
					// Look at hiliting
					if(bHilit) {
						// Draw normal, with hiliting
						if(r == hilitShadowColor->red && g == hilitShadowColor->green && b == hilitShadowColor->blue) {
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = hilitColor->red;
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = hilitColor->green;
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = hilitColor->blue;
						} else {
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = r;
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = g;
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = b;
						}
					} else {
						// Remove hiliting color
						if(r != hilitColor->red || g != hilitColor->green || b != hilitColor->blue) {
							if(r == hilitShadowColor->red && g == hilitShadowColor->green && b == hilitShadowColor->blue) {
								(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] >>= 2) *= 3;
								(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] >>= 2) *= 3;
								(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] >>= 2) *= 3;
							} else {
								_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = r;
								_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = g;
								_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = b;
							}
						}
					}
				}
			}
		}
	}
}

/**
 * This performs a blit with transparency and shadowing.
 */
void 
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, 
			 int sx, int sy, int sw, int sh, int sbytes_per_pixel, bool bUseShadow, bool bUseTransparency)
{
	unsigned int *isrc = (unsigned int *)src;
	[[maybe_unused]] unsigned int pixel;
	unsigned int msk;
	int r,g,b;
	UNREFERENCED_PARAMETER(sh);
	UNREFERENCED_PARAMETER(bUseShadow);
	UNREFERENCED_PARAMETER(bUseTransparency);

	assert(sbytes_per_pixel == 4);
	assert(_bytes_per_pixel == 4);

	// Perform some clipping
	if(dx < _clip.x) 
	{ 
		sx += (_clip.x-dx);
		dw -= (_clip.x-dx);
		dx = _clip.x;
	}
	else if(dx >= (_clip.x+_clip.w))
	{
		return;
	}
	
	if((dx+dw) > (_clip.x+_clip.w))
	{
		dw = (_clip.x+_clip.w) - dx;
	}

	if(dy < _clip.y)
	{
		sy += (_clip.y-dy);
		dh -= (_clip.y-dy);
		dy = _clip.y;
	}
	else if(dy >= (_clip.y+_clip.h))
	{
		return;
	}

	if((dy+dh) > (_clip.y+_clip.h))
	{
		dh = (_clip.y+_clip.h) - dy;
	}

	for(int j = 0; j < dh; ++j) {
		for(int i = 0; i < dw; ++i) {
			// Look at all the masks
			pixel = isrc[(sy+j)*sw + sx+i];
			msk  = (isrc[(sy+j)*sw + sx+i]) & 0xFFFFFF;

			r = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 2];
			g =	src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 1];
			b = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 0];
			
			if(MASK_TRANSPARENT == msk)
			{
				continue;
			}
			else if(MASK_SHADOW == msk) 
			{
				(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] >>= 2) *= 3;
				(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] >>= 2) *= 3;
				(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] >>= 2) *= 3;
			} 
			else
			{
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			}
		}
	}
}

void 
Screen::Blit(unsigned char *src, unsigned char *mask, 
			 int dx, int dy, int dw, int dh, 
			 int sx, int sy, int sw, int sh, 
			 bool bHilit, Color *hilitColor, int sbytes_per_pixel, int modifierIdx)
{
	unsigned int *isrc = (unsigned int *)src;
	unsigned int *imask = (unsigned int *) mask;
	[[maybe_unused]] unsigned int pixel;
	unsigned int msk;
	int r,g,b;
	UNREFERENCED_PARAMETER(sh);

	assert(sbytes_per_pixel == 4);
	assert(_bytes_per_pixel == 4);

	for(int j = 0; j < dh; ++j) {
		for(int i = 0; i < dw; ++i) {
			// Look at all the masks
			pixel = isrc[(sy+j)*sw + sx+i];
			msk = (imask[(sy+j)*sw + sx+i]) & 0xFFFFFF;

			r = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 2];
			g =	src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 1];
			b = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 0];
			
			if(MASK_SHADOW == msk) {
				(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] >>= 2) *= 3;
				(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] >>= 2) *= 3;
				(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] >>= 2) *= 3;
			} else if(MASK_SHADOW_EDGE == msk) {
				if(bHilit) {
					_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = hilitColor->red;
					_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = hilitColor->green;
					_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = hilitColor->blue;
				} else {
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] >>= 2) *= 3;
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] >>= 2) *= 3;
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] >>= 2) *= 3;
				}
			} else if(MASK_EDGE == msk && bHilit) {
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = hilitColor->red;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = hilitColor->green;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = hilitColor->blue;
			} else if(MASK_BODY == msk) {
				r += g_ColorModifiers[modifierIdx].Body.Red;
				g += g_ColorModifiers[modifierIdx].Body.Green;
				b += g_ColorModifiers[modifierIdx].Body.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_LEGS == msk) {
				r += g_ColorModifiers[modifierIdx].Legs.Red;
				g += g_ColorModifiers[modifierIdx].Legs.Green;
				b += g_ColorModifiers[modifierIdx].Legs.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_HEAD == msk) {
				r += g_ColorModifiers[modifierIdx].Head.Red;
				g += g_ColorModifiers[modifierIdx].Head.Green;
				b += g_ColorModifiers[modifierIdx].Head.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_BELT == msk) {
				r += g_ColorModifiers[modifierIdx].Belt.Red;
				g += g_ColorModifiers[modifierIdx].Belt.Green;
				b += g_ColorModifiers[modifierIdx].Belt.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_BOOTS == msk) {
				r += g_ColorModifiers[modifierIdx].Boots.Red;
				g += g_ColorModifiers[modifierIdx].Boots.Green;
				b += g_ColorModifiers[modifierIdx].Boots.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_WEAPON == msk) {
				r += g_ColorModifiers[modifierIdx].Weapon.Red;
				g += g_ColorModifiers[modifierIdx].Weapon.Green;
				b += g_ColorModifiers[modifierIdx].Weapon.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_TRANSPARENT != msk && MASK_EDGE != msk) {
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			}
		}
	}
}

bool
Screen::PointInRegion(int x, int y, Region *r)
{
	int i, j=0;
	bool oddNodes=false;
	int polySides = 4;

	for(i=0; i<polySides; i++) {
		j++; if (j==polySides) j=0;
		if ((r->points[i].y < y && r->points[j].y >= y) 
			|| (r->points[j].y < y && r->points[i].y >= y))
		{
			if (r->points[i].x + (y-r->points[i].y)/(r->points[j].y-r->points[i].y)*(r->points[j].x-r->points[i].x) < x) 
			{
				oddNodes=!oddNodes;
			}
		}
	}
	return oddNodes;
}

void 
Screen::DrawLine(int startX, int startY, int endX, int endY, int width, Color *c)
{
	// Simple Bresenham's line algorithm for software rendering
	int x0 = startX;
	int y0 = startY;
	int x1 = endX;
	int y1 = endY;
	
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;
	
	// Draw the main line
	while (true) {
		// Draw thick line by drawing a small rectangle at each point
		if (width > 1) {
			int halfWidth = width / 2;
			for (int wy = -halfWidth; wy <= halfWidth; wy++) {
				for (int wx = -halfWidth; wx <= halfWidth; wx++) {
					int px = x0 + wx;
					int py = y0 + wy;
					if (px >= _clip.x && px < _clip.x + _clip.w && 
					    py >= _clip.y && py < _clip.y + _clip.h) {
						_bits[py*_pitch + px*_bytes_per_pixel + 0] = c->blue;
						_bits[py*_pitch + px*_bytes_per_pixel + 1] = c->green;
						_bits[py*_pitch + px*_bytes_per_pixel + 2] = c->red;
					}
				}
			}
		} else {
			// Single pixel (respecting clipping)
			if (x0 >= _clip.x && x0 < _clip.x + _clip.w && 
			    y0 >= _clip.y && y0 < _clip.y + _clip.h) {
				_bits[y0*_pitch + x0*_bytes_per_pixel + 0] = c->blue;
				_bits[y0*_pitch + x0*_bytes_per_pixel + 1] = c->green;
				_bits[y0*_pitch + x0*_bytes_per_pixel + 2] = c->red;
			}
		}
		
		if (x0 == x1 && y0 == y1) break;
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
}

void
Screen::DrawRect(int x, int y, int w, int h, int width, Color *c)
{
	DrawLine(x, y, x+w, y, width, c);
	DrawLine(x+w, y, x+w, y+h, width, c);
	DrawLine(x+w, y+h, x, y+h, width, c);
	DrawLine(x, y+h, x, y, width, c);
}

void
Screen::FillRect(int x, int y, int w, int h, Color *c)
{
	// Bounds checking - skip if completely off-screen
	if (x >= _width || y >= _height || x + w <= 0 || y + h <= 0)
		return;

	// Clamp to screen bounds
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x + w > _width)
		w = _width - x;
	if (y + h > _height)
		h = _height - y;

	for(int j = 0; j < h; ++j) {
		for(int i = 0; i < w; ++i) {
			_bits[(j+y)*_pitch+(i+x)*_bytes_per_pixel + 0] = c->blue;
			_bits[(j+y)*_pitch+(i+x)*_bytes_per_pixel + 1] = c->green;
			_bits[(j+y)*_pitch+(i+x)*_bytes_per_pixel + 2] = c->red;
		}
	}
}

bool
Screen::PointInRegion(int x, int y, int rx, int ry, int rw, int rh)
{
	Region r;
	r.points[0].x = rx;		r.points[0].y = ry;
	r.points[1].x = rx+rw;	r.points[1].y = ry;
	r.points[2].x = rx+rw;	r.points[2].y = ry+rh;
	r.points[3].x = rx;		r.points[3].y = ry+rh;
	return PointInRegion(x,y,&r);
}

bool
Screen::PointInRegion(int x, int y, std::vector<Point> *points)
{
	bool c = false;
	size_t i, j;
    for (i = 0, j = points->size()-1; i < points->size(); j = i++) {
		if (((((*points)[i].y <= y) && (y < (*points)[j].y)) ||
             (((*points)[j].y <= y) && (y < (*points)[i].y))) &&
            (x < ((*points)[j].x - (*points)[i].x) * (y - (*points)[i].y) / ((*points)[j].y - (*points)[i].y) + (*points)[i].x))
		{
			c = !c;
		}
	}
    return c;
}

void
Screen::BlitSurface(SDL_Surface *src, int dx, int dy)
{
	if(src == nullptr || _bits == nullptr) {
		return;
	}

	// Get source surface dimensions
	int sw = src->w;
	int sh = src->h;

	// Clip to screen bounds
	int startX = dx;
	int startY = dy;
	int endX = dx + sw;
	int endY = dy + sh;

	if(startX < _clip.x) startX = _clip.x;
	if(startY < _clip.y) startY = _clip.y;
	if(endX > _clip.x + _clip.w) endX = _clip.x + _clip.w;
	if(endY > _clip.y + _clip.h) endY = _clip.y + _clip.h;

	if(startX >= endX || startY >= endY) {
		return;
	}

	// Lock source surface if needed
	if(SDL_MUSTLOCK(src)) {
		SDL_LockSurface(src);
	}

	// Get source pixel data
	unsigned char *srcPixels = (unsigned char*)src->pixels;
	int srcPitch = src->pitch;
	int srcBytesPerPixel = src->format->BytesPerPixel;

	// Blit pixels
	for(int y = startY; y < endY; ++y) {
		for(int x = startX; x < endX; ++x) {
			int srcX = x - dx;
			int srcY = y - dy;

			unsigned char *srcPixel = srcPixels + srcY * srcPitch + srcX * srcBytesPerPixel;
			unsigned char *destPixel = _bits + y * _pitch + x * _bytes_per_pixel;

			if(srcBytesPerPixel == 4) {
				// Source has alpha
				unsigned char alpha = srcPixel[3];
				if(alpha == 255) {
					// Fully opaque - copy directly
					destPixel[0] = srcPixel[0];
					destPixel[1] = srcPixel[1];
					destPixel[2] = srcPixel[2];
				} else if(alpha > 0) {
					// Semi-transparent - blend
					unsigned char invAlpha = 255 - alpha;
					destPixel[0] = (srcPixel[0] * alpha + destPixel[0] * invAlpha) / 255;
					destPixel[1] = (srcPixel[1] * alpha + destPixel[1] * invAlpha) / 255;
					destPixel[2] = (srcPixel[2] * alpha + destPixel[2] * invAlpha) / 255;
				}
			} else if(srcBytesPerPixel == 3) {
				// No alpha - copy directly
				destPixel[0] = srcPixel[0];
				destPixel[1] = srcPixel[1];
				destPixel[2] = srcPixel[2];
			} else if(srcBytesPerPixel == 1) {
				// Indexed color - map through palette
				SDL_Color color = src->format->palette->colors[srcPixel[0]];
				destPixel[0] = color.b;
				destPixel[1] = color.g;
				destPixel[2] = color.r;
			}
		}
	}

	// Unlock source surface if needed
	if(SDL_MUSTLOCK(src)) {
		SDL_UnlockSurface(src);
	}
}

bool
Screen::SelfTest()
{
	// Let's test the PointInRegion functionality.
	// First, let's make a rhomboid type thing
	std::vector<Point> points;
	points.push_back(Point(0,0));
	points.push_back(Point(100,0));
	points.push_back(Point(200, 100));
	points.push_back(Point(100, 100));
	assert(!PointInRegion(0,50,&points));
	assert(PointInRegion(100,50,&points));
	return true;
}

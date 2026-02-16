#include "./MiniMap.h"
#include <misc/Color.h>
#include <objects/Object.h>
#include <world/World.h>
#include <graphics/Screen.h>
#include <application/Globals.h>
#include <filesystem>

MiniMap::MiniMap(void)
{
	_zoomWidth = -1;
	_zoomHeight = -1;
	_x = 0;
	_y = 0;
	_visibleWidth = -1;
	_visibleHeight = -1;
}

MiniMap::~MiniMap(void)
{
}

void
MiniMap::Render(Screen *screen)
{
	Color white(255,255,255);
	Color black(0,0,0);

	screen->DrawRect(Position.x, Position.y, _tga->GetWidth()+4, _tga->GetHeight()+4, 1, &black);
	screen->DrawRect(Position.x+1, Position.y+1, _tga->GetWidth()+2, _tga->GetHeight()+2, 1, &white);
	screen->Blit(_tga->GetData(), Position.x+2, Position.y+2, _tga->GetWidth(), _tga->GetHeight(), _tga->GetWidth(), _tga->GetHeight(), _tga->GetDepth());

	// We need to draw all of the victory locations
	int x=0,y=0;
	Nationality *nationality = nullptr;
	int numVictoryLocations = g_Globals->World.CurrentWorld->GetNumVictoryLocations();
	for(int i = 0; i < numVictoryLocations; ++i)
	{
		g_Globals->World.CurrentWorld->GetVictoryLocation(i, &x, &y, &nationality);
		if(nationality != nullptr && nationality->MiniMap != nullptr)
		{
			float xpct = ((float) x / (float) _parentWorld->GetWidth());
			float ypct = ((float) y / (float) _parentWorld->GetHeight());
			Color white(255,255,255);
			TGA *tga = nationality->MiniMap;
			x = Position.x + 2 + (int)(xpct*(float)_tga->GetWidth()) - (tga->GetWidth()>>1);
			y = Position.y + 2 + (int)(ypct*(float)_tga->GetHeight()) - (tga->GetHeight()>>1);
			screen->Blit(tga->GetData(), x, y, tga->GetWidth(), tga->GetHeight(), tga->GetWidth(), tga->GetHeight(), tga->GetDepth(), &white);
			
			// Skip text on minimap to avoid clutter and corruption
			// Victory location names are shown on main map only
		}
	}

	// We need to mark off the position of all the moving objects
	// in the world on the mini map. We are going to mark objects
	// of the current player as blue, allied objects in green,
	// and enemy objects in red.
	Color blue(0,0,255);
	auto& objs = g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Objects;
	for(size_t i = 0; i < objs.size(); ++i) {
		// Draw a small circle
		float xpct = ((float) objs[i]->Position.x / (float) _parentWorld->GetWidth());
		float ypct = ((float) objs[i]->Position.y / (float) _parentWorld->GetHeight());
		screen->FillRect(Position.x + 2 + (int)(xpct*(float)_tga->GetWidth()), Position.y + 2 + (int)(ypct*(float)_tga->GetHeight()), 5, 5, &blue);
	}

	// Now allied objects
	Color green(0,255,0);
	for(size_t j = 0; j < g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Allies.size(); ++j)
	{
		PlayerID id = g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Allies[j];
		auto& teamObjs = g_Globals->World.Teams[id].Objects;
		for(size_t i = 0; i < teamObjs.size(); ++i) {
			// Draw a small circle
			float xpct = ((float) teamObjs[i]->Position.x / (float) _parentWorld->GetWidth());
			float ypct = ((float) teamObjs[i]->Position.y / (float) _parentWorld->GetHeight());
			screen->FillRect(Position.x + 2 + (int)(xpct*(float)_tga->GetWidth()), Position.y + 2 + (int)(ypct*(float)_tga->GetHeight()), 5, 5, &green);
		}
	}

	// Now enemy objects
	Color red(255,0,0);
	for(size_t j = 0; j < g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Enemies.size(); ++j)
	{
		PlayerID id = g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Enemies[j];
		auto& teamObjs = g_Globals->World.Teams[id].Objects;
		for(size_t i = 0; i < teamObjs.size(); ++i) {
			// Draw a small circle
			float xpct = ((float) teamObjs[i]->Position.x / (float) _parentWorld->GetWidth());
			float ypct = ((float) teamObjs[i]->Position.y / (float) _parentWorld->GetHeight());
			screen->FillRect(Position.x + 2 + (int)(xpct*(float)_tga->GetWidth()), Position.y + 2 + (int)(ypct*(float)_tga->GetHeight()), 5, 5, &red);
		}
	}

	// Now we need to draw the yellow line. Calculate size and position every frame
	// Use visible area if set, otherwise fall back to screen dimensions
	int viewWidth = (_visibleWidth > 0) ? _visibleWidth : screen->GetWidth();
	int viewHeight = (_visibleHeight > 0) ? _visibleHeight : screen->GetHeight();
	
	// Recalculate zoom extents if visible area changed
	if(_zoomWidth < 0 || _calcWidth != viewWidth || _calcHeight != viewHeight) {
		_widthPct = (float) viewWidth / (float) _parentWorld->GetWidth();
		_heightPct = (float) viewHeight / (float) _parentWorld->GetHeight();
		_calcWidth = viewWidth;
		_calcHeight = viewHeight;
		_zoomWidth = (int)(_widthPct * (float) _tga->GetWidth());
		_zoomHeight = (int)(_heightPct * (float) _tga->GetHeight());
	}
	
	// Calculate yellow rectangle position based on current world origin
	int originX, originY;
	_parentWorld->GetOrigin(&originX, &originY);
	
	// When at maximum scroll, force rectangle to edge to avoid precision gaps
	if(originX + _calcWidth >= _parentWorld->GetWidth()) {
		_x = _tga->GetWidth() - _zoomWidth;
	} else {
		_x = (int)(((float) originX / (float) _parentWorld->GetWidth()) * (float) _tga->GetWidth());
	}
	
	if(originY + _calcHeight >= _parentWorld->GetHeight()) {
		_y = _tga->GetHeight() - _zoomHeight;
	} else {
		_y = (int)(((float) originY / (float) _parentWorld->GetHeight()) * (float) _tga->GetHeight());
	}
	
	// Clamp to minimap bounds (safety check)
	if(_x < 0) _x = 0;
	if(_y < 0) _y = 0;
	
	// Draw the yellow rectangle
	Color yellow(255,255,0);
	screen->DrawRect(Position.x+2+_x, Position.y+2+_y, _zoomWidth, _zoomHeight, 1, &yellow);
}

MiniMap *
MiniMap::Create(const std::filesystem::path& fileName, World *parentWorld)
{
	MiniMap *mm = new MiniMap();
	mm->_tga = TGA::Create(fileName);
	mm->_parentWorld = parentWorld;
	return mm;
}

bool
MiniMap::Contains(int x, int y) 
{
	Region r;
	r.points[0].x = Position.x+2; r.points[0].y = Position.y+2;
	r.points[1].x = Position.x+2+_tga->GetWidth(); r.points[1].y = Position.y+2;
	r.points[2].x = Position.x+2+_tga->GetWidth(); r.points[2].y = Position.y+2+_tga->GetHeight();
	r.points[3].x = Position.x+2; r.points[3].y = Position.y+2+_tga->GetHeight();
	return Screen::PointInRegion(x, y, &r);
}

void
MiniMap::LeftMouseDown(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void 
MiniMap::LeftMouseUp(int x, int y)
{
	_x = x - (Position.x+2) - _zoomWidth/2;
	_y = y - (Position.y+2) - _zoomHeight/2;

	if(_x < 0) { _x = 0; }
	if(_y < 0) { _y = 0; }
	if(_x > (_tga->GetWidth()-_zoomWidth)) { _x = _tga->GetWidth() - _zoomWidth; }
	if(_y > (_tga->GetHeight()-_zoomHeight)) { _y = _tga->GetHeight() - _zoomHeight; }
}

void
MiniMap::LeftMouseDrag(int x, int y)
{
	_x = x - (Position.x+2) - _zoomWidth/2;
	_y = y - (Position.y+2) - _zoomHeight/2;

	if(_x < 0) { _x = 0; }
	if(_y < 0) { _y = 0; }
	if(_x > (_tga->GetWidth()-_zoomWidth)) { _x = _tga->GetWidth() - _zoomWidth; }
	if(_y > (_tga->GetHeight()-_zoomHeight)) { _y = _tga->GetHeight() - _zoomHeight; }

	// Now we need to update where the world is!
	int ox = (int)((float)_parentWorld->GetWidth() * ((float)_x/(float)_tga->GetWidth()));
	int oy = (int)((float)_parentWorld->GetHeight() * ((float)_y/(float)_tga->GetHeight()));
	if((ox+_calcWidth) >= _parentWorld->GetWidth()) {
		ox = _parentWorld->GetWidth() - _calcWidth;
	}
	if((oy+_calcHeight) >= _parentWorld->GetHeight()) {
		oy = _parentWorld->GetHeight() - _calcHeight;
	}

	_parentWorld->SetOrigin(ox, oy);
}

void
MiniMap::Update()
{
	// Get our new origin
	int x=0,y=0;
	_parentWorld->GetOrigin(&x, &y);

	// Now, where is our rectangle going to go? It is based on a percentage
	// of our extents and our origin
	float xp = ((float)x) / ((float)_parentWorld->GetWidth());
	float yp = ((float)y) / ((float)_parentWorld->GetHeight());
	_x = (int)(xp*_tga->GetWidth());
	_y = (int)(yp*_tga->GetHeight());
}

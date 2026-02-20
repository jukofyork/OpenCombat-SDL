#include "./World.h"
#include <assert.h>
#include <string>
#include <filesystem>
#include <application/CursorInterface.h>
#include <graphics/Effect.h>
#include <graphics/Mark.h>
#include <world/Map.h>
#include <objects/Object.h>
#include <orders/Orders.h>
#include <graphics/Screen.h>
#include <objects/Squad.h>
#include <objects/Soldier.h>
#include <misc/Structs.h>
#include <graphics/AnimationManager.h>
#include <graphics/ColorManager.h>
#include <world/ElementManager.h>
#include <world/LineOfSight.h>
#include <graphics/Widget.h>
#include <misc/Utilities.h>
#include <objects/Vehicle.h>
#include <application/Globals.h>
#include <world/MiniMap.h>
#include <misc/GameConstants.h>

World::World(void)
{
	_currentHeadingArc = North;
	_contextMenu = new CombatContextMenu();
	State.NumSquads = 0;
	State.SelectedSquad = -1;
	_markPoints.reserve(8);
	_markColors.reserve(8);
	_screenWidth = 0;
	_screenHeight = 0;
	_scrollLeft = false;
	_scrollRight = false;
	_scrollUp = false;
	_scrollDown = false;
	_scrollRepeating = false;
	_scrollTimer = 0;
	_middleDragActive = false;
	_middleDragLastX = 0;
	_middleDragLastY = 0;
}

World::~World(void)
{
	// Free context menu
	if (_contextMenu) {
		delete _contextMenu;
		_contextMenu = nullptr;
	}

	// Free line of sight calculator
	if (_lineOfSight) {
		delete _lineOfSight;
		_lineOfSight = nullptr;
	}

	// Free current map
	if (_currentMap) {
		delete _currentMap;
		_currentMap = nullptr;
	}

	// Clean up effects
	for (auto* effect : _effects) {
		delete effect;
	}
	_effects.clear();

	// Note: _mobileObjects, _staticObjects, _selectedObjects are managed by their respective managers
	// and should not be deleted here
}

void
World::Render(Screen *screen, Rect *clip)
{
	Color white(255,255,255);
	Color black(0,0,0);
	
	// Copy some parameters
	_screenWidth = clip->w;
	_screenHeight = clip->h;

	if(_originX < 0)
	{
		_originX = 0;
	}
	else if((_originX+_screenWidth) > _currentMap->GetWidth())
	{
		_originX = _currentMap->GetWidth()-_screenWidth;
	}

	if(_originY < 0)
	{
		_originY = 0;
	}
	else if((_originY+_screenHeight) > _currentMap->GetHeight())
	{
		_originY = _currentMap->GetHeight()-_screenHeight;
	}

	screen->SetOrigin(_originX, _originY);

	// Clear all of our marks
	ClearMarks();

	// Render the map against the clipping rectangle
	_currentMap->SetOrigin(_originX, _originY);
	_currentMap->Render(screen, clip);

	// Render all of the static objects that are in the clipping region
	for(size_t i = 0; i < _staticObjects.size(); ++i) {
		Object *o = _staticObjects[i];
		if(o->Position.x > (clip->x+_originX) && o->Position.x < (clip->x+_originX+clip->w)
			&& o->Position.y > (clip->y+_originY) && o->Position.y < (clip->y+_originY+clip->h))
		{
			o->Render(screen, clip);
		}
	}

	// Render all of the mobile objects that are in the clipping region
	_rangerSelectedObject = nullptr;
	for(size_t i = 0; i < _mobileObjects.size(); ++i) {
		Object *o = _mobileObjects[i];
		if(o->Position.x > (clip->x+_originX) && o->Position.x < (clip->x+_originX+clip->w)
			&& o->Position.y > (clip->y+_originY) && o->Position.y < (clip->y+_originY+clip->h))
		{
			// Is our ranger over another squad? If it is, then let's highlight that
			// squad
			if(_currentState == ContextSelected 
				&& o->Contains(screen->GetCursorX()+_originX, screen->GetCursorY()+_originY))
			{
				bool oldSelect = o->IsSelected();
				o->Select(true);
				o->Render(screen, clip);
				o->Select(oldSelect);
				_rangerSelectedObject = o;
			} else {
				o->Render(screen, clip);
			}
		}
	}

	// Render all of the elements (trees, etc.)
	if(g_Globals->World.bRenderElements) {
		_currentMap->RenderElements(screen, clip);
	}

	// Render victory location text on top of elements but below UI
	_currentMap->RenderVictoryLocationText(screen, clip);

	// Render any of the marks
	int x, y;
	for(size_t i = 0; i < _markPoints.size(); ++i) {
		x = _markPoints[i].x;
		y = _markPoints[i].y;
		if(x > (clip->x+_originX) && x < (clip->x+_originX+clip->w)
			&& y > (clip->y+_originY) && y < (clip->y+_originY+clip->h))
		{
			g_Globals->World.Marks->Render(screen, _markColors[i], x-_originX, y-_originY);
		}
	}

	// Render all of the effects
	for(size_t i = 0; i < _effects.size(); ++i) {
		Effect *o = _effects[i];
		if(o->Position.x > (clip->x+_originX) && o->Position.x < (clip->x+_originX+clip->w)
			&& o->Position.y > (clip->y+_originY) && o->Position.y < (clip->y+_originY+clip->h))
		{
			o->Render(screen);
		}
	}

	if(g_Globals->World.bWeaponFan && _selectedObjects.size() > 0) {
		// Show the weapon fan. This is really time consuming!!!
		int sx, sy;
		
		// Check if we have a squad selected and use the selected soldier's position
		if(_selectedObjects[0]->GetType() == Target::Squad && State.SelectedSquad >= 0) {
			Squad* squad = (Squad*)_selectedObjects[0];
			int selectedIdx = State.SquadStates[State.SelectedSquad].SelectedSoldierIdx;
			std::vector<Soldier*>* soldiers = squad->GetSoldiers();
			if(selectedIdx >= 0 && selectedIdx < static_cast<int>(soldiers->size())) {
				sx = (*soldiers)[selectedIdx]->Position.x/10;
				sy = (*soldiers)[selectedIdx]->Position.y/10;
			} else {
				sx = _selectedObjects[0]->Position.x/10;
				sy = _selectedObjects[0]->Position.y/10;
			}
		} else {
			sx = _selectedObjects[0]->Position.x/10;
			sy = _selectedObjects[0]->Position.y/10;
		}
		int ox, oy, oz;

		int m=0,n=0;
		for(int j = (clip->y+_originY)/10; j < (clip->y+clip->h+_originY)/10; ++j) 
		{
			m = 0;
			for(int i = (clip->x+_originX)/10; i < (clip->x+clip->w+_originX)/10; ++i) 
			{
				if(!_lineOfSight->CalculateLOSForTile(sx, sy, i, j, &ox, &oy, &oz, _currentMap)) {
					// Draw checkered pattern so terrain is still visible underneath
					// Checkerboard at pixel level: every other pixel
					for(int py = 0; py < 10; ++py) {
						for(int px = 0; px < 10; ++px) {
							if((px + py) % 2 == 0) {
								screen->FillRect(m*10 + px, n*10 + py, 1, 1, &black);
							}
						}
					}
				}
				++m;
			}
			++n;
		}
	}

	if(_currentState == Ambushing)
	{
		assert(_selectedObjects.size() > 0);
		
		// Find out which direction we need to show
		Direction dir = Utilities::FindHeading(_selectedObjects[0]->Position.x - _originX, _selectedObjects[0]->Position.y-_originY, screen->GetCursorX(), screen->GetCursorY());
		_currentHeadingArc = dir;
		Widget *w = nullptr;
		switch(dir) {
			case North:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle North");
				break;
			case NorthEast:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle NorthEast");
				break;
			case East:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle East");
				break;
			case SouthEast:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle SouthEast");
				break;
			case South:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle South");
				break;
			case SouthWest:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle SouthWest");
				break;
			case West:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle West");
				break;
			case NorthWest:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle NorthWest");
				break;
			default:
				assert(false);
				break;
		}
		assert(w != nullptr);
		// Center the direction circle on the unit (subtract half width/height)
		w->Render(screen, _selectedObjects[0]->Position.x-_originX-w->GetWidth()/2, _selectedObjects[0]->Position.y-_originY-w->GetHeight()/2, w->GetWidth(), w->GetHeight(), true);
		delete w;
	}

	if(_currentState == Defending)
	{
		assert(_selectedObjects.size() > 0);
		
		// Find out which direction we need to show
		Direction dir = Utilities::FindHeading(_selectedObjects[0]->Position.x - _originX, _selectedObjects[0]->Position.y-_originY, screen->GetCursorX(), screen->GetCursorY());
		_currentHeadingArc = dir;
		Widget *w = nullptr;
		switch(dir) {
			case North:
				w = g_Globals->World.Icons->GetWidget("Defend Circle North");
				break;
			case NorthEast:
				w = g_Globals->World.Icons->GetWidget("Defend Circle NorthEast");
				break;
			case East:
				w = g_Globals->World.Icons->GetWidget("Defend Circle East");
				break;
			case SouthEast:
				w = g_Globals->World.Icons->GetWidget("Defend Circle SouthEast");
				break;
			case South:
				w = g_Globals->World.Icons->GetWidget("Defend Circle South");
				break;
			case SouthWest:
				w = g_Globals->World.Icons->GetWidget("Defend Circle SouthWest");
				break;
			case West:
				w = g_Globals->World.Icons->GetWidget("Defend Circle West");
				break;
			case NorthWest:
				w = g_Globals->World.Icons->GetWidget("Defend Circle NorthWest");
				break;
			default:
				assert(false);
				break;
		}
		assert(w != nullptr);
		// Center the direction circle on the unit (subtract half width/height)
		w->Render(screen, _selectedObjects[0]->Position.x-_originX-w->GetWidth()/2, _selectedObjects[0]->Position.y-_originY-w->GetHeight()/2, w->GetWidth(), w->GetHeight(), true);
		delete w;
	}

	if(_contextMenu->IsShowing()) {
		_contextMenu->Render(screen, screen->GetCursorX(), screen->GetCursorY(), clip);
	}

	if(_currentState == ContextSelected) {
		assert(_selectedObjects.size() > 0);
		_rangerX = _selectedObjects[0]->Position.x-_originX;
		_rangerY = _selectedObjects[0]->Position.y-_originY;

		if(_currentChoice == CombatContextMenu::ContextMenuChoice::Fire) {
			int ox, oy, oz;
			int x0 = _selectedObjects[0]->Position.x/10;
			int y0 = _selectedObjects[0]->Position.y/10;
			int x1 = (screen->GetCursorX()+_originX)/10;
			int y1 = (screen->GetCursorY()+_originY)/10;
			Color c;
			bool hasLOS = _lineOfSight->CalculateLOSForTile(x0, y0, x1, y1, &ox, &oy, &oz, _currentMap);
			
			// Update cursor based on hit chance
			UpdateFireCursor(screen->GetCursorX(), screen->GetCursorY(), _rangerSelectedObject != nullptr);
			
			if(hasLOS)
			{
				c.red = 0;
				c.blue = 0;
				c.green = 255;
				screen->DrawLine(_rangerX, _rangerY, screen->GetCursorX(), screen->GetCursorY(), 3, &c);
			Vector2 v;
			v.x = static_cast<float>(screen->GetCursorX()-_rangerX);
			v.y = static_cast<float>(screen->GetCursorY()- _rangerX);
			std::string msg = std::to_string(static_cast<int>(v.Magnitude())/g_Globals->World.Constants.PixelsPerMeter) + "m";
			// Position text just outside SE corner of 32x32 cursor
			// Cursor is 32x32 centered on hotspot, so corners are at +/-16
			// Text goes at (16+2, 16+2) = (18, 18) from center for small margin
			const int textOffset = 10;
			g_Globals->World.Fonts->Render(screen, msg, screen->GetCursorX() + textOffset, screen->GetCursorY() + textOffset, &white);
		} else {
			// Find the distance to the blocked element
			Vector2 vb;
			vb.x = static_cast<float>((ox*10+5)-x0*10);
			vb.y = static_cast<float>((oy*10+5)-y0*10);
			float bdist = vb.Magnitude();
			// Find the total distance
			Vector2 vt;
			vt.x = static_cast<float>(x1*10-x0*10);
			vt.y = static_cast<float>(y1*10-y0*10);
			float tdist = vt.Magnitude();
			int bx = static_cast<int>(_rangerX + (screen->GetCursorX() - _rangerX)*bdist/tdist);
			int by = static_cast<int>(_rangerY + (screen->GetCursorY() - _rangerY)*bdist/tdist);

			Vector2 v;
			v.x = static_cast<float>(bx-_rangerX);
			v.y = static_cast<float>(by-_rangerY);
			std::string msg = std::to_string(static_cast<int>(v.Magnitude())/g_Globals->World.Constants.PixelsPerMeter) + "m";
			// Position text at SE corner of blocked line end
			const int textOffset = 10;
			g_Globals->World.Fonts->Render(screen, msg, bx + textOffset, by + textOffset, &white);
				c.red = 0;
				c.blue = 0;
				c.green = 255;
				screen->DrawLine(_rangerX, _rangerY, bx, by, 3, &c);
				v.x = static_cast<float>(screen->GetCursorX()-_rangerX);
				v.y = static_cast<float>(screen->GetCursorY()- _rangerX);
				msg = std::to_string(static_cast<int>(v.Magnitude())/g_Globals->World.Constants.PixelsPerMeter) + "m";
			g_Globals->World.Fonts->Render(screen, msg, screen->GetCursorX() + textOffset, screen->GetCursorY() + textOffset, &white);
				c.red = 255;
				c.blue = 0;
				c.green = 0;
				screen->DrawLine(bx, by, screen->GetCursorX(), screen->GetCursorY(), 3, &c);
			}
		} else {
			screen->DrawLine(_rangerX, _rangerY, screen->GetCursorX(), screen->GetCursorY(), 3, &_rangerColor);
		}
	}
}

void
World::Simulate(long dt)
{
	// Simulate all of the mobile objects
	for(auto* o : _mobileObjects) {
		o->Simulate(dt, this);
	}

	// Simulate all of the effects
	for(auto it = _effects.begin(); it != _effects.end(); ) {
		(*it)->Simulate(dt);
		if((*it)->IsCompleted()) {
			delete *it;
			it = _effects.erase(it);
		} else {
			++it;
		}
	}

	// Handle scroll key repeat
	bool isScrolling = _scrollLeft || _scrollRight || _scrollUp || _scrollDown;
	if(isScrolling) {
		_scrollTimer += dt;
		if(_scrollTimer >= SCROLL_INITIAL_DELAY_MS) {
			// Mark that we've started continuous scrolling
			_scrollRepeating = true;
			
			// Calculate scroll amount based on time delta for frame-rate independent movement
			int scrollAmount = (SCROLL_SPEED_PPS * dt) / 1000;
			int newX = _originX;
			int newY = _originY;

			if(_scrollLeft) {
				newX -= scrollAmount;
			}
			if(_scrollRight) {
				newX += scrollAmount;
			}
			if(_scrollUp) {
				newY -= scrollAmount;
			}
			if(_scrollDown) {
				newY += scrollAmount;
			}

			SetOrigin(newX, newY);
			_currentMiniMap->Update();
		}
	}

	UpdateState();
}

void
World::Load(const std::filesystem::path& fileName, SoldierManager *soldierManager, AnimationManager *animationManager)
{
	_soldierManager = soldierManager;
	_animationManager = animationManager;
	_currentState = Normal;
	_originX = 0;
	_originY = 0;

	// Initialize the context menu
	std::filesystem::path widgetsFile = g_Globals->Application.ConfigDirectory / "ContextMenuWidgets.xml";
	_contextMenu->Initialize(widgetsFile);

	// Create the element manager
	widgetsFile = g_Globals->Application.ConfigDirectory / "Elements.xml";
	_elementManager = new ElementManager();
	_elementManager->Load(widgetsFile);
	g_Globals->World.Elements = _elementManager;

	// Create the color manager
	widgetsFile = g_Globals->Application.ConfigDirectory / "Colors.xml";
	_colorManager = new ColorManager();
	_colorManager->Load(widgetsFile);

	// Create the effect manager
	_effectManager = new EffectManager();
	widgetsFile = g_Globals->Application.ConfigDirectory / "Effects.xml";
	_effectManager->LoadEffects(widgetsFile);
	g_Globals->World.Effects = _effectManager;

	// Create the weapon manager
	_weaponManager = new WeaponManager();
	widgetsFile = g_Globals->Application.ConfigDirectory / "Weapons.xml";
	_weaponManager->LoadWeapons(widgetsFile);
	g_Globals->World.Weapons = _weaponManager;

	// Create the vehicle manager
	// The vehicle manager needs to be created after the weapon manager!
	widgetsFile = g_Globals->Application.ConfigDirectory / "Vehicles.xml";
	_vehicleManager = new VehicleManager();
	_vehicleManager->Load(widgetsFile);
	g_Globals->World.Vehicles = _vehicleManager;

	// Create the squad manager
	widgetsFile = g_Globals->Application.ConfigDirectory / "Squads.xml";
	_squadManager = new SquadManager();
	_squadManager->LoadSquads(widgetsFile);
	g_Globals->World.Squads = _squadManager;

	// XXX/GWS: Load the current map. This is hard coded for now
	_currentMap = Map::Create(fileName);
	_currentMap->GetNumTiles(&(NumTiles.x), &(NumTiles.y));
	_currentMap->GetTileSize(&(TileSize.w), &(TileSize.h));

	// Create the line of sight calculator
	_lineOfSight = new LineOfSight();

	// XXX/GWS: The following is temporary, just to populate this world
	//			with some stuff
	std::vector<Soldier*> *soldiers;
	State.NumSquads = 0;
	for(int i = 0; i < 1; ++i) {
		Squad *s = _squadManager->CreateSquad("BAR Rifle", _soldierManager, _vehicleManager, _animationManager, _weaponManager);
		soldiers = s->GetSoldiers();
		for(size_t j = 0; j < soldiers->size(); ++j)
		{
			// Let's place this object on the map
			_currentMap->PlaceObject((*soldiers)[j], &(*soldiers)[j]->Position);
			(*soldiers)[j]->SetTeam(g_Globals->World.CurrentPlayer);
		}

		s->SetPosition((i+1)*200, 100);
		AddObject(s);
		s->SetTeam(g_Globals->World.CurrentPlayer);
		g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Objects.push_back(s);
	}

	Squad *squad = _squadManager->CreateSquad("Bazooka", _soldierManager, _vehicleManager, _animationManager, _weaponManager);
	soldiers = squad->GetSoldiers();
	for(size_t i = 0; i < soldiers->size(); ++i)
	{
		// Let's place this object on the map
		_currentMap->PlaceObject((*soldiers)[i], &(*soldiers)[i]->Position);
		(*soldiers)[i]->SetTeam(g_Globals->World.CurrentPlayer);
	}
	squad->SetPosition(748, 604);
	AddObject(squad);
	g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Objects.push_back(squad);
	squad->SetTeam(g_Globals->World.CurrentPlayer);

	squad = _squadManager->CreateSquad(".30 Cal MG", _soldierManager, _vehicleManager, _animationManager, _weaponManager);
	soldiers = squad->GetSoldiers();
	for(size_t i = 0; i < soldiers->size(); ++i)
	{
		// Let's place this object on the map
		_currentMap->PlaceObject((*soldiers)[i], &(*soldiers)[i]->Position);
		(*soldiers)[i]->SetTeam(g_Globals->World.CurrentPlayer);
	}
	squad->SetPosition(200, 200);
	AddObject(squad);
	g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Objects.push_back(squad);
	squad->SetTeam(g_Globals->World.CurrentPlayer);

	// Create a couple of enemy teams
	for(int i = 0; i < 1; ++i) {
		Squad *s = _squadManager->CreateSquad("BAR Rifle", _soldierManager, _vehicleManager, _animationManager, _weaponManager);
		soldiers = s->GetSoldiers();
		for(size_t j = 0; j < soldiers->size(); ++j)
		{
			// Let's place this object on the map
			_currentMap->PlaceObject((*soldiers)[j], &(*soldiers)[j]->Position);
			(*soldiers)[j]->SetTeam(1);
		}

		s->SetPosition((i)*100+1700, 1800);
		AddObject(s);
		s->SetTeam(1);
		g_Globals->World.Teams[1].Objects.push_back(s);
	}

	// And now an allied team
	for(int i = 0; i < 1; ++i) {
		Squad *s = _squadManager->CreateSquad("BAR Rifle", _soldierManager, _vehicleManager, _animationManager, _weaponManager);
		soldiers = s->GetSoldiers();
		for(size_t j = 0; j < soldiers->size(); ++j)
		{
			// Let's place this object on the map
			_currentMap->PlaceObject((*soldiers)[j], &(*soldiers)[j]->Position);
			(*soldiers)[j]->SetTeam(2);
		}

		s->SetPosition((i)*100+1800, 200);
		AddObject(s);
		s->SetTeam(2);
		g_Globals->World.Teams[2].Objects.push_back(s);
	}

	squad = _squadManager->CreateSquad("Panzer IVG", _soldierManager, _vehicleManager, _animationManager, _weaponManager);
	squad->SetPosition(300, 200);
	AddObject(squad);
}

void
World::UpdateState()
{
	State.NumSquads = static_cast<int>(g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Objects.size());
	for(size_t i = 0; i < g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Objects.size(); ++i) {
		Object *o = g_Globals->World.Teams[g_Globals->World.CurrentPlayer].Objects[i];
		o->UpdateInterfaceState(&State, static_cast<int>(i), 0);
	}

	// Update the state of any selected objects
	State.SelectedSquad = -1;
	if(_selectedObjects.size() > 0) {
		for(int i = 0; i < State.NumSquads; ++i) {
			if(_selectedObjects[0]->GetID() == State.SquadStates[i].ID) {
				State.SelectedSquad = i;
				break;
			}
		}
	}

	// Now check to see if we have any highlighted objects
	for(auto* o : _mobileObjects) {
		if(o->IsHighlighted()) {
			for(int j = 0; j < State.NumSquads; ++j) {
				if(o->GetID() == State.SquadStates[j].ID) {
					State.SelectedSquad = j;
					break;
				}
			}
			break;
		}
	}
}

void
World::AddObject(Object *o) 
{
	if(o->IsMobile()) {
		_mobileObjects.push_back(o);
	} else {
		_staticObjects.push_back(o);
	}
}

bool
World::TryMove(Object *o, int x, int y)
{
	UNREFERENCED_PARAMETER(o);
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	return true;
}

void 
World::LeftMouseDown(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void 
World::LeftMouseUp(int x, int y)
{
	if(_currentState == Normal) {
		// Select anything at this point
		Select(x, y);
	
		// Hide the context menu if we were showing it
		_contextMenu->Hide();
	} else if(_currentState == ContextSelecting) {
		_currentState = ContextSelected;

		// Now perform whatever action we need to do from the context menu
		_contextMenu->Hide();
		_currentChoice = _contextMenu->Choose(x, y);

		// Now act on this choice
		assert(_selectedObjects.size() > 0);
		_rangerX = _selectedObjects[0]->Position.x-_originX;
		_rangerY = _selectedObjects[0]->Position.y-_originY;
				
		switch(_currentChoice) {
			case CombatContextMenu::ContextMenuChoice::Move:
				_colorManager->CopyColor("RangerMove", &_rangerColor);
				g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::MarkBlue);
				break;
			case CombatContextMenu::ContextMenuChoice::MoveFast:
				_colorManager->CopyColor("RangerMoveFast", &_rangerColor);
				g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::MarkPurple);
				break;
			case CombatContextMenu::ContextMenuChoice::Fire:
				_colorManager->CopyColor("RangerFire", &_rangerColor);
				g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::MarkRed);
				break;
			case CombatContextMenu::ContextMenuChoice::Sneak:
				_colorManager->CopyColor("RangerSneak", &_rangerColor);
				g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::MarkYellow);
				break;
		case CombatContextMenu::ContextMenuChoice::Smoke:
			_colorManager->CopyColor("RangerSmoke", &_rangerColor);
			g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::MarkGrey);
			break;
			case CombatContextMenu::ContextMenuChoice::Defend:
				_currentState = Defending;
				break;
			case CombatContextMenu::ContextMenuChoice::Ambush:
				_currentState = Ambushing;
				break;
			default:
				// Default action is to cancel everything
				_currentState = Normal;
				break;
		}
	} else if(_currentState == ContextSelected) {
		// Show the cursor
		g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::Regular);


		// Our context selection is done, so finish it
		switch(_currentChoice) {
			case CombatContextMenu::ContextMenuChoice::Move:
				IssueOrder(new MoveOrder(x+_originX, y+_originY, Orders::Move));
				break;
			case CombatContextMenu::ContextMenuChoice::MoveFast:
				IssueOrder(new MoveOrder(x+_originX, y+_originY, Orders::MoveFast));
				break;
			case CombatContextMenu::ContextMenuChoice::Fire:
				if(_rangerSelectedObject != nullptr) {
					IssueOrder(new FireOrder(_rangerSelectedObject, _rangerSelectedObject->GetType()));
				} else {
					IssueOrder(new FireOrder(x+_originX, y+_originY));
				}
				break;
			case CombatContextMenu::ContextMenuChoice::Sneak:
				IssueOrder(new MoveOrder(x+_originX, y+_originY, Orders::Sneak));
				break;
			case CombatContextMenu::ContextMenuChoice::Smoke:
				break;
			case CombatContextMenu::ContextMenuChoice::Defend:
				assert(false);
				break;
			case CombatContextMenu::ContextMenuChoice::Ambush:
				assert(false);
				break;
			default:
				break;
		}
		_currentState = Normal;
	} else if(_currentState == Ambushing) {
		// Show the cursor
		g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::Regular);
		_currentState = Normal;
		IssueOrder(new AmbushOrder(_currentHeadingArc));
	} else if(_currentState == Defending) {
		// Show the cursor
		g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::Regular);
		_currentState = Normal;
		IssueOrder(new DefendOrder(_currentHeadingArc));
	}
}

void 
World::LeftMouseDrag(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void 
World::RightMouseDown(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void 
World::RightMouseUp(int x, int y)
{
	if(_currentState == ContextSelected || _currentState == Ambushing || _currentState == Defending) {
		_currentState = Normal;
		g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::Regular);
		return;
	} else if(_currentState == ContextSelecting) {
		_currentState = Normal;
		g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::Regular);
		// Hide the context menu if we were showing it
		_contextMenu->Hide();
		return;
	}

	// We need to make sure that this point is inside our selected objects
	if(_selectedObjects.size() <= 0) {
		// We have no selected items, so try to select something
		Select(x,y);
	} else {
		bool bContains = false;
		for(auto* o : _selectedObjects) {
			if(o->Contains(x+_originX, y+_originY)) {
				bContains = true;
			}
		}

		if(!bContains) {
			// We have some selected objects, but our right click was not
			// in any of their extents. So let's de-select everything, and try to re-select
			// whatever is underneath us
			ClearSelect(x, y);
			Select(x, y);
		}
	}

	// Show the context menu
	bool move=false, moveFast=false, sneak=false, fire=false, defend=false, ambush=false, smoke=false;
	bool bShow = false;
	for(auto* o : _selectedObjects) {
		bShow = true;
			
		if(o->CanMove()) {
			move = true;
		}
		if(o->CanMoveFast()) {
			moveFast = true;
		}
		if(o->CanFire()) {
			fire = true;
		}
		if(o->CanDefend()) {
			defend = true;
		}
		if(o->CanAmbush()) {
			ambush = true;
		}
		if(o->CanSmoke()) {
			smoke = true;
		}
		if(o->CanSneak()) {
			sneak = true;
		}
	}

	if(bShow) {
		_contextMenu->SetMove(move);
		_contextMenu->SetMoveFast(moveFast);
		_contextMenu->SetAmbush(ambush);
		_contextMenu->SetFire(fire);
		_contextMenu->SetDefend(defend);
		_contextMenu->SetSmoke(smoke);
		_contextMenu->SetSneak(sneak);
		_contextMenu->Show(x,y);
		_currentState = ContextSelecting;
	}
}

void 
World::RightMouseDrag(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void
World::Select(int x, int y)
{
	// Clear current selections
	ClearSelect(x+_originX,y+_originY);

	// Try selecting mobile objects first
	_currentMap->SelectObjects(x+_originX, y+_originY, &_selectedObjects);
	
	// Only check mobile objects directly if map selection didn't find anything
	if(_selectedObjects.empty()) {
		for(size_t i = 0; i < _mobileObjects.size(); ++i) {
			if(_mobileObjects[i]->Select(x+_originX,y+_originY)) {
				_selectedObjects.push_back(_mobileObjects[i]);
				break;
			}
		}
	}
}

void
World::ClearSelect(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	for(auto* o : _selectedObjects) {
		o->Select(false);
	}
	_selectedObjects.clear();
}

void
World::IssueOrder(Order *order)
{
	for(auto* o : _selectedObjects) {
		o->ClearOrders();
		o->AddOrder(order);
	}
}

constexpr int KEY_LEFT = 0x25;
constexpr int KEY_RIGHT = 0x27;
constexpr int KEY_UP = 0x26;
constexpr int KEY_DOWN = 0x28;
void
World::KeyUp(int key)
{
	// Let's kill one of our soldiers
	if(key == 'k' || key == 'K') {
		for(auto* o : _selectedObjects) {
			o->Kill();
		}
	}
	else if(key == 'f' || key == 'F')
	{
		// Change the formation of the currently selected object
		for(auto* o : _selectedObjects) {
			Formation::Type f = o->GetFormation();
			o->SetFormation((Formation::Type)((f+1)%Formation::NumFormations));
		}
	}
	else if(key == KEY_LEFT)
	{
		// Only do the jump if we weren't continuously scrolling
		if(!_scrollRepeating) {
			SetOrigin(_originX-KEY_MULTIPLIER_TILES*TileSize.w, _originY);
			_currentMiniMap->Update();
		}
	}
	else if(key == KEY_RIGHT)
	{
		// Only do the jump if we weren't continuously scrolling
		if(!_scrollRepeating) {
			SetOrigin(_originX+KEY_MULTIPLIER_TILES*TileSize.w, _originY);
			_currentMiniMap->Update();
		}
	}
	else if(key == KEY_UP)
	{
		// Only do the jump if we weren't continuously scrolling
		if(!_scrollRepeating) {
			SetOrigin(_originX, _originY-KEY_MULTIPLIER_TILES*TileSize.h);
			_currentMiniMap->Update();
		}
	}
	else if(key == KEY_DOWN)
	{
		// Only do the jump if we weren't continuously scrolling
		if(!_scrollRepeating) {
			SetOrigin(_originX, _originY+KEY_MULTIPLIER_TILES*TileSize.h);
			_currentMiniMap->Update();
		}
	}

	// Handle scroll key release
	if(key == KEY_LEFT) {
		_scrollLeft = false;
	}
	if(key == KEY_RIGHT) {
		_scrollRight = false;
	}
	if(key == KEY_UP) {
		_scrollUp = false;
	}
	if(key == KEY_DOWN) {
		_scrollDown = false;
	}

	// Reset scroll state if no scroll keys are held
	if(!_scrollLeft && !_scrollRight && !_scrollUp && !_scrollDown) {
		_scrollTimer = 0;
		_scrollRepeating = false;
	}
}

void
World::KeyDown(int key)
{
	// Handle scroll key press
	if(key == KEY_LEFT) {
		_scrollLeft = true;
	}
	if(key == KEY_RIGHT) {
		_scrollRight = true;
	}
	if(key == KEY_UP) {
		_scrollUp = true;
	}
	if(key == KEY_DOWN) {
		_scrollDown = true;
	}
}

void
World::MiddleMouseDown(int x, int y)
{
	_middleDragActive = true;
	_middleDragLastX = x;
	_middleDragLastY = y;
}

void
World::MiddleMouseUp(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	_middleDragActive = false;
}

void
World::MiddleMouseDrag(int x, int y)
{
	if(_middleDragActive) {
		// Calculate delta from last position
		int deltaX = _middleDragLastX - x;
		int deltaY = _middleDragLastY - y;

		// Scroll the view by the delta
		SetOrigin(_originX + deltaX, _originY + deltaY);
		_currentMiniMap->Update();

		// Update last position
		_middleDragLastX = x;
		_middleDragLastY = y;
	}
}

void
World::SetOrigin(int x, int y)
{
	_originX = x;
	_originY = y;
	
	if(_originX < 0)
	{
		_originX = 0;
	}
	else if((_originX+_screenWidth) > _currentMap->GetWidth())
	{
		_originX = _currentMap->GetWidth()-_screenWidth;
	}

	if(_originY < 0)
	{
		_originY = 0;
	}
	else if((_originY+_screenHeight) > _currentMap->GetHeight())
	{
		_originY = _currentMap->GetHeight()-_screenHeight;
	}
}

void
World::ConvertTileToPosition(int i, int j, int *x, int *y)
{
	int w=0,h=0;
	_currentMap->GetTileSize(&w, &h);
	*x = i*w+w/2;
	*y = j*h+h/2;
}

void 
World::ConvertPositionToTile(int x, int y, int *i, int *j)
{
	int w=0,h=0;
	_currentMap->GetTileSize(&w, &h);
	*i = x/w;
	*j = y/h;
}

// Add a mark to a given position
void
World::AddMark(Mark::Color markColor, int x, int y)
{
	_markPoints.push_back({x, y});
	_markColors.push_back(markColor);
}

// Clear all our marks
void
World::ClearMarks()
{
	_markPoints.clear();
	_markColors.clear();
}

void
World::MoveObject(Object *object, Point *from, Point *to)
{
	// Let's move this object on the map
	_currentMap->MoveObject(object, from, to);
}

// Calculates hit chance for firing (0-100%)
// Returns: -1 if no LOS, otherwise 0-100
int
World::CalculateHitChance(int shooterX, int shooterY, int targetX, int targetY, bool hasLOS)
{
	if(!hasLOS) {
		return -1;
	}
	
	// Calculate distance
	Vector2 v;
	v.x = static_cast<float>(targetX - shooterX);
	v.y = static_cast<float>(targetY - shooterY);
	float distance = v.Magnitude();
	float distanceMeters = distance / g_Globals->World.Constants.PixelsPerMeter;
	
	// Base hit chance - starts at 100% and decreases with distance
	// Using a simplified model:
	// - 0-50m: 100% base
	// - 50-100m: 80% base
	// - 100-200m: 60% base
	// - 200-300m: 40% base
	// - 300m+: 20% base
	int baseChance;
	if(distanceMeters <= 50) {
		baseChance = 100;
	} else if(distanceMeters <= 100) {
		baseChance = 80;
	} else if(distanceMeters <= 200) {
		baseChance = 60;
	} else if(distanceMeters <= 300) {
		baseChance = 40;
	} else {
		baseChance = 20;
	}
	
	// Apply modifiers based on target state (if we have a selected object)
	// This is simplified - in a full implementation we'd check:
	// - Target stance (standing, prone, etc.)
	// - Target cover
	// - Shooter skill
	// - Weapon accuracy
	// - etc.
	
	return baseChance;
}

// Updates the cursor based on hit chance during Fire mode
void
World::UpdateFireCursor(int cursorX, int cursorY, bool hasTarget)
{
	if(_selectedObjects.size() <= 0) {
		return;
	}
	
	int shooterX = _selectedObjects[0]->Position.x;
	int shooterY = _selectedObjects[0]->Position.y;
	int targetX = cursorX + _originX;
	int targetY = cursorY + _originY;
	
	// Calculate LOS
	int ox, oy, oz;
	int x0 = shooterX / 10;
	int y0 = shooterY / 10;
	int x1 = targetX / 10;
	int y1 = targetY / 10;
	bool hasLOS = _lineOfSight->CalculateLOSForTile(x0, y0, x1, y1, &ox, &oy, &oz, _currentMap);
	
	// Calculate hit chance
	int hitChance = CalculateHitChance(shooterX, shooterY, targetX, targetY, hasLOS);
	
	// Determine cursor type based on hit chance and target presence
	CursorInterface::CursorType cursorType;
	
	if(!hasLOS || hitChance < 0) {
		// No LOS - use black (no chance)
		cursorType = hasTarget ? CursorInterface::CursorType::CrosshairsBlack 
		                       : CursorInterface::CursorType::CrosshairsEmptyBlack;
	} else if(hitChance >= 75) {
		// Good chance (75-100%) - green
		cursorType = hasTarget ? CursorInterface::CursorType::CrosshairsGreen 
		                       : CursorInterface::CursorType::CrosshairsEmptyGreen;
	} else if(hitChance >= 40) {
		// OK chance (40-74%) - yellow
		cursorType = hasTarget ? CursorInterface::CursorType::CrosshairsYellow 
		                       : CursorInterface::CursorType::CrosshairsEmptyYellow;
	} else {
		// Low chance (0-39%) - red
		cursorType = hasTarget ? CursorInterface::CursorType::CrosshairsRed 
		                       : CursorInterface::CursorType::CrosshairsEmptyRed;
	}
	
	// Update the cursor
	g_Globals->Application.Cursor->ShowCursor(true, cursorType);
}

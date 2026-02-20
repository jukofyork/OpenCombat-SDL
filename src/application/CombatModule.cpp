#include "./CombatModule.h"
#include <assert.h>
#include <stdio.h>
#include <string>
#include <filesystem>
#include <graphics/AnimationManager.h>
#include <graphics/FontManager.h>
#include <graphics/Mark.h>
#include <graphics/Screen.h>
#include <graphics/ColorModifierManager.h>
#include <graphics/SoldierAnimationManager.h>
#include <misc/TGA.h>
#include <objects/SoldierManager.h>
#include <world/MiniMap.h>
#include <world/World.h>
#include <application/Globals.h>
#include <application/SoundInterface.h>
#include <states/SoldierStateLoader.h>
#include <states/SoldierActionLoader.h>
#include "../main.h"

CombatModule::CombatModule()
{
	_animationManager = NULL;
	_soldierManager = NULL;
	_terrainManager = NULL;
	_currentWorld = NULL;
	_soundManager = NULL;
	_soundEffectsManager = NULL;
	_uiManager = NULL;
	_iconManager = NULL;
	_weaponIconManager = NULL;
	_showUnitPanel = false;
	_showMiniMap = false;
	_showTeamPanel = false;
	_colorModifierManager = NULL;
	_marks = NULL;
	_frameTimeAccumulator = 0;
	_frameCount = 0;
	_currentFPS = 0.0f;
	_currentFrameTime = 0.0f;
	_longBottomBackground = NULL;
	_unitBackground = NULL;
	_teamBarBlank = NULL;
	_airstrikeNeg = NULL;
	_artilleryNeg = NULL;
	_bombardNeg = NULL;
	_activeTeamPanel = NULL;
}

CombatModule::~CombatModule(void)
{
}

void
CombatModule::Initialize(void *app)
{
	_app = app;

	// Setup our team information for the game.
	// XXX/GWS: This is just temporay. Setup two teams, one user controlled,
	//          and one computer controlled
	g_Globals->World.NumTeams = 3;
	g_Globals->World.Teams[0].Controller = PlayerControlled;
	g_Globals->World.Teams[0].Player = 0;
	g_Globals->World.Teams[0].Enemies.push_back(1);
	g_Globals->World.Teams[0].Allies.push_back(2);
	g_Globals->World.Teams[0].Nationality = 0;
	g_Globals->World.Teams[1].Controller = ComputerControlled;
	g_Globals->World.Teams[1].Player = 1;
	g_Globals->World.Teams[1].Enemies.push_back(0);
	g_Globals->World.Teams[1].Enemies.push_back(2);
	g_Globals->World.Teams[1].Nationality = 1;
	g_Globals->World.Teams[2].Controller = ComputerControlled;
	g_Globals->World.Teams[2].Player = 2;
	g_Globals->World.Teams[2].Enemies.push_back(1);
	g_Globals->World.Teams[2].Allies.push_back(0);
	g_Globals->World.Teams[2].Nationality = 2;

	g_Globals->World.CurrentPlayer = 0;

	// Load the nationalities
	g_Globals->Application.Status->Status("Loading Nationalities...");
	std::filesystem::path fileName = g_Globals->Application.ConfigDirectory / "Nationalities.xml";
	Nationality::Load(fileName, &(g_Globals->World.Nationalities));

	// Load the UI graphical elements
	g_Globals->Application.Status->Status("Loading Combat UI widgets...");
	_uiManager = new WidgetManager();
	fileName = g_Globals->Application.ConfigDirectory / "CombatUI.xml";
	_uiManager->LoadWidgets(fileName);

	g_Globals->Application.Status->Status("Loading Combat Icons...");
	_iconManager = new WidgetManager();
	fileName = g_Globals->Application.ConfigDirectory / "Icons.xml";
	_iconManager->LoadWidgets(fileName);
	g_Globals->World.Icons = _iconManager;

	g_Globals->Application.Status->Status("Loading Terrain...");
	_terrainManager = new WidgetManager();
	fileName = g_Globals->Application.ConfigDirectory / "Terrain.xml";
	_terrainManager->LoadWidgets(fileName);
	g_Globals->World.Terrain = _terrainManager;

	g_Globals->Application.Status->Status("Loading weapon icons...");
	_weaponIconManager = new WidgetManager();
	fileName = g_Globals->Application.ConfigDirectory / "WeaponIcons.xml";
	_weaponIconManager->LoadWidgets(fileName);

	Widget *widget = _uiManager->GetWidget("Bottom Background");
	if(widget) {
		_longBottomBackground = widget->GetImage();
	}

	widget = _uiManager->GetWidget("Team Panel Background");
	if(widget) _unitBackground = widget->GetImage();

	widget = _uiManager->GetWidget("Blank Team Bar");
	if(widget) _teamBarBlank = widget->GetImage();

	widget = _uiManager->GetWidget("Artillery Neg");
	if(widget) _artilleryNeg = widget->GetImage();

	widget = _uiManager->GetWidget("Airstrike Neg");
	if(widget) _airstrikeNeg = widget->GetImage();

	widget = _uiManager->GetWidget("Naval Neg");
	if(widget) _bombardNeg = widget->GetImage();

	widget = _uiManager->GetWidget("Active Team Panel");
	if(widget) _activeTeamPanel = widget->GetImage();

	// Load all of our soldier states and stuff
	g_Globals->Application.Status->Status("Loading soldier states...");
	fileName = g_Globals->Application.ConfigDirectory / "SoldierStates.txt";
	SoldierStateLoader::Load(fileName, &(g_Globals->World.States.Soldiers));

	// Soldier actions
	g_Globals->Application.Status->Status("Loading soldier actions...");
	fileName = g_Globals->Application.ConfigDirectory / "SoldierActions.txt";
	SoldierActionLoader::Load(fileName, &(g_Globals->World.Actions.Soldiers));

	// Load the color modifier
	g_Globals->Application.Status->Status("Loading color modifiers...");
	_colorModifierManager = new ColorModifierManager();
	fileName = g_Globals->Application.ConfigDirectory / "ColorModifiers.xml";
	_colorModifierManager->Load(fileName);

	// Load the current set of animations
	g_Globals->Application.Status->Status("Loading soldier animations...");
	_animationManager = (AnimationManager *) new SoldierAnimationManager();
	fileName = g_Globals->Application.ConfigDirectory / "SoldierAnimations.xml";
	_animationManager->LoadAnimations(fileName);
	fileName = g_Globals->Application.ConfigDirectory / "SoldierDeaths.xml";
	_animationManager->LoadAnimations(fileName);
	fileName = g_Globals->Application.ConfigDirectory / "SoldierDead.xml";
	_animationManager->LoadAnimations(fileName);
	fileName = g_Globals->Application.ConfigDirectory / "BazookaAnimations.xml";
	_animationManager->LoadAnimations(fileName);
	fileName = g_Globals->Application.ConfigDirectory / "MachineGunAnimations.xml";
	_animationManager->LoadAnimations(fileName);

	// Load all of the soldier templates
	g_Globals->Application.Status->Status("Loading soldiers...");
	_soldierManager = new SoldierManager();
	fileName = g_Globals->Application.ConfigDirectory / "Soldiers.xml";
	std::filesystem::path fileName2 = g_Globals->Application.ConfigDirectory / "USNames.txt";
	_soldierManager->LoadSoldiers(fileName, fileName2);
	g_Globals->World.Soldiers = _soldierManager;

	// Create the current world
	// XXX/GWS: This is hard coded for now, pretty bad
	g_Globals->Application.Status->Status("Loading world...");
	_currentWorld = new World();
	g_Globals->World.CurrentWorld = _currentWorld;
	fileName = g_Globals->Application.MapsDirectory / "Acqueville/Acqueville.xml";
	_currentWorld->Load(fileName, _soldierManager, _animationManager);

	// Set maximum window size to prevent exceeding map dimensions
	// This prevents assertion failures when window is larger than map
	if(_app) {
		CSDLApplication* app = (CSDLApplication*)_app;
		int mapWidth = _currentWorld->GetWidth();
		int mapHeight = _currentWorld->GetHeight();
		// Max height includes space for UI panels
		int bottomBarHeight = _longBottomBackground ? _longBottomBackground->GetHeight() : 84;
		int maxHeight = mapHeight + bottomBarHeight;
		app->SetMaxWindowSize(mapWidth, maxHeight);
	}

	// Create the mini map
	g_Globals->Application.Status->Status("Creating mini map...");
	_currentMiniMap = MiniMap::Create(_currentWorld->GetMiniMapName(), _currentWorld);
	_currentWorld->SetMiniMap(_currentMiniMap);

	// Create the sound manager
	g_Globals->Application.Status->Status("Loading soldier voices...");
	_soundManager = new SoundManager();
	fileName = g_Globals->Application.ConfigDirectory / "EnglishVoices.xml";
	_soundManager->LoadSounds(fileName);
	g_Globals->World.Voices = _soundManager;

	g_Globals->Application.Status->Status("Loading sound effects...");
	_soundEffectsManager = new SoundManager();
	fileName = g_Globals->Application.ConfigDirectory / "SoundEffects.xml";
	_soundEffectsManager->LoadSounds(fileName);
	g_Globals->World.SoundEffects = _soundEffectsManager;

	_marks = new Mark();
	_marks->LoadMarks(_iconManager);
	g_Globals->World.Marks = _marks;
}

void 
CombatModule::Simulate(long dt)
{
	_currentWorld->Simulate(dt);

	// Track FPS and frame time
	_frameTimeAccumulator += dt;
	_frameCount++;
	
	// Update FPS display every 500ms
	if(_frameTimeAccumulator >= 500) {
		_currentFrameTime = (float)_frameTimeAccumulator / (float)_frameCount;
		_currentFPS = (float)_frameCount * 1000.0f / (float)_frameTimeAccumulator;
		_frameTimeAccumulator = 0;
		_frameCount = 0;
	}
}

void 
CombatModule::Render(Screen *screen)
{
	int dwidth;
	int squadPanelDY = 7;
	int x = 0, y = 0;

	// Guard against NULL pointers if widget loading failed
	if(!_longBottomBackground) {
		return;
	}

	// Render the world
	Rect clip;
	clip.x = 0;
	clip.y = 0;
	clip.w = screen->GetWidth();
	clip.h = screen->GetHeight() - _longBottomBackground->GetHeight() - (_showTeamPanel && _unitBackground ? _unitBackground->GetHeight()-squadPanelDY : 0);
	screen->SetClippingRectangle(clip.x, clip.y, clip.w, clip.h);
	_currentWorld->Render(screen, &clip);
	screen->SetClippingRectangle(0, 0, screen->GetWidth(), screen->GetHeight());

	// Render the mini map
	if(_showMiniMap) {
		x = 0;
		y = screen->GetHeight() - _longBottomBackground->GetHeight() - (_showTeamPanel && _unitBackground ? _unitBackground->GetHeight()-squadPanelDY : 0) - _currentMiniMap->GetHeight();
		_currentMiniMap->SetPosition(x, y);
		// Tell the minimap the actual visible map area (accounting for panels)
		_currentMiniMap->SetVisibleArea(clip.w, clip.h);
		_currentMiniMap->Render(screen);
	}

	// Render the unit panel
	if(_currentWorld->State.SelectedSquad >= 0 && _showUnitPanel) {
		x = 0;
		y = screen->GetHeight() - _longBottomBackground->GetHeight() - (_showTeamPanel && _unitBackground ? _unitBackground->GetHeight()-squadPanelDY : 0);
		int dy;
		for(int i = _currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].NumUnits-1; i >= 0 ; --i) {
			Widget *w = _uiManager->GetWidget("Unit Panel");
			x = screen->GetWidth() - w->GetWidth();
			w->Render(screen, x, y-w->GetHeight());

			// Am I selected?
			if(_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].SelectedSoldierIdx == i)
			{
				// Draw a yellow rectangle
				Color yellow(255,255,0);
				screen->DrawRect(x, y-w->GetHeight(), w->GetWidth()-1, w->GetHeight()-1, 1, &yellow);
			}

			dy = w->GetHeight();
			delete w;
			
Color white(255,255,255);
			g_Globals->World.Fonts->Render(screen, _currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].UnitStates[i].Name, x+3+10, y-dy+3, &white);

			// Do the status
			w = NULL;
			switch(_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].UnitStates[i].CurrentStatus)
			{
			case Unit::Healthy:
				w = _iconManager->GetWidget("Unit Status Healthy");
				break;
			case Unit::Dead:
				w = _iconManager->GetWidget("Unit Status Dead");
				break;
			case Unit::Wounded:
				w = _iconManager->GetWidget("Unit Status Wounded");
				break;
			default:
				w = _iconManager->GetWidget("Unit Status Healthy");
				break;
			}
			w->Render(screen, x+158,y-dy+2);
			delete w;

			// Do the action
			w = NULL;
			switch(_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].UnitStates[i].CurrentAction)
			{
			case Unit::Defending:
				w = _iconManager->GetWidget("Unit Action Defending Green");
				break;
			case Unit::Firing:
				w = _iconManager->GetWidget("Unit Action Firing Green");
				break;
			case Unit::Reloading:
				w = _iconManager->GetWidget("Unit Action Reloading Green");
				break;
			case Unit::NoTarget:
				w = _iconManager->GetWidget("Unit Action No Target Green");
				break;
			case Unit::Moving:
				w = _iconManager->GetWidget("Unit Action Moving Green");
				break;
			case Unit::Crawling:
				w = _iconManager->GetWidget("Unit Action Crawling Green");
				break;
			case Unit::MovingFast:
				w = _iconManager->GetWidget("Unit Action Running Green");
				break;
			case Unit::Ambushing:
				w = _iconManager->GetWidget("Unit Action Defending Green");
				break;
			case Unit::Sneaking:
				w = _iconManager->GetWidget("Unit Action Crawling Green");
				break;
			default:
				w = _iconManager->GetWidget("Unit Action Defending White");
				break;
			}
			w->Render(screen, x+2,y-dy+18);
			delete w;

// Do the weapon icon
			w = _weaponIconManager->GetWidget(_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].UnitStates[i].WeaponIcon);
			w->Render(screen, x+93, y-dy+18);
			delete w;

			// Do the number of rounds
			std::string rounds = std::to_string(_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].UnitStates[i].NumRounds);
			g_Globals->World.Fonts->Render(screen, rounds, x+166, y-dy+18, &white);

// Do the title
			w = _iconManager->GetWidget(_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].UnitStates[i].Title);
			w->Render(screen, x+93, y-dy+3);
			delete w;

			y -= dy;
		}
	}

	// Now the blank tile for the squads.
	if(_showTeamPanel && _unitBackground && _activeTeamPanel) {
		dwidth = 0;
		while(dwidth < screen->GetWidth()) {
			screen->Blit(_unitBackground->GetData(),
				dwidth, screen->GetHeight() - _longBottomBackground->GetHeight() - _unitBackground->GetHeight() + squadPanelDY,
				((dwidth+_unitBackground->GetWidth()) < screen->GetWidth()) ? _unitBackground->GetWidth() : screen->GetWidth() - dwidth, _unitBackground->GetHeight()-squadPanelDY,
				_unitBackground->GetWidth(), _unitBackground->GetHeight(),
				_unitBackground->GetDepth());
			dwidth += _unitBackground->GetWidth();
		}

		// Render the active team panel
		x = 0;
		y = screen->GetHeight() - _longBottomBackground->GetHeight() - _unitBackground->GetHeight() + squadPanelDY;
		for(int i = 0, j = 0; i < _currentWorld->State.NumSquads; ++i) {
			screen->Blit(_activeTeamPanel->GetData(), x, y,
				_activeTeamPanel->GetWidth(), _activeTeamPanel->GetHeight(),
				_activeTeamPanel->GetWidth(), _activeTeamPanel->GetHeight(),
				_activeTeamPanel->GetDepth());
		
			if(_currentWorld->State.SelectedSquad == i) {
				// This one is selected, so highlight it
				Color yellow(255, 255, 0);
				screen->DrawRect(x, y, _activeTeamPanel->GetWidth(), _activeTeamPanel->GetHeight(), 1, &yellow);
			}

			// First render the icon
			// XXX/GWS: This should be done better
			Widget *w = _iconManager->GetWidget(_currentWorld->State.SquadStates[i].Icon);
			w->Render(screen, x+4, y + 3);
			delete w;
		
			// Now render the squad name background
			w = _iconManager->GetWidget("Green Background");
			w->Render(screen, x+43, y+3, 77, 10);
			delete w;

			// Now render the squad name
			Color black(0,0,0);
			g_Globals->World.Fonts->Render(screen, _currentWorld->State.SquadStates[i].Name, x+44,y+3, &black);

			// Now render the current action
			w = NULL;
			switch(_currentWorld->State.SquadStates[i].CurrentAction) {
				case Team::Moving:
					w = _iconManager->GetWidget("Team Action Moving Green");
					break;
				case Team::MovingFast:
					w = _iconManager->GetWidget("Team Action Moving Fast Green");
					break;
				case Team::Sneaking:
					w = _iconManager->GetWidget("Team Action Sneaking Green");
					break;
			case Team::Defending:
				w = _iconManager->GetWidget("Team Action Defending White");
				break;
			case Team::Ambushing:
				w = _iconManager->GetWidget("Team Action Defending White");
				break;
			case Team::Hiding:
				w = _iconManager->GetWidget("Team Action Sneaking Green");
				break;
			case Team::Cowering:
				w = _iconManager->GetWidget("Team Action Defending White");
				break;
			case Team::Firing:
				w = _iconManager->GetWidget("Team Action Firing Green");
				break;
			default:
				w = _iconManager->GetWidget("Team Action Defending White");
				break;
			}
			w->Render(screen, x+43, y+16, 77, 10);
			delete w;

			// Now keep track of where we are drawing
			++j;
			if(j >= 3) {
				x += _activeTeamPanel->GetWidth();
				y = screen->GetHeight() - _longBottomBackground->GetHeight() - _unitBackground->GetHeight() + squadPanelDY;
				j = 0;
			} else {
				y += _activeTeamPanel->GetHeight();
			}
		}
	}

	// Render the long panel on the bottom
	// Guard against NULL pointers if widget loading failed
	if(_longBottomBackground && _airstrikeNeg && _artilleryNeg && _bombardNeg && _teamBarBlank) {
		dwidth = 0;
		while(dwidth < screen->GetWidth()) {
			screen->Blit(_longBottomBackground->GetData(), 
				dwidth, screen->GetHeight() - _longBottomBackground->GetHeight(),
				((dwidth+_longBottomBackground->GetWidth()) < screen->GetWidth()) ? _longBottomBackground->GetWidth() : screen->GetWidth() - dwidth, _longBottomBackground->GetHeight(),
				_longBottomBackground->GetWidth(), _longBottomBackground->GetHeight(),
				_longBottomBackground->GetDepth());
			dwidth += _longBottomBackground->GetWidth();
		}

		screen->Blit(_airstrikeNeg->GetData(),
			47, screen->GetHeight() - _longBottomBackground->GetHeight()+ 4,
			_airstrikeNeg->GetWidth(), _airstrikeNeg->GetHeight(),
			_airstrikeNeg->GetWidth(), _airstrikeNeg->GetHeight(),
			_airstrikeNeg->GetDepth());
		screen->Blit(_artilleryNeg->GetData(),
			47+_airstrikeNeg->GetWidth(), screen->GetHeight() - _longBottomBackground->GetHeight()+ 4,
			_artilleryNeg->GetWidth(), _artilleryNeg->GetHeight(),
			_artilleryNeg->GetWidth(), _artilleryNeg->GetHeight(),
			_artilleryNeg->GetDepth());
		screen->Blit(_bombardNeg->GetData(),
			47+_artilleryNeg->GetWidth()+_airstrikeNeg->GetWidth(), screen->GetHeight() - _longBottomBackground->GetHeight()+ 4,
			_bombardNeg->GetWidth(), _bombardNeg->GetHeight(),
			_bombardNeg->GetWidth(), _bombardNeg->GetHeight(),
			_bombardNeg->GetDepth());

		// Render the team bar
		x = 47+_artilleryNeg->GetWidth()+_airstrikeNeg->GetWidth()+_bombardNeg->GetWidth()+10;
		y = screen->GetHeight() - _longBottomBackground->GetHeight() + 4;
		screen->Blit(_teamBarBlank->GetData(), x, y,
			_teamBarBlank->GetWidth(), _teamBarBlank->GetHeight(),
			_teamBarBlank->GetWidth(), _teamBarBlank->GetHeight(),
			_teamBarBlank->GetDepth());
	}

	if(_currentWorld->State.SelectedSquad >= 0) {
		Widget *w = _iconManager->GetWidget(_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].Icon);
		w->Render(screen, x+4, y+6);
		delete w;

		w = _iconManager->GetWidget("Green Background");
		w->Render(screen, x+84, y+4);
		delete w;

		Color black(0,0,0);
		g_Globals->World.Fonts->Render(screen, _currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].Name, x+85,y+4, &black);

		int x1=x+84, y1=y+22;
		for(int j = 0; j < _currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].NumUnits; ++j) {
			switch(_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].UnitStates[j].CurrentStatus)
			{
			case Unit::Healthy:
			default:
				w = _iconManager->GetWidget("Team Head Green");
				break;
			case Unit::Wounded:
				w = _iconManager->GetWidget("Team Head Yellow");
				break;
			case Unit::Dead:
				w = _iconManager->GetWidget("Team Head Red");
				break;
			}
			w->Render(screen, x1, y1);
			x1 += w->GetWidth() + 2;
			delete w;
		}

		// Render the rank
		Color white(255,255,255);
		w = _iconManager->GetWidget(_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].UnitStates[_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].SquadLeaderIdx].Rank);
		w->Render(screen, x+47, y+5, &white);
		delete w;

		// Render the team quality
		w = _iconManager->GetWidget(_currentWorld->State.SquadStates[_currentWorld->State.SelectedSquad].Quality);
		w->Render(screen, x+171, y+21);
		delete w;
	}

	// Render FPS and frame time stats if enabled
	if(g_Globals->World.bRenderStats) {
		Color yellow(255,255,0);
		char stats[64];
		snprintf(stats, sizeof(stats), "FPS: %.1f  Frame: %.2fms", _currentFPS, _currentFrameTime);
		// Position in top-right corner (10px from top, 10px from right)
		g_Globals->World.Fonts->Render(screen, stats, screen->GetWidth() - 140, 10, &yellow);
	}

	// Render help text if enabled
	if(g_Globals->World.bRenderHelpText) {
		Color yellow(255,255,0);
		int y = 10;
		int lineHeight = 14;
		g_Globals->World.Fonts->Render(screen, "=== CONTROLS ===", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "F1: Toggle this help", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "F2: Toggle FPS/stats display", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "F3: Toggle path rendering", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "F4: Toggle weapon fan/LOS", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "F5: Toggle minimap", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "F6: Toggle team panel", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "F7: Toggle unit panel", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "F8: Cycle building display", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "F9: Toggle terrain elements", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "F10: Toggle bounding boxes", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "MOUSE:", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "Left Click: Select unit", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "Right Click: Context menu", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "Middle Drag: Pan camera", 10, y, &yellow);
		y += lineHeight;
		g_Globals->World.Fonts->Render(screen, "Arrow Keys: Scroll view", 10, y, &yellow);
	}
}

void 
CombatModule::LeftMouseDown(int x, int y)
{
	if(_showMiniMap && _currentMiniMap->Contains(x,y)) {
		_currentMiniMap->LeftMouseDown(x,y);
	} else {
		_currentWorld->LeftMouseDown(x,y);
	}
}

void 
CombatModule::LeftMouseUp(int x, int y)
{
	if(_showMiniMap && _currentMiniMap->Contains(x, y)) {
		_currentMiniMap->LeftMouseUp(x, y);
	} else {
		_currentWorld->LeftMouseUp(x,y);
	}
}

void 
CombatModule::LeftMouseDrag(int x, int y)
{
	if(_showMiniMap && _currentMiniMap->Contains(x,y)) {
		_currentMiniMap->LeftMouseDrag(x, y);
	}
}

void 
CombatModule::RightMouseDown(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void 
CombatModule::RightMouseUp(int x, int y)
{
	_currentWorld->RightMouseUp(x,y);
}

void 
CombatModule::RightMouseDrag(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void 
CombatModule::KeyUp(int key)
{
	switch(key) {
		case 112: /* F1 */
			g_Globals->World.bRenderHelpText = !g_Globals->World.bRenderHelpText;
			break;
		case 113: /* F2 */
			g_Globals->World.bRenderStats = !g_Globals->World.bRenderStats;
			break;
		case 114: /* F3 */
			g_Globals->World.bRenderPaths = !g_Globals->World.bRenderPaths;
			break;
		case 115: /* F4 */
			g_Globals->World.bWeaponFan = !g_Globals->World.bWeaponFan;
			break;
		case 116: /* F5 */
			_showMiniMap = !_showMiniMap;
			break;
		case 117: /* F6 */
			_showTeamPanel = !_showTeamPanel;
			break;
		case 118: /* F7 */
			_showUnitPanel = !_showUnitPanel;
			break;
		case 119: /* F8 */
			// The order is Interiors->Outlines->Elevation->NULL and back
			if(g_Globals->World.bRenderBuildingInteriors)
			{
				g_Globals->World.bRenderBuildingOutlines = true;
				g_Globals->World.bRenderElevation = false;
				g_Globals->World.bRenderBuildingInteriors = false;
			}
			else if(g_Globals->World.bRenderBuildingOutlines) {
				g_Globals->World.bRenderBuildingOutlines = false;
				g_Globals->World.bRenderElevation = true;
				g_Globals->World.bRenderBuildingInteriors = false;
			} else if(g_Globals->World.bRenderElevation) {
				g_Globals->World.bRenderElevation = false;
				g_Globals->World.bRenderBuildingInteriors = false;
				g_Globals->World.bRenderBuildingOutlines = false;
			} else {
				g_Globals->World.bRenderBuildingInteriors = true;
			}
			break;
		case 120: /* F9 */
			g_Globals->World.bRenderElements = !g_Globals->World.bRenderElements;
			break;
		case 121: /* F10 */
			g_Globals->World.bRenderBoundingBoxes = !g_Globals->World.bRenderBoundingBoxes;
			break;
	default:
			_currentWorld->KeyUp(key);
			break;
	}
}

void
CombatModule::KeyDown(int key)
{
	// Pass through to world for scroll key handling
	// (F-keys and other special keys don't need repeat)
	if(key != 112 && key != 113 && key != 114 && key != 115 && key != 116 && key != 117 && key != 118 && key != 119 && key != 120 && key != 121) {
		_currentWorld->KeyDown(key);
	}
}

void
CombatModule::MiddleMouseDown(int x, int y)
{
	_currentWorld->MiddleMouseDown(x, y);
}

void
CombatModule::MiddleMouseUp(int x, int y)
{
	_currentWorld->MiddleMouseUp(x, y);
}

void
CombatModule::MiddleMouseDrag(int x, int y)
{
	_currentWorld->MiddleMouseDrag(x, y);
}

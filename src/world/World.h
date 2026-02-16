#pragma once

#include <filesystem>
#include <string>

struct Rect;
class AnimationManager;
class ColorManager;
class Effect;
class EffectManager;
class ElementManager;
class LineOfSight;
class Object;
class Order;
class Screen;
class SoldierManager;
class SquadManager;
class VehicleManager;
class WeaponManager;
class MiniMap;

#include <misc/Color.h>
#include <graphics/CombatContextMenu.h>
#include <graphics/Mark.h>
#include <world/Map.h>
#include <objects/Status.h>

// This class is how the world communicates its state
// about the objects in it to the user interface
// above it
#define MAX_UNITS		16
#define MAX_SQUADS		32

class UnitState
{
public:
	std::string Name;
	std::string State;
	std::string Icon;
	std::string WeaponIcon;
	std::string Title;
	std::string Rank;
	int NumRounds;
	long ID;
	Unit::Action CurrentAction;
	Unit::Status CurrentStatus;
};

class SquadState
{
public:
	std::string Name;
	std::string Icon;
	std::string Quality;
	UnitState UnitStates[MAX_UNITS];
	int NumUnits;
	long ID;
	int SquadLeaderIdx;
	int SelectedSoldierIdx;
	Team::Action CurrentAction;
	Team::Status CurrentStatus;
};

class InterfaceState
{
public:
	SquadState SquadStates[MAX_SQUADS];
	int NumSquads;
	int SelectedSquad;
};

class World
{
public:
	World(void);
	virtual ~World(void);

	// The states of this world
	enum WorldState {
		Normal,			// Normal rendering operations
		ContextSelecting,	// Currently selecting an item on the context menu
		ContextSelected, // An item on the context menu was selected and we are performing and action
		Ambushing, // Choosing an ambush direction
		Defending  // Choosing a defend direction
	};

	// Renders this world to the screen, given a clipping rectangle
	virtual void Render(Screen *screen, Rect *clip);

	// Loads a world from a file
	virtual void Load(const std::filesystem::path& fileName, SoldierManager *soldierManager, AnimationManager *animationManager);

	// Adds an object to this world
	virtual void AddObject(Object *o);

	// Moves an object in the world from one position to another
	virtual void MoveObject(Object *object, Point *from, Point *to);

	// Simulate this world for an interval of time
	virtual void Simulate(long dt);

	// Trys to move an object to a new position in the world
	virtual bool TryMove(Object *o, int x, int y);

	// Mouse actions
	virtual void LeftMouseDown(int x, int y);
	virtual void LeftMouseUp(int x, int y);
	virtual void LeftMouseDrag(int x, int y);
	virtual void RightMouseDown(int x, int y);
	virtual void RightMouseUp(int x, int y);
	virtual void RightMouseDrag(int x, int y);

	// Key events - XXX/GWS: This is only used for debugging the game I believe
	virtual void KeyUp(int key);
	virtual void KeyDown(int key);

	// Middle mouse drag scrolling
	virtual void MiddleMouseDown(int x, int y);
	virtual void MiddleMouseUp(int x, int y);
	virtual void MiddleMouseDrag(int x, int y);

	// Issue orders to all selected objects
	virtual void IssueOrder(Order *o);

	// Gets the width and height of the world
	inline int GetWidth() { return _currentMap->GetWidth(); }
	inline int GetHeight() { return _currentMap->GetHeight(); }

	// Set's the origin of this world
	void SetOrigin(int x, int y);
	void GetOrigin(int *x, int *y) { *x = _originX; *y = _originY; }

	// Gets the list of mobile objects in this world
	inline std::vector<Object*> *GetObjects() { return &_mobileObjects; }

	// The state of objects in this world
	InterfaceState State;

	// Gets the name of the mini map for this world
	const std::string& GetMiniMapName() { return _currentMap->GetMiniName(); }

	// Gets the name of the overland map
	const std::string& GetOverlandName() { return _currentMap->GetOverlandName(); }

	// Checks whether or not the tile (i,j) is passable
	bool IsPassable(int i, int j) { return _currentMap->GetTileElement(i,j)->Passable; }
	Element *GetTileElement(int i, int j) { return _currentMap->GetTileElement(i,j); }
	Point NumTiles;
	Size TileSize;

	// Converts a tile (i,j) location to a world position (x,y)
	void ConvertTileToPosition(int i, int j, int *x, int *y);
	void ConvertPositionToTile(int x, int y, int *i, int *j);

	// Add a mark to a given position
	void AddMark(Mark::Color markColor, int x, int y);
	// Clear all our marks
	void ClearMarks();

	// Set's the mini map for this world
	void SetMiniMap(MiniMap *miniMap) { _currentMiniMap = miniMap; }

	// Gets the number of victory locations
	int GetNumVictoryLocations() { return _currentMap->GetNumVictoryLocations(); }
	void GetVictoryLocation(int index, int *x, int *y, Nationality **nationality) { return _currentMap->GetVictoryLocation(index, x, y, nationality); }
	const std::string& GetVictoryLocationName(int index) { return _currentMap->GetVictoryLocationName(index); }

protected:

	// Clears all of the current selections
	void ClearSelect(int x, int y);

	// Trys to select an object at (x,y)
	void Select(int x, int y);

	// Updates the State variable
	void UpdateState();

	// Calculates hit chance for firing (0-100%)
	// Returns: -1 if no LOS, otherwise 0-100
	int CalculateHitChance(int shooterX, int shooterY, int targetX, int targetY, bool hasLOS);

	// Updates the cursor based on hit chance during Fire mode
	void UpdateFireCursor(int cursorX, int cursorY, bool hasTarget);

	// The current map for this world
	Map *_currentMap;

	// The current state of this world
	WorldState _currentState;

	// A list of mobile objects.
	std::vector<Object*> _mobileObjects;

	// A list of static objects
	std::vector<Object*> _staticObjects;

	// The list of selected objects
	std::vector<Object*> _selectedObjects;

	// A list of effects in the world
	std::vector<Effect*> _effects;

	// Soldier and animation manager from the combat module
	SoldierManager *_soldierManager;
	AnimationManager *_animationManager;
	SquadManager *_squadManager;
	EffectManager *_effectManager;
	WeaponManager *_weaponManager;
	ElementManager *_elementManager;
	VehicleManager *_vehicleManager;

	// A system color manager
	ColorManager *_colorManager;

	// A context menu for this world
	CombatContextMenu *_contextMenu;

	// The current choice of the context menu
	CombatContextMenu::ContextMenuChoice _currentChoice;

	// The source position for the 'ranger' line
	int _rangerX, _rangerY;
	
	// The current selection at the end of the ranger line
	Object *_rangerSelectedObject;

	// The color of the ranger line
	Color _rangerColor;

	// The origin of the world
	int _originX, _originY;

	// The width and height of our screen bounds
	int _screenWidth, _screenHeight;

	// Lists of marks that we have to render
	Point *_markPoints;
	Mark::Color *_markColors;
	int _numMarks;
	int _maxMarks;

	// A line of sight calculator
	LineOfSight *_lineOfSight;

	// The direction of the heading arc (used for Ambushing and Defending)
	Direction _currentHeadingArc;

	// The current mini map for this world
	MiniMap *_currentMiniMap;

	// Scroll state for key repeat
	bool _scrollLeft;
	bool _scrollRight;
	bool _scrollUp;
	bool _scrollDown;
	bool _scrollRepeating;
	long _scrollTimer;

	// Middle mouse drag scrolling state
	bool _middleDragActive;
	int _middleDragLastX;
	int _middleDragLastY;
};

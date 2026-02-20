#include "./Object.h"
#include <string.h> // for memset()
#include <orders/Order.h>
#include <graphics/Screen.h>
#include <application/Globals.h>

static long g_id = 0;

Object::Object()
{
	NextObject = nullptr;
	PrevObject = nullptr;
	Position.x = Position.y = 0;
	_isSelected = false;
	_bHighlight = false;
	memset(&_minBounds, 0, sizeof(Bounds));
	_id = g_id++;
	_type = Target::NoTarget;
	_health = HEALTH_MAX;
	_bSquadLeader = false;
	_moving = false;
	_pathComplete = false;
	_currentHeading = Direction::North;
	
	// Action states
	_canFire = false;
	_canMove = false;
	_canMoveFast = false;
	_canSneak = false;
	_canDefend = false;
	_canAmbush = false;
	_canSmoke = false;
	
	// Target info
	_currentTarget = nullptr;
	_currentTargetType = Target::NoTarget;
	_currentTargetX = 0;
	_currentTargetY = 0;
	
	// Squad and team info
	_currentSquad = nullptr;
	_currentTeamID = -1;
	_currentTileElement = nullptr;
	
	// Color
	_highlightColor = Color(0, 0, 0);
}

Object::~Object(void)
{
}

bool
Object::Select(int x, int y)
{
	return Contains(x,y);
}

bool
Object::Contains(int x, int y)
{
	Region r;
	r.points[0].x = _minBounds.x0; r.points[0].y = _minBounds.y0;
	r.points[1].x = _minBounds.x1; r.points[1].y = _minBounds.y0;
	r.points[2].x = _minBounds.x1; r.points[2].y = _minBounds.y1;
	r.points[3].x = _minBounds.x0; r.points[3].y = _minBounds.y1;
	return Screen::PointInRegion(x, y, &r);
}

void
Object::AddOrder(Order *o)
{
	o->IncrementRefCount();
	_orders.push_back(o);
}

void
Object::InsertOrder(Order *o, int i)
{
	o->IncrementRefCount();
	_orders.insert(_orders.begin() + i, o);
}

void
Object::ClearOrders()
{
	while(!_orders.empty()) {
		Order *o = _orders.front();
		_orders.pop_front();
		o->Release();
	}
}

void
Object::SetPosition(int x, int y)
{
	Position.x = x;
	Position.y = y;
}

bool
Object::IsStopped()
{
	return !_moving;
}

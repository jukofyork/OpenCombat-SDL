#include "./Squad.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <assert.h>
#include <world/World.h>
#include <graphics/Mark.h>
#include <graphics/Screen.h>
#include <graphics/Widget.h>
#include <sound/Sound.h>
#include <application/Globals.h>

static const std::vector<std::string> _squadQualityIcons = { "Team Quality Useless", "Team Quality Fragile", "Team Quality Weak", "Team Quality Average", "Team Quality Good", "Team Quality Strong" };

Squad::Squad() : Object()
{
	_currentStatus = Team::Healthy;
	_currentAction = Team::Defending;
	_quality = Average;
	_type = Target::Squad;
	_currentTarget = NULL;
	_selectedSoldierIdx = -1;
	_selectedVehicleIdx = -1;
	_currentPointManIdx = 0;
	_canMove = true;
	_canMoveFast = true;
	_canDefend = true;
	_canSmoke = true;
	_canAmbush = true;
	_canSneak = true;
	_canFire = true;
	_currentPath = NULL;
	_bShowMark = false;
	_bMarkTargetPosition = false;
	_currentFormationSpread = 2.0f;
	_currentFormation = Formation::Column;
}

Squad::~Squad(void)
{
}

void
Squad::Render(Screen *screen, Rect *clip)
{
	// If our squad contains the cursor, then
	// we need to highlight this
	if(Contains(screen->GetCursorX() + screen->Origin.x, screen->GetCursorY() + screen->Origin.y))
	{
		Color white(255,255,255);
		Highlight(&white);
	} else {
		UnHighlight();
	}

	// Render our current path as a series of filled blocks
	if(g_Globals->World.bRenderPaths && _currentPath != NULL) {
		Path *p = _currentPath;
		while(p != NULL) {
			Color red(128,0,0);

			// We need to clip the path rectangles
			if(screen->PointInRegion(p->X*g_Globals->World.CurrentWorld->TileSize.w-screen->Origin.x, 
				p->Y*g_Globals->World.CurrentWorld->TileSize.h-screen->Origin.y, 
				clip->x, clip->y, clip->w, clip->h))
			{
				screen->FillRect(p->X*g_Globals->World.CurrentWorld->TileSize.w-screen->Origin.x,
					p->Y*g_Globals->World.CurrentWorld->TileSize.h-screen->Origin.y,
					g_Globals->World.CurrentWorld->TileSize.w,
					g_Globals->World.CurrentWorld->TileSize.h, &red);
			}
			p = p->Next;
		}
	}

	for(auto* vehicle : _vehicles) {
		vehicle->Render(screen, clip);
	}

	for(size_t i = 0; i < _soldiers.size(); ++i) {
		_soldiers[i]->Render(screen, clip);
		if(_selectedSoldierIdx == static_cast<int>(i) && IsSelected()) {
			Color white(255,255,255);
			Widget *w = g_Globals->World.Icons->GetWidget("Unit Selected Bracket");
			w->Render(screen, _soldiers[i]->Position.x - screen->Origin.x, _soldiers[i]->Position.y-screen->Origin.y, &white);
			delete w;
		}
	}

	if(IsSelected() && IsActive() && _bShowMark)
	{
		if(_bMarkTargetPosition) {
			g_Globals->World.CurrentWorld->AddMark(_markColor, _currentTarget->Position.x, _currentTarget->Position.y);
		} else {
			g_Globals->World.CurrentWorld->AddMark(_markColor, _currentTargetX, _currentTargetY);
		}
	}
}

void
Squad::Simulate(long dt, World *world)
{
	// Simulate any vehicles
	for(auto* vehicle : _vehicles) {
		vehicle->Simulate(dt, world);
	}

	// XXX/GWS: Make sure the squad leader always moves first!!!
	for(auto* soldier : _soldiers) {
		soldier->Simulate(dt, world);
	}

	// Now update the position of the squad. Use the position of the
	// 'squad leader'
	// XXX/GWS: Need to handle squads of more than one vehicle here!
	Position.x = GetSquadLeader()->Position.x;
	Position.y = GetSquadLeader()->Position.y;
}

bool
Squad::Select(int x, int y)
{
	for(size_t i = 0; i < _vehicles.size(); ++i) {
		if(_vehicles[i]->Contains(x,y)) {
			_selectedVehicleIdx = static_cast<int>(i);
			Select(true);
			return true;
		}
	}

	for(size_t i = 0; i < _soldiers.size(); ++i) {
		if(_soldiers[i]->Contains(x,y)) {
			// Play a sound
			g_Globals->World.Voices->GetSound("awaiting orders")->Play();
			Select(true);
			_selectedSoldierIdx = static_cast<int>(i);
			return true;
		}
	}
	_selectedVehicleIdx = -1;
	_selectedSoldierIdx = -1;
	return false;
}

bool 
Squad::IsSelected()
{
	for(auto* vehicle : _vehicles) {
		if(vehicle->IsSelected()) {
			return true;
		}
	}

	for(auto* soldier : _soldiers) {
		if(soldier->IsSelected()) {
			return true;
		}
	}
	return false;
}

void 
Squad::SetPosition(int x, int y)
{
	int sx=0,sy=0;
	Object::SetPosition(x,y);

	// Let's set our soldier positions based on the current formation
	// XXX/GWS: Need smarter formation setting here....cover, etc
	int j = 1;
	for(size_t i = 0; i < _soldiers.size(); ++i) {
		if(static_cast<int>(i) != _currentPointManIdx)
		{
			Direction heading = _soldiers[_currentPointManIdx]->GetHeading();
			Formation::GetFormationPosition(_currentFormation, j++, _currentFormationSpread, &Position,
				heading, &sx, &sy);
			_soldiers[i]->SetPosition(sx, sy);
		}
		else
		{
			Direction heading = _soldiers[_currentPointManIdx]->GetHeading();
			Formation::GetFormationPosition(_currentFormation, 0, _currentFormationSpread, &Position,
				heading, &sx, &sy);
			_soldiers[i]->SetPosition(sx, sy);
		}
	}

	for(size_t i = 0; i < _vehicles.size(); ++i) {
		_vehicles[i]->SetPosition(x+static_cast<int>(i)*20, y);
	}
}

void 
Squad::Select(bool s)
{
	for(auto* vehicle : _vehicles) {
		vehicle->Select(s);
	}

	for(auto* soldier : _soldiers) {
		soldier->Object::Select(s);
	}
}

void
Squad::AddOrder(Order *o)
{
	_bShowMark = false;
	switch(o->GetType()) {
		case Orders::Ambush:
			HandleAmbushOrder((AmbushOrder *)o);
			return;
		case Orders::Defend:
			HandleDefendOrder((DefendOrder *)o);
			return;
		case Orders::Fire:
			{
				FireOrder *f = (FireOrder *)o;
				_currentAction = Team::Firing;
				_currentTarget = f->Target;
				_currentTargetType = f->TargetType;
				_currentTargetX = f->X;
				_currentTargetY = f->Y;
				_markColor = Mark::Red;
				_bShowMark = true;
				if(f->TargetType == Target::Area) {
					_bMarkTargetPosition = false;
				} else {
					_bMarkTargetPosition = true;
				}
			}
			break;
		case Orders::Hide:
			_currentAction = Team::Hiding;
			break;
		case Orders::Move:
			HandleMoveOrder((MoveOrder *)o, SoldierAction::WalkTo, Mark::Blue);
			for(auto* vehicle : _vehicles) {
				vehicle->AddOrder(o);
			}
			return;
		case Orders::MoveFast:
			HandleMoveOrder((MoveOrder *)o, SoldierAction::RunTo, Mark::Purple);
			for(auto* vehicle : _vehicles) {
				vehicle->AddOrder(o);
			}
			return;
		case Orders::Sneak:
			HandleMoveOrder((MoveOrder *)o, SoldierAction::CrawlTo, Mark::Yellow);
			for(auto* vehicle : _vehicles) {
				vehicle->AddOrder(o);
			}
			return;
	}

	for(auto* vehicle : _vehicles) {
		vehicle->AddOrder(o);
	}

	for(auto* soldier : _soldiers) {
		soldier->AddOrder(o);
	}
}

void
Squad::HandleMoveOrder(MoveOrder *order, SoldierAction::Action movementStyle, Mark::Color color)
{
	int i=0,j=0,di=0,dj=0;

	_currentAction = Team::Moving;
	_currentTargetX = order->X;
	_currentTargetY = order->Y;

	// Find a path
	FreePath(_currentPath, true);
	g_Globals->World.CurrentWorld->ConvertPositionToTile(Position.x, Position.y, &i, &j);
	g_Globals->World.CurrentWorld->ConvertPositionToTile(_currentTargetX, _currentTargetY, &di, &dj);
	Element::Level level = Element::Medium;
	switch(movementStyle)
	{
	case SoldierAction::Crawl:
	case SoldierAction::CrawlTo:
		level = Element::Prone;
		break;

	case SoldierAction::Run:
	case SoldierAction::RunTo:
		level = Element::High;
		break;

	case SoldierAction::WalkSlow:
	case SoldierAction::WalkSlowTo:
		level = Element::Low;
		break;

	case SoldierAction::Walk:
	case SoldierAction::WalkTo:
	default:
		level = Element::Medium;
		break;
	}

	_currentPath = g_Globals->World.Pathing.FindPath(i, j, di, dj, level);
	if(NULL == _currentPath) {
		g_Globals->World.Voices->GetSound("no clear path")->Play();
		return;
	}

	// Set our mark colors
	_markColor = color;
	_bShowMark = true;
	_bMarkTargetPosition = false;

	// Now we need to get our soldiers moving. We order the point man
	// to walk/run/crawl to the location, and we order everyone else
	// to follow him in formation.
	//
	// XXX/GWS: What happens if our point man is dead?
	j = 1;
	for(size_t idx = 0; idx < _soldiers.size(); ++idx) {
		if(static_cast<int>(idx) == _currentPointManIdx)
		{
			// Order this soldier to walk/run/crawl to the destination
			_soldiers[idx]->FollowPath(_currentPath, movementStyle);
		}
		else if(!_soldiers[idx]->IsDead())
		{
			// Order this soldier to follow our point man
			_soldiers[idx]->Follow(_soldiers[_currentPointManIdx], _currentFormation, _currentFormationSpread, j++, movementStyle);
		}
	}
}

void
Squad::HandleAmbushOrder(AmbushOrder *order)
{
	_currentAction = Team::Ambushing;
	for(auto* soldier : _soldiers)
	{
		soldier->Ambush(order->Heading);
	}
}

void
Squad::HandleDefendOrder(DefendOrder *order)
{
	_currentAction = Team::Defending;
	for(auto* soldier : _soldiers)
	{
		soldier->Defend(order->Heading);
	}
}

void
Squad::ClearOrders()
{
	for(auto* soldier : _soldiers) {
		soldier->ClearOrders();
	}
	for(auto* vehicle : _vehicles) {
		vehicle->ClearOrders();
	}
}

Object *
Squad::GetSquadLeader()
{
	// XXX/GWS: This needs to work with vehicles too
	for(auto* soldier : _soldiers) {
		if(soldier->IsSquadLeader()) {
			return soldier;
		}
	}
	for(auto* vehicle : _vehicles) {
		if(vehicle->IsSquadLeader()) {
			return vehicle;
		}
	}
	assert(false);
	return nullptr;
}

Object *
Squad::GetPointMan()
{
	return _soldiers[_currentPointManIdx];
}

const std::string&
Squad::GetQualityDesc()
{
	return _squadQualityIcons[_quality];
}

void
Squad::UpdateInterfaceState(InterfaceState *state, int teamIdx, int unitIdx)
{
	UNREFERENCED_PARAMETER(unitIdx);
	state->SquadStates[teamIdx].Name = GetName();
	state->SquadStates[teamIdx].Icon = GetIconName();
	state->SquadStates[teamIdx].NumUnits = static_cast<int>(GetSoldiers()->size());
	state->SquadStates[teamIdx].ID = GetID();
	state->SquadStates[teamIdx].CurrentAction = _currentAction;
	state->SquadStates[teamIdx].Quality = GetQualityDesc();
	state->SquadStates[teamIdx].SelectedSoldierIdx = _selectedSoldierIdx;
	for(size_t j = 0; j < GetSoldiers()->size(); ++j) {
		(*GetSoldiers())[j]->UpdateInterfaceState(state, teamIdx, static_cast<int>(j));
		if(GetSquadLeader()->GetID() == (*GetSoldiers())[j]->GetID()) {
			state->SquadStates[teamIdx].SquadLeaderIdx = static_cast<int>(j);
		}
	}
	for(size_t j = 0; j < _vehicles.size(); ++j) {
		_vehicles[j]->UpdateInterfaceState(state, teamIdx, static_cast<int>(j));
		if(GetSquadLeader()->GetID() == _vehicles[j]->GetID()) {
			state->SquadStates[teamIdx].SquadLeaderIdx = static_cast<int>(j);
		}
	}
}

void
Squad::Kill()
{
	// XXX/GWS: This is supposed to kill this squad, but for now,
	// just kill a random soldier in it
	int i = rand() % static_cast<int>(_soldiers.size());
	_soldiers[i]->Kill();
}

bool 
Squad::Contains(int x, int y)
{
	for(auto* soldier : _soldiers) {
		if(soldier->Contains(x, y)) {
			return true;
		}
	}

	for(auto* vehicle : _vehicles) {
		if(vehicle->Contains(x, y)) {
			return true;
		}
	}

	return false;
}

bool
Squad::IsActive()
{
	for(auto* vehicle : _vehicles) {
		if(!vehicle->IsDestroyed() && !vehicle->IsAbandoned()) {
			return true;
		}
	}
	for(auto* soldier : _soldiers) {
		if(!soldier->IsDead()) {
			return true;
		}
	}
	return false;
}

void 
Squad::Highlight(Color *color)
{
	Object::Highlight(color);
	for(auto* soldier : _soldiers) {
		soldier->Highlight(color);
	}
	for(auto* vehicle : _vehicles) {
		vehicle->Highlight(color);
	}

}

void 
Squad::UnHighlight()
{
	Object::UnHighlight();
	for(auto* soldier : _soldiers) {
		soldier->UnHighlight();
	}
	for(auto* vehicle : _vehicles) {
		vehicle->UnHighlight();
	}
}

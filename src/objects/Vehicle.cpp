#include "./Vehicle.h"
#include <misc/Color.h>
#include <graphics/Screen.h>
#include <misc/Utilities.h>
#include <world/World.h>
#include <objects/Soldier.h>
#include <objects/Squad.h>
#include <objects/Weapon.h>
#include <graphics/Effect.h>
#include <graphics/Widget.h>
#include <application/Globals.h>
#include <orders/DefendOrder.h>
#include <orders/AmbushOrder.h>
#include <sound/Sound.h>
#include <assert.h>
#include <math.h>


#define NORMALIZE_ANGLE(a) (float)(((a) < 0) ? ((a)+2.0f*M_PI) : (((a)>2.0f*M_PI) ? (a)-2.0f*M_PI : (a)))
#define DA (M_PI/180.0f)

Vehicle::Vehicle(void)
: Object()
{
	_currentHullAngle = 0.0;
	_currentTurretAngle = 0.0;
	_turretRotating = false;
	_hullRotating = false;
	_numWeapons = 0;
	_numCrew = 0;
}

Vehicle::~Vehicle(void)
{
}

// Render this object to the screen
void 
Vehicle::Render(Screen *screen, Rect *clip)
{
	UNREFERENCED_PARAMETER(clip);
	Color white(255,255,255);
	screen->Blit(_hullGraphics->GetData(), 
		Position.x - screen->Origin.x - _hullGraphics->GetOriginX(), 
		Position.y - screen->Origin.y - _hullGraphics->GetOriginY(),
		_hullGraphics->GetWidth(), _hullGraphics->GetHeight(),
		_hullGraphics->GetWidth(), _hullGraphics->GetHeight(),
		_hullGraphics->GetDepth(), &white,
		_hullGraphics->GetOriginX(), _hullGraphics->GetOriginY(), _currentHullAngle);
	screen->Blit(_turretGraphics->GetData(), 
		Position.x - screen->Origin.x - _hullGraphics->GetOriginX() + _turretPosition.x - _turretGraphics->GetOriginX(), 
		Position.y - screen->Origin.y - _hullGraphics->GetOriginY() + _turretPosition.y - _turretGraphics->GetOriginY(),
		_turretGraphics->GetWidth(), _turretGraphics->GetHeight(),
		_turretGraphics->GetWidth(), _turretGraphics->GetHeight(),
		_turretGraphics->GetDepth(), &white, 
		_turretGraphics->GetOriginX(), _turretGraphics->GetOriginY(), _currentTurretAngle);

	// Render any effects
	for(auto& effect : _effects) {
		if(effect->IsDynamic()) {
			// Set the positions
			if(effect->IsPlaceOnTurret()) {
				// Find out my turret position
				Point p;
				Utilities::Rotate(&p, &_muzzlePosition, _currentTurretAngle);
				effect->SetPosition(Position.x-screen->Origin.x-p.x, Position.y- screen->Origin.x-p.y);
			} else {
				effect->SetPosition(Position.x, Position.y);
			}
		}
		effect->Render(screen);
	}

	if(IsHighlighted()) {
		Color black(0,0,0);
		Widget *w = g_Globals->World.Icons->GetWidget("Unit Highlighted Bracket");
		w->Render(screen, Position.x - screen->Origin.x, Position.y-screen->Origin.y, &black);
		delete w;
	} else if(IsSelected()) {
		Color white(255,255,255);
		Widget *w = g_Globals->World.Icons->GetWidget("Unit Selected Bracket");
		w->Render(screen, Position.x - screen->Origin.x, Position.y-screen->Origin.y, &white);
		delete w;
	}
}

// Simulate this object for dt milliseconds
void 
Vehicle::Simulate(long dt, World *world)
{
	UNREFERENCED_PARAMETER(world);
	// Check our current orders
	if(!_orders.empty()) {
		Order *order = _orders.front();
		bool handled = false;
		switch(order->GetType()) {
		case Orders::Move:
		case Orders::MoveFast:
		case Orders::Sneak:
			// This is a move order, let's head in that direction
			// TODO: Move should move tank in reverse (like old Close Combat)
			// TODO: Sneak should move slower than normal Move
			//       NOTE: Not implemented because vehicles only have one speed (MaxRoadSpeed)
			//       defined in Vehicles.xml, unlike infantry which have separate
			//       WalkingSpeed/RunningSpeed/CrawlingSpeed/SneakingSpeed in Soldiers.xml.
			//       To implement properly, either:
			//       1. Add MaxSneakSpeed/SneakAcceleration to Vehicles.xml (like infantry), or
			//       2. Use a hard-coded multiplier (e.g., Sneak = 50% of MaxRoadSpeed)
			// TODO: MoveFast could use a speed boost (currently same as normal Move)
			_currentAction = Unit::Moving;
			handled = HandleMoveOrder(dt, (MoveOrder *) order, Moving);
			break;
			case Orders::Fire:
				handled = HandleFireOrder((FireOrder *)order);
				break;
			case Orders::Destination:
				handled = HandleDestinationOrder((MoveOrder *)order);
				break;
		case Orders::Stop:
			handled = HandleStopOrder();
			break;
		case Orders::Defend:
			handled = HandleDefendOrder((DefendOrder *)order);
			break;
		case Orders::Ambush:
			handled = HandleAmbushOrder((AmbushOrder *)order);
			break;
			default:
				handled = true;
				break;
		}

		if(handled) {
			_orders.pop_front();
			order->Release();
		}
	}

	if(_turretRotating) {
		_currentTurretAngle +=(float)( _turretRotationDirection*((float)dt)*2.0f*M_PI / (16.0f*_turretRotationRate));
		_currentTurretAngle = (float)NORMALIZE_ANGLE(_currentTurretAngle);
		if(_currentTurretAngle<=(_turretTargetAngle+DA) && _currentTurretAngle>=(_turretTargetAngle-DA))
		{
			_turretRotating = false;
			_currentTurretAngle = _turretTargetAngle;
		}
	}
	assert(_currentTurretAngle <= 2.0f*M_PI && _currentTurretAngle >= 0.0f);

	if(_hullRotating) {
		_currentHullAngle += (float)(_hullRotationDirection*((float)dt)*2.0f*M_PI / (16.0f*_hullRotationRate));
		_currentHullAngle = (float)NORMALIZE_ANGLE(_currentHullAngle);
		if(_currentHullAngle<=(_hullTargetAngle+DA) && _currentHullAngle>=(_hullTargetAngle-DA))
		{
			_hullRotating = false;
			_currentHullAngle = _hullTargetAngle;
		}
	}
	assert(_currentHullAngle <= 2.0f*M_PI && _currentHullAngle >= 0.0f);

	// Let's do our movement
	PlanMovement(dt);

	// Update any effects (erase-remove idiom for unique_ptr)
	for(auto it = _effects.begin(); it != _effects.end(); ) {
		(*it)->Simulate(dt);
		if((*it)->IsCompleted()) {
			it = _effects.erase(it);
		} else {
			++it;
		}
	}

	// Update my weapons if I can
	for(int i = 0; i < _numWeapons; ++i) {
		_weapons[i]->Simulate(dt);
		if(_currentState == State::Firing)
		{
			if(_weapons[i]->CanFire()) {
				if(_weaponIsOnHull[i]) {
					if(!_hullRotating) {
						Shoot(_weapons[i], _currentTarget, _currentTargetType, _currentTargetX, _currentTargetY);
					}
				} else {
					if(!_turretRotating) {
						Shoot(_weapons[i], _currentTarget, _currentTargetType, _currentTargetX, _currentTargetY);
					}
				}
			} else if(_weapons[i]->IsEmpty()) {
				if(_weaponsNumClips[i] > 0) {
					_weapons[i]->Reload();
					--_weaponsNumClips[i];
				}
			}
		}
	}
}

void
Vehicle::PlanMovement(long dt)
{
	if(_moving && !_hullRotating)
	{
		// Go ahead and head towards our destination
		float secs = ((float)dt)/1000.0f;
		_velocity.x += -_acceleration*secs*sin(_currentHullAngle);
		_velocity.y += -_acceleration*secs*cos(_currentHullAngle);

		if(_velocity.Magnitude() > _maxRoadSpeed) { 
			_velocity.Normalize();
			_velocity.Multiply(_maxRoadSpeed);
		}

		// Now we need to try moving this object to its new position
		_position.x += _velocity.x*secs*(float)(g_Globals->World.Constants.PixelsPerMeter);
		_position.y += _velocity.y*secs*(float)(g_Globals->World.Constants.PixelsPerMeter);
		Position.x = (int)_position.x;
		Position.y = (int)_position.y;
	}
}

bool
Vehicle::HandleMoveOrder(long dt, MoveOrder *order, State newState)
{
	UNREFERENCED_PARAMETER(dt);
	// Head to the destination
	MoveOrder *o = new MoveOrder(order->X, order->Y, Orders::Destination);
	AddOrder(o);

	_moving = true;
	_currentState = newState;
	_destination.x = order->X;
	_destination.y = order->Y;
	_velocity.x = 0; // Stop moving!
	_velocity.y = 0;

	// Set the desired turret angle and hull angle to get us pointed there
	AimTurret(order->X, order->Y);
	
	return true;
}

bool
Vehicle::HandleFireOrder(FireOrder *order)
{
	// Stop moving
	//HandleStopOrder(NULL);

	// Set my state
	_currentState = State::Firing;
	_currentAction = Unit::Firing;

	// Set the current target and stuff
	_currentTarget = order->Target;
	_currentTargetType = order->TargetType;
	_currentTargetX = order->X;
	_currentTargetY = order->Y;

	// We need to line the damn turret up!
	AimTurret(order->X, order->Y);

	return true;
}

bool
Vehicle::HandleDestinationOrder(MoveOrder *order)
{
	Vector2 dist;
	dist.x = (float)(Position.x - order->X);
	dist.y = (float)(Position.y - order->Y);

	if(dist.Magnitude() < 5.01f) {
		// Clear marks and play sound if we're the squad leader (matches infantry behavior)
		if(IsSquadLeader() && _currentSquad != NULL) {
			_currentSquad->ClearMarks();
			g_Globals->World.Voices->GetSound("move completed")->Play();
		}
		AddOrder(new StopOrder());
		return true;
	}
	return false;
}

bool
Vehicle::HandleStopOrder()
{
	_moving = false;
	_currentState = Stopped;
	_currentAction = Unit::Defending;
	_velocity.x = 0; // Stop moving!
	_velocity.y = 0;
	return true;
}

bool
Vehicle::HandleDefendOrder(DefendOrder *order)
{
	// Stop the tank
	_moving = false;
	_currentState = Stopped;
	_currentAction = Unit::Defending;
	_velocity.x = 0;
	_velocity.y = 0;
	
	// Rotate hull and turret to face the specified direction
	AimTurret(order->Heading);
	
	return true;
}

bool
Vehicle::HandleAmbushOrder(AmbushOrder *order)
{
	// Stop the tank
	_moving = false;
	_currentState = Stopped;
	_currentAction = Unit::Ambushing;
	_velocity.x = 0;
	_velocity.y = 0;
	
	// Rotate hull and turret to face the specified direction
	AimTurret(order->Heading);
	
	return true;
}

Vehicle *
Vehicle::Clone()
{
	Vehicle *v = new Vehicle();
	v->_name = _name;
	v->_hullGraphics = _hullGraphics;
	v->_turretGraphics = _turretGraphics;
	v->_wreckGraphics = _wreckGraphics;
	v->_turretPosition.x = _turretPosition.x;
	v->_turretPosition.y = _turretPosition.y;
	v->_hullRotationRate = _hullRotationRate;
	v->_turretRotationRate = _turretRotationRate;
	return v;
}

void 
Vehicle::SetPosition(int x, int y)
{
	Object::SetPosition(x,y);
	_position.x = (float) x;
	_position.y = (float) y;
}

bool 
Vehicle::Select(int x, int y)
{
	if(Contains(x,y)) {
		Object::Select(true);
		return true;
	}
	return false;
}

bool 
Vehicle::Contains(int x, int y)
{
	Region r;
	r.points[0].x = Position.x - _hullGraphics->GetWidth()/2;
	r.points[0].y = Position.y - _hullGraphics->GetWidth()/2;
	r.points[1].x = Position.x + _hullGraphics->GetWidth()/2;
	r.points[1].y = Position.y - _hullGraphics->GetWidth()/2;
	r.points[2].x = Position.x + _hullGraphics->GetWidth()/2;
	r.points[2].y = Position.y + _hullGraphics->GetWidth()/2;
	r.points[3].x = Position.x - _hullGraphics->GetWidth()/2;
	r.points[3].y = Position.y + _hullGraphics->GetWidth()/2;
	return Screen::PointInRegion(x, y, &r);
}

void 
Vehicle::UpdateInterfaceState(InterfaceState *state, int teamIdx, int unitIdx)
{
	UNREFERENCED_PARAMETER(unitIdx);
	for(int i = 0; i < _numCrew; ++i) {
		Soldier *s = _crew[i].soldier;
		s->UpdateInterfaceState(state, teamIdx, i);
		state->SquadStates[teamIdx].UnitStates[i].NumRounds = _weapons[_crew[i].weaponSlot]->GetCurrentRounds() + _weapons[_crew[i].weaponSlot]->GetRoundsPerClip()*_weaponsNumClips[_crew[i].weaponSlot];
		state->SquadStates[teamIdx].NumUnits++;
	}

#if 0
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].Name, GetPersonalName());
	state->SquadStates[teamIdx].UnitStates[unitIdx].CurrentAction = GetCurrentAction();
	state->SquadStates[teamIdx].UnitStates[unitIdx].CurrentStatus = GetCurrentStatus();
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].WeaponIcon, _primaryWeapon->GetIconName());
	state->SquadStates[teamIdx].UnitStates[unitIdx].NumRounds = _primaryWeapon->GetCurrentRounds() + _primaryWeapon->GetRoundsPerClip()*_primaryWeaponNumClips;
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].Title, _title);
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].Rank, _rank);
#endif
}

void 
Vehicle::AddWeapon(Weapon *weapon, int slot, int numClips, bool hull)
{
	if(slot < 0) {
		// Add to the end
		slot = _numWeapons;
	} 

	assert(slot >= 0 && slot < MAX_WEAPONS_PER_VEHICLE);
	_weapons[slot] = weapon;
	_weaponIsOnHull[slot] = hull;
	_weaponsNumClips[slot] = numClips;
	_numWeapons++;
}

void
Vehicle::Shoot(Weapon *weapon, Object *target, Target::Type targetType, int targetX, int targetY)
{
	Direction effectHeading=North;
	switch(targetType) {
		case Target::Soldier:
			if(target != NULL) {
				// Make sure my target is not already dead or dying!
				Soldier *s = (Soldier *) target;
				if(s->IsDead()) 
				{
					_currentTarget = s->GetSquad();
					_currentTargetType = Target::Squad;
					return;
				}

				effectHeading = Utilities::FindHeading(Position.x, Position.y, _currentTarget->Position.x, _currentTarget->Position.y);
#if 0
				if(g_World->CalculateShot(this, _currentTarget, weapon))
				{
					// We killed the guy, so find another target next time
					_currentTarget = ((Soldier *)_currentTarget)->GetSquad();
					_currentTargetType = Target::Squad;
				}
#endif
			}
			break;
		case Target::Squad:
			// Find a target in the squad
			_currentTarget = FindTarget((Squad *) target);
			_currentTargetType = Target::Soldier;
			if(_currentTarget == NULL) {
				_currentAction = Unit::NoTarget;
				_currentState = State::Stopped;
			}
			return;
		case Target::Area:
			// Set my heading
			effectHeading = Utilities::FindHeading(Position.x, Position.y, targetX, targetY);
			break;
	}
	weapon->Fire();
			
	_currentState = State::Firing;
	_currentAction = Unit::Firing;
	_effects.push_back(std::unique_ptr<Effect>(g_Globals->World.Effects->GetEffect(weapon->GetEffect(effectHeading))));

	if(weapon->IsGroundShaker()) {
		Effect *e = g_Globals->World.Effects->GetEffect("Dust Cloud");
		e->SetPosition(Position.x, Position.y);
		_effects.push_back(std::unique_ptr<Effect>(e));
	}
}

Soldier *
Vehicle::FindTarget(Squad *squad)
{
	if(NULL == squad) {
		return NULL;
	}

	bool anyAlive = false;
	std::vector<Soldier*> *soldiers = squad->GetSoldiers();
	for(int i = static_cast<int>(soldiers->size())-1; i >= 0 ; --i) {
		if(!(*soldiers)[i]->IsDead())
		{
			anyAlive = true;
			break;
		}
	}
	if(anyAlive) {
		Soldier *o;
		for(;;)
		{
			if(!((o = (*soldiers)[rand()%static_cast<int>(soldiers->size())])->IsDead())) {
				return o;
			}
		}
	}
	return NULL;
}

void
Vehicle::AddCrew(Soldier *soldier, int slot)
{
	if(slot < 0) {
		slot = _numWeapons-1;
	}
	soldier->InsertWeapon(GetWeapon(slot), GetWeaponNumClips(slot));

	// Add this soldier
	_crew[_numCrew].soldier = soldier;
	_crew[_numCrew].weaponSlot = slot;
	_numCrew++;
}

void
Vehicle::AimTurret(int x, int y)
{
	_hullRotating = true;
	_turretRotating = true;
	float angle = Utilities::FindAngle(Position.x, Position.y, x, y);
	if(angle < M_PI/2.0f) {
		angle += (float)(3.0f*M_PI/2.0f);
	} else {
		angle -= (float)(M_PI/2.0f);
	}
	_turretTargetAngle = angle;
	_hullTargetAngle = _turretTargetAngle;

	// Set the direction of rotation
	float ta1 = _currentTurretAngle - _turretTargetAngle;
	float ta2 = -ta1;
	ta1 = NORMALIZE_ANGLE(ta1);
	ta2 = NORMALIZE_ANGLE(ta2);
	_turretRotationDirection = (ta1 < ta2) ? -1.0f : 1.0f;

	ta1 = _currentHullAngle - _hullTargetAngle;
	ta2 = -ta1;
	ta1 = NORMALIZE_ANGLE(ta1);
	ta2 = NORMALIZE_ANGLE(ta2);
	_hullRotationDirection = (ta1 < ta2) ? -1.0f : 1.0f;
}

void
Vehicle::AimTurret(Direction dir)
{
	_hullRotating = true;
	_turretRotating = true;
	
	// Convert Direction enum to radians (0 = South, going clockwise)
	float targetAngle = (float)(dir * 2.0f * M_PI / 8.0f);
	_hullTargetAngle = targetAngle;
	_turretTargetAngle = targetAngle;
	
	// Calculate hull rotation direction (shortest path)
	float hullDiff = _hullTargetAngle - _currentHullAngle;
	while(hullDiff > M_PI) hullDiff -= 2.0f * M_PI;
	while(hullDiff < -M_PI) hullDiff += 2.0f * M_PI;
	_hullRotationDirection = (hullDiff > 0) ? 1.0f : -1.0f;
	
	// Calculate turret rotation direction (shortest path)
	float turretDiff = _turretTargetAngle - _currentTurretAngle;
	while(turretDiff > M_PI) turretDiff -= 2.0f * M_PI;
	while(turretDiff < -M_PI) turretDiff += 2.0f * M_PI;
	_turretRotationDirection = (turretDiff > 0) ? 1.0f : -1.0f;
}


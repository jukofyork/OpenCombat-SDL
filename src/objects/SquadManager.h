#pragma once

#include <misc/Array.h>

class AnimationManager;
class SoldierManager;
class Squad;
struct SquadTemplate;
class VehicleManager;
class WeaponManager;

class SquadManager
{
public:
	SquadManager(void);
	virtual ~SquadManager(void);

	// Loads a group of soldiers from a configuration file
	void LoadSquads(char *fileName);

	// Creates an instance of a specific soldier
	Squad *CreateSquad(const char *squadName, SoldierManager *soldierManager, VehicleManager *vehicleManager, AnimationManager *animationManager, WeaponManager *weaponManager);

protected:
	// The list of squad templates that we are managing
	Array<SquadTemplate> _squads;
};

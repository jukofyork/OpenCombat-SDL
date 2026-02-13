#pragma once

#include <misc/Array.h>

class Weapon;

struct WeaponTemplate
{
	WeaponTemplate() { ShakeGround = false; }
	char Name[32];
	char Icon[32];
	char Sound[64];
	char Animation[64];
	int NumRounds;
	int ReloadTimeClip;
	int ReloadTimeChamber;
	int RoundsPerBurst;
	int TimeToFire;
	bool ShakeGround;
};

class WeaponManager
{
public:
	WeaponManager(void);
	virtual ~WeaponManager(void);

	// Loads weapons from an XML file
	void LoadWeapons(char *fileName);

	// Retrieves a weapon by name
	Weapon *GetWeapon(char *weaponName);

protected:
	// The array of weapon templates
	Array<WeaponTemplate> _weapons;
};

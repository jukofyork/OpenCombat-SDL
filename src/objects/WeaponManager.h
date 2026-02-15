#pragma once

#include <string>
#include <misc/Array.h>

class Weapon;

struct WeaponTemplate
{
	WeaponTemplate() { ShakeGround = false; }
	std::string Name;
	std::string Icon;
	std::string Sound;
	std::string Animation;
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

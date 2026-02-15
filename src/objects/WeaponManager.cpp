#include "./WeaponManager.h"
#include <misc/tinyxml2.h>

#include <objects/Weapon.h>

using namespace tinyxml2;

WeaponManager::WeaponManager(void)
{
}

WeaponManager::~WeaponManager(void)
{
}

void
WeaponManager::LoadWeapons(char *fileName)
{

	XMLDocument doc;
	if (doc.LoadFile(fileName) != XML_SUCCESS) {
		printf("Failed to load weapons file: %s\n", fileName);
		return;
	}
	
	XMLElement* root = doc.FirstChildElement("Weapons");
	if (!root) return;
	
	for (XMLElement* weaponElem = root->FirstChildElement("Weapon"); 
		 weaponElem != nullptr; 
		 weaponElem = weaponElem->NextSiblingElement("Weapon")) 
	{
		WeaponTemplate* weapon = new WeaponTemplate();
		
		// Get attributes from <Weapon> element
		const char* shakeAttr = weaponElem->Attribute("earthShaker");
		if (shakeAttr && strcmp(shakeAttr, "true") == 0) {
			weapon->ShakeGround = true;
		}
		
		// Parse fields
		XMLElement* nameElem = weaponElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			weapon->Name = nameElem->GetText();
		}
		
		XMLElement* iconElem = weaponElem->FirstChildElement("Icon");
		if (iconElem && iconElem->GetText()) {
			weapon->Icon = iconElem->GetText();
		}
		
		XMLElement* soundElem = weaponElem->FirstChildElement("Sound");
		if (soundElem && soundElem->GetText()) {
			weapon->Sound = soundElem->GetText();
		}
		
		XMLElement* animElem = weaponElem->FirstChildElement("Animation");
		if (animElem && animElem->GetText()) {
			weapon->Animation = animElem->GetText();
		}
		
		XMLElement* timeElem = weaponElem->FirstChildElement("TimeToFire");
		if (timeElem && timeElem->GetText()) {
			weapon->TimeToFire = atoi(timeElem->GetText());
		}
		
		XMLElement* burstElem = weaponElem->FirstChildElement("RoundsPerBurst");
		if (burstElem && burstElem->GetText()) {
			weapon->RoundsPerBurst = atoi(burstElem->GetText());
		}
		
		XMLElement* roundsElem = weaponElem->FirstChildElement("RoundsPerClip");
		if (roundsElem && roundsElem->GetText()) {
			weapon->NumRounds = atoi(roundsElem->GetText());
		}
		
		XMLElement* reloadClipElem = weaponElem->FirstChildElement("ReloadTimeClip");
		if (reloadClipElem && reloadClipElem->GetText()) {
			weapon->ReloadTimeClip = atoi(reloadClipElem->GetText());
		}
		
		XMLElement* reloadChamberElem = weaponElem->FirstChildElement("ReloadTimeChamber");
		if (reloadChamberElem && reloadChamberElem->GetText()) {
			weapon->ReloadTimeChamber = atoi(reloadChamberElem->GetText());
		}
		
		_weapons.Add(weapon);
	}
}

Weapon *
WeaponManager::GetWeapon(char *weaponName)
{
	for(int i = 0; i < _weapons.Count; ++i) {
		if(strcmp(weaponName, _weapons.Items[i]->Name.c_str()) == 0) {
			Weapon *w = new Weapon();
			
			w->_numRounds = _weapons.Items[i]->NumRounds;
			w->_totalRounds = _weapons.Items[i]->NumRounds;
			w->_name = _weapons.Items[i]->Name;
			w->_iconName = _weapons.Items[i]->Icon;
			w->_sound = _weapons.Items[i]->Sound;
			w->SetEffect(_weapons.Items[i]->Animation);
			w->_reloadTimeChamber = _weapons.Items[i]->ReloadTimeChamber;
			w->_reloadTimeClip = _weapons.Items[i]->ReloadTimeClip;
			w->_roundsPerBurst = _weapons.Items[i]->RoundsPerBurst;
			w->_timeToFire = _weapons.Items[i]->TimeToFire;
			w->SetGroundShaker(_weapons.Items[i]->ShakeGround);
			return w;
		}
	}
	return NULL;
}

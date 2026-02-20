#include "./WeaponManager.h"
#include <misc/tinyxml2.h>
#include <misc/Error.h>

#include <filesystem>
#include <objects/Weapon.h>

using namespace tinyxml2;

WeaponManager::WeaponManager(void)
{
}

WeaponManager::~WeaponManager(void)
{
}

void
WeaponManager::LoadWeapons(const std::filesystem::path& fileName)
{

	XMLDocument doc;
	if (doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
		ERROR("Failed to load weapons file: " + fileName.string());
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
		
		_weapons.push_back(weapon);
	}
}

Weapon *
WeaponManager::GetWeapon(const std::string& weaponName)
{
	for(auto* weaponTemplate : _weapons) {
		if(weaponName == weaponTemplate->Name) {
			Weapon *w = new Weapon();
			
			w->_numRounds = weaponTemplate->NumRounds;
			w->_totalRounds = weaponTemplate->NumRounds;
			w->_name = weaponTemplate->Name;
			w->_iconName = weaponTemplate->Icon;
			w->_sound = weaponTemplate->Sound;
			w->SetEffect(weaponTemplate->Animation);
			w->_reloadTimeChamber = weaponTemplate->ReloadTimeChamber;
			w->_reloadTimeClip = weaponTemplate->ReloadTimeClip;
			w->_roundsPerBurst = weaponTemplate->RoundsPerBurst;
			w->_timeToFire = weaponTemplate->TimeToFire;
			w->SetGroundShaker(weaponTemplate->ShakeGround);
			return w;
		}
	}
	return nullptr;
}

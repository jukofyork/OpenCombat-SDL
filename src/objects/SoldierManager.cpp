#include "./SoldierManager.h"
#include "misc/tinyxml2.h"
#include "misc/Error.h"

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <misc/Color.h>
#include <graphics/AnimationManager.h>
#include <objects/WeaponManager.h>
#include <fstream>
#include <string>
#include <filesystem>

using namespace tinyxml2;

// SoldierState structure for animation states
struct SoldierState {
	std::string Name;
	std::string Animation;
};

// SoldierTemplate class definition
class SoldierTemplate {
public:
	SoldierTemplate() {
		CanFire = false;
		CanMove = false;
		CanMoveFast = false;
		CanSneak = false;
		CanDefend = false;
		CanAmbush = false;
		CanSmoke = false;
		WalkingSpeed = 0;
		WalkingAcceleration = 0;
		RunningSpeed = 0;
		RunningAcceleration = 0;
		CrawlingSpeed = 0;
		SneakingSpeed = 0;
		SneakingAcceleration = 0;
		PrimaryWeaponNumClips = 0;
	}

	float WalkingSpeed;
	float WalkingAcceleration;
	float RunningSpeed;
	float RunningAcceleration;
	float CrawlingSpeed;
	float SneakingSpeed;
	float SneakingAcceleration;

	bool CanFire;
	bool CanMove;
	bool CanMoveFast;
	bool CanSneak;
	bool CanDefend;
	bool CanAmbush;
	bool CanSmoke;

	std::string Name;

	std::string PrimaryWeapon;
	int PrimaryWeaponNumClips;

	std::vector<SoldierState*> States;
};

SoldierManager::SoldierManager(void)
{
}

SoldierManager::~SoldierManager(void)
{
}

void
SoldierManager::LoadSoldiers(const std::filesystem::path& fileName, const std::filesystem::path& soldierNames)
{

	XMLDocument doc;
	if (doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
		ERROR("Failed to load soldiers file: " + fileName.string());
	}
	
	XMLElement* root = doc.FirstChildElement("Soldiers");
	if (!root) return;
	
	for (XMLElement* soldierElem = root->FirstChildElement("Soldier"); 
		 soldierElem != nullptr; 
		 soldierElem = soldierElem->NextSiblingElement("Soldier")) 
	{
		SoldierTemplate* soldier = new SoldierTemplate();
		
		// Parse simple fields
		XMLElement* nameElem = soldierElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			soldier->Name = nameElem->GetText();
		}
		
		XMLElement* weaponElem = soldierElem->FirstChildElement("PrimaryWeapon");
		if (weaponElem && weaponElem->GetText()) {
			soldier->PrimaryWeapon = weaponElem->GetText();
		}
		
		XMLElement* clipsElem = soldierElem->FirstChildElement("PrimaryWeaponNumClips");
		if (clipsElem && clipsElem->GetText()) {
			soldier->PrimaryWeaponNumClips = atoi(clipsElem->GetText());
		}
		
		// Parse speeds and accelerations from Attributes section
		XMLElement* attrsElem = soldierElem->FirstChildElement("Attributes");
		if (attrsElem) {
			XMLElement* walkElem = attrsElem->FirstChildElement("WalkingSpeed");
			if (walkElem && walkElem->GetText()) {
				soldier->WalkingSpeed = (float)atof(walkElem->GetText());
			}
			
			XMLElement* walkAccElem = attrsElem->FirstChildElement("WalkingAcceleration");
			if (walkAccElem && walkAccElem->GetText()) {
				soldier->WalkingAcceleration = (float)atof(walkAccElem->GetText());
			} else {
			}
			
			XMLElement* runElem = attrsElem->FirstChildElement("RunningSpeed");
			if (runElem && runElem->GetText()) {
				soldier->RunningSpeed = (float)atof(runElem->GetText());
			}
			
			XMLElement* runAccElem = attrsElem->FirstChildElement("RunningAcceleration");
			if (runAccElem && runAccElem->GetText()) {
				soldier->RunningAcceleration = (float)atof(runAccElem->GetText());
			} else {
			}
			
			XMLElement* crawlElem = attrsElem->FirstChildElement("CrawlingSpeed");
			if (crawlElem && crawlElem->GetText()) {
				soldier->CrawlingSpeed = (float)atof(crawlElem->GetText());
			}
			
			XMLElement* sneakElem = attrsElem->FirstChildElement("SneakingSpeed");
			if (sneakElem && sneakElem->GetText()) {
				soldier->SneakingSpeed = (float)atof(sneakElem->GetText());
			}
			
			XMLElement* sneakAccElem = attrsElem->FirstChildElement("SneakingAcceleration");
			if (sneakAccElem && sneakAccElem->GetText()) {
				soldier->SneakingAcceleration = (float)atof(sneakAccElem->GetText());
			} else {
			}
			
			// Parse boolean flags (element exists = true)
			if (attrsElem->FirstChildElement("CanMove")) soldier->CanMove = true;
			if (attrsElem->FirstChildElement("CanMoveFast")) soldier->CanMoveFast = true;
			if (attrsElem->FirstChildElement("CanFire")) soldier->CanFire = true;
			if (attrsElem->FirstChildElement("CanDefend")) soldier->CanDefend = true;
			if (attrsElem->FirstChildElement("CanSmoke")) soldier->CanSmoke = true;
			if (attrsElem->FirstChildElement("CanAmbush")) soldier->CanAmbush = true;
			if (attrsElem->FirstChildElement("CanSneak")) soldier->CanSneak = true;
		} else {
		}
		if (soldierElem->FirstChildElement("CanSmoke")) soldier->CanSmoke = true;
		if (soldierElem->FirstChildElement("CanSneak")) soldier->CanSneak = true;
		
		// Parse nested States
		XMLElement* statesElem = soldierElem->FirstChildElement("States");
		if (statesElem) {
			for (XMLElement* stateElem = statesElem->FirstChildElement("State");
				 stateElem != nullptr;
				 stateElem = stateElem->NextSiblingElement("State"))
			{
				SoldierState* state = new SoldierState();
				
				XMLElement* stateNameElem = stateElem->FirstChildElement("Name");
				if (stateNameElem && stateNameElem->GetText()) {
					state->Name = stateNameElem->GetText();
				}
				
				XMLElement* animElem = stateElem->FirstChildElement("Animation");
				if (animElem && animElem->GetText()) {
					state->Animation = animElem->GetText();
				}
				
				soldier->States.push_back(state);
			}
		}
		
		_soldiers.push_back(soldier);
	}

	// Now read in the soldier names file
	std::ifstream fp(soldierNames.c_str());
	std::string line;
	srand(time(nullptr));
	while(std::getline(fp, line)) {
		if(!line.empty() && line[0] == '#') {
			break;
		}

		assert(line.length() < 32);
		_soldierNames.push_back(line);
	}
	fp.close();
}

Soldier *
SoldierManager::CreateSoldier(const std::string& soldierType, AnimationManager *animationManager, WeaponManager *weaponManager)
{
	for(auto* t : _soldiers) {
		if(t->Name == soldierType) {
			// Create a soldier of this type
			Soldier *s = new Soldier();
			s->_name = soldierType;

			// Give this soldier a personal name
			s->_personalName = _soldierNames[rand()%_soldierNames.size()];

			// Get the primary weapon
			s->_weapons[0] = weaponManager->GetWeapon(t->PrimaryWeapon);
			s->_currentWeaponIdx = 0;
			s->_numWeapons = 1;
			s->_weaponsNumClips[0] = t->PrimaryWeaponNumClips;

			// Read in the action state flags
			s->_canAmbush = t->CanAmbush;
			s->_canDefend = t->CanDefend;
			s->_canFire = t->CanFire;
			s->_canMove = t->CanMove;
			s->_canMoveFast = t->CanMoveFast;
			s->_canSmoke = t->CanSmoke;
			s->_canSneak = t->CanSneak;
			s->_currentHeading = South;

			// Now read in the speeds, accelerations, and animations for each state
			s->_animations[Soldier::AnimationState::Standing] = GetAnimation(animationManager, "Standing", t);
			s->_animations[Soldier::AnimationState::StandingFiring] = GetAnimation(animationManager, "Standing Firing", t);
			s->_animations[Soldier::AnimationState::StandingReloading] = GetAnimation(animationManager, "Standing Reloading", t);
			s->_animations[Soldier::AnimationState::Prone] = GetAnimation(animationManager, "Prone", t);
			s->_animations[Soldier::AnimationState::ProneFiring] = GetAnimation(animationManager, "Prone Firing", t);
			s->_animations[Soldier::AnimationState::ProneReloading] = GetAnimation(animationManager, "Prone Reloading", t);
			s->_animations[Soldier::AnimationState::DyingBlownUp] = GetAnimation(animationManager, "Dying Blown Up", t);
			s->_animations[Soldier::AnimationState::DyingBackward] = GetAnimation(animationManager, "Dying Backward", t);
			s->_animations[Soldier::AnimationState::DyingForward] = GetAnimation(animationManager, "Dying Forward", t);
			s->_animations[Soldier::AnimationState::Dead] = GetAnimation(animationManager, "Dead", t);
			s->_animations[Soldier::AnimationState::StandingUp] = GetAnimation(animationManager, "Standing Up", t);
			
			s->_animations[Soldier::AnimationState::LyingDown] = GetAnimation(animationManager, "Standing Up", t);
			s->_animations[Soldier::AnimationState::LyingDown]->SetReverse(true);

			s->_walkingAccel = t->WalkingAcceleration;
			s->_animations[Soldier::AnimationState::Walking] = GetAnimation(animationManager, "Walking", t);

			s->_crawlingAccel = t->SneakingAcceleration;
			s->_animations[Soldier::AnimationState::Sneaking] = GetAnimation(animationManager, "Sneaking", t);
			
			s->_runningAccel = t->RunningAcceleration;
			s->_animations[Soldier::AnimationState::Running] = GetAnimation(animationManager, "Running", t);

			return s;
		}
	}
	return nullptr;
}

Animation *
SoldierManager::GetAnimation(AnimationManager *animationManager, const std::string& name, SoldierTemplate *tplate)
{
	for(auto* state : tplate->States) {
		if(state->Name == name) {
			return animationManager->GetAnimation(state->Animation);
		}
	}
	return nullptr;
}

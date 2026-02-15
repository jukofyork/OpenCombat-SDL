#include "./SoldierManager.h"
#include "misc/tinyxml2.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <misc/Color.h>
#include <graphics/AnimationManager.h>
#include <objects/WeaponManager.h>
#include <fstream>
#include <string>

using namespace tinyxml2;

// SoldierState structure for animation states
struct SoldierState {
	char Name[MAX_NAME];
	char Animation[MAX_NAME];
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

	char Name[MAX_NAME];

	char PrimaryWeapon[MAX_NAME];
	int PrimaryWeaponNumClips;

	Array<SoldierState> States;
};

SoldierManager::SoldierManager(void)
{
}

SoldierManager::~SoldierManager(void)
{
}

void
SoldierManager::LoadSoldiers(char *fileName, char *soldierNames)
{

	XMLDocument doc;
	if (doc.LoadFile(fileName) != XML_SUCCESS) {
		printf("Failed to load soldiers file: %s\n", fileName);
		return;
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
			strcpy(soldier->Name, nameElem->GetText());
		}
		
		XMLElement* weaponElem = soldierElem->FirstChildElement("PrimaryWeapon");
		if (weaponElem && weaponElem->GetText()) {
			strcpy(soldier->PrimaryWeapon, weaponElem->GetText());
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
					strcpy(state->Name, stateNameElem->GetText());
				}
				
				XMLElement* animElem = stateElem->FirstChildElement("Animation");
				if (animElem && animElem->GetText()) {
					sprintf(state->Animation, "%s", animElem->GetText());
				}
				
				soldier->States.Add(state);
			}
		}
		
		_soldiers.Add(soldier);
	}

	// Now read in the soldier names file
	std::ifstream fp(soldierNames);
	std::string line;
	srand(time(NULL));
	while(std::getline(fp, line)) {
		if(!line.empty() && line[0] == '#') {
			break;
		}

		assert(line.length() < 32);
		_soldierNames.Add(strdup(line.c_str()));
	}
	fp.close();
}

Soldier *
SoldierManager::CreateSoldier(char *soldierType, AnimationManager *animationManager, WeaponManager *weaponManager)
{
	for(int i = 0; i < _soldiers.Count; ++i) {
		SoldierTemplate *t = _soldiers.Items[i];
		if(strcmp(t->Name, soldierType) == 0) {
			// Create a soldier of this type
			Soldier *s = new Soldier();
			strcpy(s->_name, soldierType);

			// Give this soldier a personal name
			strcpy(s->_personalName, _soldierNames.Items[rand()%_soldierNames.Count]);

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
	return NULL;
}

Animation *
SoldierManager::GetAnimation(AnimationManager *animationManager, char *name, SoldierTemplate *tplate)
{
	for(int i = 0; i < tplate->States.Count; ++i) {
		if(strcmp(name, tplate->States.Items[i]->Name) == 0) {
			return animationManager->GetAnimation(tplate->States.Items[i]->Animation);
		}
	}
	return NULL;
}

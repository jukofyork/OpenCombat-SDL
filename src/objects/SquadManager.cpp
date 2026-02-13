#include "./SquadManager.h"
#include "misc/tinyxml2.h"
#include <objects/SoldierManager.h>
#include <objects/VehicleManager.h>
#include <objects/WeaponManager.h>
#include <objects/Squad.h>
#include <stdio.h>
#include <assert.h>
#include <strings.h>

#define MAX_SOLDIERS_IN_SQUAD 32
#define MAX_VEHICLES_IN_SQUAD 1

struct SquadSoldierAttributes {
	SquadSoldierAttributes() { Slot=-1; }
	char Type[256];
	char Title[32];
	char Rank[32];
	char Camo[64];
	int Slot;
};

struct SquadVehicleAttributes {
	SquadVehicleAttributes() { NumSoldiers=0; }
	int NumSoldiers;
	SquadSoldierAttributes Soldiers[MAX_SOLDIERS_IN_SQUAD];
	char Type[64];
};

struct SquadTemplate {
	SquadTemplate() { NumSoldiers = NumVehicles = 0; }
	int  NumSoldiers;
	int  NumVehicles;
	SquadSoldierAttributes Soldiers[MAX_SOLDIERS_IN_SQUAD];
	SquadVehicleAttributes Vehicles[MAX_VEHICLES_IN_SQUAD];
	char Name[256];
	char IconName[32];
};

using namespace tinyxml2;

SquadManager::SquadManager(void)
{
}

SquadManager::~SquadManager(void)
{
}

// Loads a group of soldiers from a configuration file
void
SquadManager::LoadSquads(char *fileName)
{

	XMLDocument doc;
	if (doc.LoadFile(fileName) != XML_SUCCESS) {
		printf("Failed to load squads file: %s\n", fileName);
		return;
	}
	
	XMLElement* root = doc.FirstChildElement("Squads");
	if (!root) root = doc.FirstChildElement("Squad"); // Handle both root names
	if (!root) return;
	
	for (XMLElement* squadElem = root->FirstChildElement("Squad");
		 squadElem != nullptr;
		 squadElem = squadElem->NextSiblingElement("Squad"))
	{
		SquadTemplate* squad = new SquadTemplate();
		
		// Parse Name
		XMLElement* nameElem = squadElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			assert(strlen(nameElem->GetText()) < 32);
			strcpy(squad->Name, nameElem->GetText());
		}
		
		// Parse Icon
		XMLElement* iconElem = squadElem->FirstChildElement("Icon");
		if (iconElem && iconElem->GetText()) {
			assert(strlen(iconElem->GetText()) <= 32);
			strcpy(squad->IconName, iconElem->GetText());
		}
		
		// Parse Soldiers directly under Squad
		for (XMLElement* soldierElem = squadElem->FirstChildElement("Soldier");
			 soldierElem != nullptr;
			 soldierElem = soldierElem->NextSiblingElement("Soldier"))
		{
			if (squad->NumSoldiers >= MAX_SOLDIERS_IN_SQUAD) break;
			
			XMLElement* titleElem = soldierElem->FirstChildElement("Title");
			if (titleElem && titleElem->GetText()) {
				assert(strlen(titleElem->GetText()) < 32);
				strcpy(squad->Soldiers[squad->NumSoldiers].Title, titleElem->GetText());
			}
			
			XMLElement* rankElem = soldierElem->FirstChildElement("Rank");
			if (rankElem && rankElem->GetText()) {
				assert(strlen(rankElem->GetText()) < 32);
				strcpy(squad->Soldiers[squad->NumSoldiers].Rank, rankElem->GetText());
			}
			
			XMLElement* typeElem = soldierElem->FirstChildElement("Type");
			if (typeElem && typeElem->GetText()) {
				assert(strlen(typeElem->GetText()) < 64);
				strcpy(squad->Soldiers[squad->NumSoldiers].Type, typeElem->GetText());
			}
			
			XMLElement* camoElem = soldierElem->FirstChildElement("Camo");
			if (camoElem && camoElem->GetText()) {
				assert(strlen(camoElem->GetText()) < 64);
				strcpy(squad->Soldiers[squad->NumSoldiers].Camo, camoElem->GetText());
			}
			
			squad->NumSoldiers++;
		}
		
		// Parse Vehicles
		for (XMLElement* vehicleElem = squadElem->FirstChildElement("Vehicle");
			 vehicleElem != nullptr;
			 vehicleElem = vehicleElem->NextSiblingElement("Vehicle"))
		{
			if (squad->NumVehicles >= MAX_VEHICLES_IN_SQUAD) break;
			
			SquadVehicleAttributes* vehicle = &squad->Vehicles[squad->NumVehicles];
			
			XMLElement* typeElem = vehicleElem->FirstChildElement("Type");
			if (typeElem && typeElem->GetText()) {
				assert(strlen(typeElem->GetText()) < 64);
				strcpy(vehicle->Type, typeElem->GetText());
			}
			
			// Parse Soldiers inside Vehicle
			for (XMLElement* soldierElem = vehicleElem->FirstChildElement("Soldier");
				 soldierElem != nullptr;
				 soldierElem = soldierElem->NextSiblingElement("Soldier"))
			{
				if (vehicle->NumSoldiers >= MAX_SOLDIERS_IN_SQUAD) break;
				
				const char* slotAttr = soldierElem->Attribute("slot");
				if (slotAttr) {
					vehicle->Soldiers[vehicle->NumSoldiers].Slot = atoi(slotAttr);
				}
				
				XMLElement* titleElem = soldierElem->FirstChildElement("Title");
				if (titleElem && titleElem->GetText()) {
					assert(strlen(titleElem->GetText()) < 32);
					strcpy(vehicle->Soldiers[vehicle->NumSoldiers].Title, titleElem->GetText());
				}
				
				XMLElement* rankElem = soldierElem->FirstChildElement("Rank");
				if (rankElem && rankElem->GetText()) {
					assert(strlen(rankElem->GetText()) < 32);
					strcpy(vehicle->Soldiers[vehicle->NumSoldiers].Rank, rankElem->GetText());
				}
				
				XMLElement* soldierTypeElem = soldierElem->FirstChildElement("Type");
				if (soldierTypeElem && soldierTypeElem->GetText()) {
					assert(strlen(soldierTypeElem->GetText()) < 64);
					strcpy(vehicle->Soldiers[vehicle->NumSoldiers].Type, soldierTypeElem->GetText());
				}
				
				XMLElement* camoElem = soldierElem->FirstChildElement("Camo");
				if (camoElem && camoElem->GetText()) {
					assert(strlen(camoElem->GetText()) < 64);
					strcpy(vehicle->Soldiers[vehicle->NumSoldiers].Camo, camoElem->GetText());
				}
				
				vehicle->NumSoldiers++;
			}
			
			squad->NumVehicles++;
		}
		
		_squads.Add(squad);
	}
}

// Creates an instance of a specific soldier
Squad *
SquadManager::CreateSquad(char *squadType, SoldierManager *soldierManager, VehicleManager *vehicleManager, AnimationManager *animationManager, WeaponManager *weaponManager)
{
	for(int i = 0; i < _squads.Count; ++i) {
		if(strcmp(squadType, _squads.Items[i]->Name) == 0) {
			Squad *squad = new Squad();
			strcpy(squad->_iconName, _squads.Items[i]->IconName);
			strcpy(squad->_name, squadType);
			// XXX/GWS: Fix this random team quality thing here!
			squad->_quality = (Squad::Quality) (rand() % Squad::NumQuality);

			// Add all of the soldiers
			for(int j = 0; j < _squads.Items[i]->NumSoldiers; ++j) {
				Soldier *s = soldierManager->CreateSoldier(_squads.Items[i]->Soldiers[j].Type, animationManager, weaponManager);
				s->SetTitle(_squads.Items[i]->Soldiers[j].Title);
				s->SetRank(_squads.Items[i]->Soldiers[j].Rank);
				s->SetCamouflage(_squads.Items[i]->Soldiers[j].Camo);
				s->SetFormationPosition(j);
				// XXX/GWS: Need better determination of the squad leader
				if(strcasecmp(_squads.Items[i]->Soldiers[j].Title, "Leader") == 0
					|| strcasecmp(_squads.Items[i]->Soldiers[j].Title, "Gunner") == 0)
				{
					s->SetSquadLeader(true);
				}
				squad->_soldiers.Add(s);
				s->SetSquad(squad);
			}

			// Add all of the vehicles
			for(int j = 0; j < _squads.Items[i]->NumVehicles; ++j)
			{
				Vehicle *v = vehicleManager->GetVehicle(_squads.Items[i]->Vehicles[j].Type);
				v->SetSquadLeader(true); // XXX/GWS: Better determination here

				// Now add soldiers to this vehicle
				for(int k = 0; k < _squads.Items[i]->Vehicles[j].NumSoldiers; ++k) {
					Soldier *s = soldierManager->CreateSoldier(_squads.Items[i]->Vehicles[j].Soldiers[k].Type, animationManager, weaponManager);
					s->SetTitle(_squads.Items[i]->Vehicles[j].Soldiers[k].Title);
					s->SetRank(_squads.Items[i]->Vehicles[j].Soldiers[k].Rank);
					s->SetCamouflage(_squads.Items[i]->Vehicles[j].Soldiers[k].Camo);
					// XXX/GWS: Need better determination of the squad leader
					if(strcasecmp(_squads.Items[i]->Vehicles[j].Soldiers[k].Title, "Leader") == 0
						|| strcasecmp(_squads.Items[i]->Vehicles[j].Soldiers[k].Title, "Gunner") == 0)
					{
						s->SetSquadLeader(true);
					}
					s->SetSquad(squad);
					s->SetInVechicle(true);
					v->AddCrew(s, _squads.Items[i]->Vehicles[j].Soldiers[k].Slot);
				}
				squad->_vehicles.Add(v);
				v->SetSquad(squad);
			}

			return squad;
		}
	}
	return NULL;
}

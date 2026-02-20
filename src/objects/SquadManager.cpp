#include "./SquadManager.h"
#include "misc/tinyxml2.h"
#include "misc/Error.h"
#include <objects/SoldierManager.h>
#include <objects/VehicleManager.h>
#include <objects/WeaponManager.h>
#include <objects/Squad.h>
#include <assert.h>
#include <strings.h>
#include <string>
#include <filesystem>

#define MAX_SOLDIERS_IN_SQUAD 32
#define MAX_VEHICLES_IN_SQUAD 1

struct SquadSoldierAttributes {
	SquadSoldierAttributes() { Slot=-1; }
	std::string Type;
	std::string Title;
	std::string Rank;
	std::string Camo;
	int Slot;
};

struct SquadVehicleAttributes {
	SquadVehicleAttributes() { NumSoldiers=0; }
	int NumSoldiers;
	SquadSoldierAttributes Soldiers[MAX_SOLDIERS_IN_SQUAD];
	std::string Type;
};

struct SquadTemplate {
	SquadTemplate() { NumSoldiers = NumVehicles = 0; }
	int  NumSoldiers;
	int  NumVehicles;
	SquadSoldierAttributes Soldiers[MAX_SOLDIERS_IN_SQUAD];
	SquadVehicleAttributes Vehicles[MAX_VEHICLES_IN_SQUAD];
	std::string Name;
	std::string IconName;
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
SquadManager::LoadSquads(const std::filesystem::path& fileName)
{

	XMLDocument doc;
	if (doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
		ERROR("Failed to load squads file: " + fileName.string());
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
			squad->Name = nameElem->GetText();
		}

		// Parse Icon
		XMLElement* iconElem = squadElem->FirstChildElement("Icon");
		if (iconElem && iconElem->GetText()) {
			squad->IconName = iconElem->GetText();
		}
		
		// Parse Soldiers directly under Squad
		for (XMLElement* soldierElem = squadElem->FirstChildElement("Soldier");
			 soldierElem != nullptr;
			 soldierElem = soldierElem->NextSiblingElement("Soldier"))
		{
			if (squad->NumSoldiers >= MAX_SOLDIERS_IN_SQUAD) break;
			
			XMLElement* titleElem = soldierElem->FirstChildElement("Title");
			if (titleElem && titleElem->GetText()) {
				squad->Soldiers[squad->NumSoldiers].Title = titleElem->GetText();
			}

			XMLElement* rankElem = soldierElem->FirstChildElement("Rank");
			if (rankElem && rankElem->GetText()) {
				squad->Soldiers[squad->NumSoldiers].Rank = rankElem->GetText();
			}

			XMLElement* typeElem = soldierElem->FirstChildElement("Type");
			if (typeElem && typeElem->GetText()) {
				squad->Soldiers[squad->NumSoldiers].Type = typeElem->GetText();
			}

			XMLElement* camoElem = soldierElem->FirstChildElement("Camo");
			if (camoElem && camoElem->GetText()) {
				squad->Soldiers[squad->NumSoldiers].Camo = camoElem->GetText();
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
				vehicle->Type = typeElem->GetText();
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
					vehicle->Soldiers[vehicle->NumSoldiers].Title = titleElem->GetText();
				}

				XMLElement* rankElem = soldierElem->FirstChildElement("Rank");
				if (rankElem && rankElem->GetText()) {
					vehicle->Soldiers[vehicle->NumSoldiers].Rank = rankElem->GetText();
				}

				XMLElement* soldierTypeElem = soldierElem->FirstChildElement("Type");
				if (soldierTypeElem && soldierTypeElem->GetText()) {
					vehicle->Soldiers[vehicle->NumSoldiers].Type = soldierTypeElem->GetText();
				}

				XMLElement* camoElem = soldierElem->FirstChildElement("Camo");
				if (camoElem && camoElem->GetText()) {
					vehicle->Soldiers[vehicle->NumSoldiers].Camo = camoElem->GetText();
				}
				
				vehicle->NumSoldiers++;
			}
			
			squad->NumVehicles++;
		}
		
		_squads.push_back(squad);
	}
}

// Creates an instance of a specific soldier
Squad *
SquadManager::CreateSquad(const std::string& squadType, SoldierManager *soldierManager, VehicleManager *vehicleManager, AnimationManager *animationManager, WeaponManager *weaponManager)
{
	for(auto* squadTemplate : _squads) {
		if(squadType == squadTemplate->Name) {
			Squad *squad = new Squad();
			squad->_iconName = squadTemplate->IconName;
			squad->_name = squadType;
			// XXX/GWS: Fix this random team quality thing here!
			squad->_quality = (Squad::Quality) (rand() % Squad::NumQuality);

			// Add all of the soldiers
			for(int j = 0; j < squadTemplate->NumSoldiers; ++j) {
				Soldier *s = soldierManager->CreateSoldier(squadTemplate->Soldiers[j].Type, animationManager, weaponManager);
				s->SetTitle(squadTemplate->Soldiers[j].Title);
				s->SetRank(squadTemplate->Soldiers[j].Rank);
				s->SetCamouflage(squadTemplate->Soldiers[j].Camo);
				s->SetFormationPosition(j);
				// XXX/GWS: Need better determination of the squad leader
				if(strcasecmp(squadTemplate->Soldiers[j].Title.c_str(), "Leader") == 0
					|| strcasecmp(squadTemplate->Soldiers[j].Title.c_str(), "Gunner") == 0)
				{
					s->SetSquadLeader(true);
				}
				squad->_soldiers.push_back(s);
				s->SetSquad(squad);
			}

			// Add all of the vehicles
			for(int j = 0; j < squadTemplate->NumVehicles; ++j)
			{
				Vehicle *v = vehicleManager->GetVehicle(squadTemplate->Vehicles[j].Type);
				v->SetSquadLeader(true); // XXX/GWS: Better determination here

				// Now add soldiers to this vehicle
				for(int k = 0; k < squadTemplate->Vehicles[j].NumSoldiers; ++k) {
					Soldier *s = soldierManager->CreateSoldier(squadTemplate->Vehicles[j].Soldiers[k].Type.c_str(), animationManager, weaponManager);
					s->SetTitle(squadTemplate->Vehicles[j].Soldiers[k].Title);
					s->SetRank(squadTemplate->Vehicles[j].Soldiers[k].Rank);
					s->SetCamouflage(squadTemplate->Vehicles[j].Soldiers[k].Camo);
					// XXX/GWS: Need better determination of the squad leader
					if(strcasecmp(squadTemplate->Vehicles[j].Soldiers[k].Title.c_str(), "Leader") == 0
						|| strcasecmp(squadTemplate->Vehicles[j].Soldiers[k].Title.c_str(), "Gunner") == 0)
					{
						s->SetSquadLeader(true);
					}
					s->SetSquad(squad);
					s->SetInVechicle(true);
					v->AddCrew(s, squadTemplate->Vehicles[j].Soldiers[k].Slot);
				}
				squad->_vehicles.push_back(v);
				v->SetSquad(squad);
			}

			return squad;
		}
	}
	return nullptr;
}

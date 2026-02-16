#include "./VehicleManager.h"
#include "misc/tinyxml2.h"

#include <string>
#include <filesystem>
#include <stdio.h>
#include <math.h>
#include <misc/TGA.h>
#include <misc/Structs.h>
#include <objects/Vehicle.h>
#include <application/Globals.h>

#define MAX_TURRET_WEAPONS  8
#define MAX_HULL_WEAPONS    8

struct WeaponAttributes {
    std::string Name;
    int Slot;
    int NumClips;
};

struct TurretAttributes {
    TurretAttributes() { Tga = NULL; NumWeapons = 0; RotationRate = 0; }
    std::string Graphic;
    TGA *Tga;
    WeaponAttributes Weapons[MAX_TURRET_WEAPONS];
    int NumWeapons;
    int RotationRate;
    Point Position;
    Point PrimaryMuzzlePosition;
};

struct HullAttributes {
    HullAttributes() { Tga = NULL; NumWeapons = 0; RotationRate = 0; }
    std::string Graphic;
    TGA *Tga;
    WeaponAttributes Weapons[MAX_HULL_WEAPONS];
    int NumWeapons;
    int RotationRate;
};

struct WreckAttributes {
    WreckAttributes() { Tga = NULL; }
    std::string Graphic;
    TGA *Tga;
};

struct VehicleAttributes {
    std::string Name;
    int Index;
    HullAttributes Hull;
    TurretAttributes Turret;
    WreckAttributes Wreck;
    float MaxRoadSpeed;
    float Acceleration;
};

using namespace tinyxml2;

VehicleManager::VehicleManager(void)
{
}

VehicleManager::~VehicleManager(void)
{
}

void
VehicleManager::Load(const std::filesystem::path& fileName)
{

    XMLDocument doc;
    if (doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
        printf("Failed to load vehicles file: %s\n", fileName.c_str());
        return;
    }
    
    XMLElement* root = doc.FirstChildElement("Vehicles");
    if (!root) return;
    
    for (XMLElement* vehicleElem = root->FirstChildElement("Vehicle"); 
         vehicleElem != nullptr; 
         vehicleElem = vehicleElem->NextSiblingElement("Vehicle")) 
    {
        VehicleAttributes* vehicle = new VehicleAttributes();
        vehicle->Index = -1;
        vehicle->Hull.NumWeapons = 0;
        vehicle->Turret.NumWeapons = 0;
        vehicle->MaxRoadSpeed = 0;
        vehicle->Acceleration = 0;
        
        // Parse Name
        XMLElement* nameElem = vehicleElem->FirstChildElement("Name");
        if (nameElem && nameElem->GetText()) {
            vehicle->Name = nameElem->GetText();
        }
        
        // Parse Index
        XMLElement* indexElem = vehicleElem->FirstChildElement("Index");
        if (indexElem && indexElem->GetText()) {
            vehicle->Index = atoi(indexElem->GetText());
        }
        
        // Parse MaxRoadSpeed
        XMLElement* speedElem = vehicleElem->FirstChildElement("MaxRoadSpeed");
        if (speedElem && speedElem->GetText()) {
            vehicle->MaxRoadSpeed = (float)atof(speedElem->GetText());
        }
        
        // Parse Acceleration
        XMLElement* accelElem = vehicleElem->FirstChildElement("Acceleration");
        if (accelElem && accelElem->GetText()) {
            vehicle->Acceleration = (float)atof(accelElem->GetText());
        }
        
        // Parse Hull
        XMLElement* hullElem = vehicleElem->FirstChildElement("Hull");
        if (hullElem) {
            XMLElement* graphicElem = hullElem->FirstChildElement("Graphic");
            if (graphicElem && graphicElem->GetText()) {
                vehicle->Hull.Graphic = graphicElem->GetText();
            }
            
            XMLElement* rotElem = hullElem->FirstChildElement("RotationRate");
            if (rotElem && rotElem->GetText()) {
                vehicle->Hull.RotationRate = atoi(rotElem->GetText());
            }
            
            // Parse Hull Weapons
            for (XMLElement* weaponElem = hullElem->FirstChildElement("Weapon");
                 weaponElem != nullptr;
                 weaponElem = weaponElem->NextSiblingElement("Weapon"))
            {
                if (vehicle->Hull.NumWeapons >= MAX_HULL_WEAPONS) break;
                
                const char* slotAttr = weaponElem->Attribute("slot");
                if (slotAttr) {
                    vehicle->Hull.Weapons[vehicle->Hull.NumWeapons].Slot = atoi(slotAttr);
                }
                
                const char* clipsAttr = weaponElem->Attribute("clips");
                if (clipsAttr) {
                    vehicle->Hull.Weapons[vehicle->Hull.NumWeapons].NumClips = atoi(clipsAttr);
                }
                
                if (weaponElem->GetText()) {
                    vehicle->Hull.Weapons[vehicle->Hull.NumWeapons].Name = weaponElem->GetText();
                }
                
                vehicle->Hull.NumWeapons++;
            }
        }
        
        // Parse Turret
        XMLElement* turretElem = vehicleElem->FirstChildElement("Turret");
        if (turretElem) {
            XMLElement* graphicElem = turretElem->FirstChildElement("Graphic");
            if (graphicElem && graphicElem->GetText()) {
                vehicle->Turret.Graphic = graphicElem->GetText();
            }
            
            XMLElement* rotElem = turretElem->FirstChildElement("RotationRate");
            if (rotElem && rotElem->GetText()) {
                vehicle->Turret.RotationRate = atoi(rotElem->GetText());
            }
            
            XMLElement* posXElem = turretElem->FirstChildElement("PositionX");
            if (posXElem && posXElem->GetText()) {
                vehicle->Turret.Position.x = atoi(posXElem->GetText());
            }
            
            XMLElement* posYElem = turretElem->FirstChildElement("PositionY");
            if (posYElem && posYElem->GetText()) {
                vehicle->Turret.Position.y = atoi(posYElem->GetText());
            }
            
            XMLElement* muzzleXElem = turretElem->FirstChildElement("PrimaryMuzzleX");
            if (muzzleXElem && muzzleXElem->GetText()) {
                vehicle->Turret.PrimaryMuzzlePosition.x = atoi(muzzleXElem->GetText());
            }
            
            XMLElement* muzzleYElem = turretElem->FirstChildElement("PrimaryMuzzleY");
            if (muzzleYElem && muzzleYElem->GetText()) {
                vehicle->Turret.PrimaryMuzzlePosition.y = atoi(muzzleYElem->GetText());
            }
            
            // Parse Turret Weapons
            for (XMLElement* weaponElem = turretElem->FirstChildElement("Weapon");
                 weaponElem != nullptr;
                 weaponElem = weaponElem->NextSiblingElement("Weapon"))
            {
                if (vehicle->Turret.NumWeapons >= MAX_TURRET_WEAPONS) break;
                
                const char* slotAttr = weaponElem->Attribute("slot");
                if (slotAttr) {
                    vehicle->Turret.Weapons[vehicle->Turret.NumWeapons].Slot = atoi(slotAttr);
                }
                
                const char* clipsAttr = weaponElem->Attribute("clips");
                if (clipsAttr) {
                    vehicle->Turret.Weapons[vehicle->Turret.NumWeapons].NumClips = atoi(clipsAttr);
                }
                
                if (weaponElem->GetText()) {
                    vehicle->Turret.Weapons[vehicle->Turret.NumWeapons].Name = weaponElem->GetText();
                }
                
                vehicle->Turret.NumWeapons++;
            }
        }
        
        // Parse Wreck
        XMLElement* wreckElem = vehicleElem->FirstChildElement("Wreck");
        if (wreckElem) {
            XMLElement* graphicElem = wreckElem->FirstChildElement("Graphic");
            if (graphicElem && graphicElem->GetText()) {
                vehicle->Wreck.Graphic = graphicElem->GetText();
            }
        }
        
        _vehicles.Add(vehicle);
    }
}

Vehicle *
VehicleManager::GetVehicle(const std::string& vehicleName)
{
	for(int i = 0; i < _vehicles.Count; ++i) {
		if(vehicleName == _vehicles.Items[i]->Name) {
			Vehicle *v = new Vehicle();
		    // Okay, now iterate through all of the widget attributes and create
		    // our widgets
			// XXX/GWS: The hardcoded directory here is bad

			v->_name = _vehicles.Items[i]->Name;
			v->_turretPosition.x = _vehicles.Items[i]->Turret.Position.x;
			v->_turretPosition.y = _vehicles.Items[i]->Turret.Position.y;
			v->_turretRotationRate = _vehicles.Items[i]->Turret.RotationRate;
			v->_hullRotationRate = _vehicles.Items[i]->Hull.RotationRate;
			v->_muzzlePosition.x = _vehicles.Items[i]->Turret.PrimaryMuzzlePosition.x;
			v->_muzzlePosition.y = _vehicles.Items[i]->Turret.PrimaryMuzzlePosition.y;
			v->_maxRoadSpeed = _vehicles.Items[i]->MaxRoadSpeed;
			v->_acceleration = _vehicles.Items[i]->Acceleration;

			// The hull graphics
			if(_vehicles.Items[i]->Hull.Tga == NULL) {
				std::filesystem::path fName = g_Globals->Application.GraphicsDirectory / _vehicles.Items[i]->Hull.Graphic;
				_vehicles.Items[i]->Hull.Tga = TGA::Create(fName);
			}
			v->_hullGraphics = _vehicles.Items[i]->Hull.Tga;

			// The turret graphic
			if(_vehicles.Items[i]->Turret.Tga == NULL) {
				std::filesystem::path fName = g_Globals->Application.GraphicsDirectory / _vehicles.Items[i]->Turret.Graphic;
				_vehicles.Items[i]->Turret.Tga = TGA::Create(fName);
			}
			v->_turretGraphics = _vehicles.Items[i]->Turret.Tga;

			// The wreck graphic
			if(_vehicles.Items[i]->Wreck.Tga == NULL) {
				std::filesystem::path fName = g_Globals->Application.GraphicsDirectory / _vehicles.Items[i]->Wreck.Graphic;
				_vehicles.Items[i]->Wreck.Tga = TGA::Create(fName);
			}
			v->_wreckGraphics = _vehicles.Items[i]->Wreck.Tga;

			// Now do all of the weapons
			for(int j = 0; j < _vehicles.Items[i]->Hull.NumWeapons; ++j)
			{
				v->AddWeapon(g_Globals->World.Weapons->GetWeapon(_vehicles.Items[i]->Hull.Weapons[j].Name),
					_vehicles.Items[i]->Hull.Weapons[j].Slot, _vehicles.Items[i]->Hull.Weapons[j].NumClips, true);
			}
			for(int j = 0; j < _vehicles.Items[i]->Turret.NumWeapons; ++j)
			{
				v->AddWeapon(g_Globals->World.Weapons->GetWeapon(_vehicles.Items[i]->Turret.Weapons[j].Name),
					_vehicles.Items[i]->Turret.Weapons[j].Slot, _vehicles.Items[i]->Turret.Weapons[j].NumClips, false);
			}

			// The last weapon is always an empty weapon!!!
			v->AddWeapon(g_Globals->World.Weapons->GetWeapon("Blank"), -1, 0, false);
			return v;
		}
	}
	return NULL;
}

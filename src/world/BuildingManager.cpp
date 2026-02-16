#include "./BuildingManager.h"
#include <misc/tinyxml2.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <filesystem>
#include <misc/Structs.h>
#include <misc/TGA.h>
#include <world/Building.h>
#include <application/Globals.h>

void
BuildingManager::LoadBuildings(const char *fileName, Array<Building> *buildings)
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(fileName) != tinyxml2::XML_SUCCESS) {
		printf("Failed to load buildings file: %s\n", fileName);
		return;
	}
	
	tinyxml2::XMLElement* root = doc.FirstChildElement("Buildings");
	if (!root) return;
	
	for (tinyxml2::XMLElement* buildingElem = root->FirstChildElement("Building");
		 buildingElem != nullptr;
		 buildingElem = buildingElem->NextSiblingElement("Building"))
	{
		Building* building = new Building();
		
		// Parse Position
		tinyxml2::XMLElement* posElem = buildingElem->FirstChildElement("Position");
		if (posElem) {
			tinyxml2::XMLElement* xElem = posElem->FirstChildElement("X");
			if (xElem && xElem->GetText()) {
				building->Position.x = atoi(xElem->GetText());
			}
			
			tinyxml2::XMLElement* yElem = posElem->FirstChildElement("Y");
			if (yElem && yElem->GetText()) {
				building->Position.y = atoi(yElem->GetText());
			}
		}
		
		// Parse Boundary (array of Points)
		tinyxml2::XMLElement* boundaryElem = buildingElem->FirstChildElement("Boundary");
		if (boundaryElem) {
			for (tinyxml2::XMLElement* pointElem = boundaryElem->FirstChildElement("Point");
				 pointElem != nullptr;
				 pointElem = pointElem->NextSiblingElement("Point"))
			{
				Point* p = new Point();
				p->x = 0;
				p->y = 0;
				
				tinyxml2::XMLElement* xElem = pointElem->FirstChildElement("X");
				if (xElem && xElem->GetText()) {
					p->x = atoi(xElem->GetText());
				}
				
				tinyxml2::XMLElement* yElem = pointElem->FirstChildElement("Y");
				if (yElem && yElem->GetText()) {
					p->y = atoi(yElem->GetText());
				}
				
				building->AddBoundaryPoint(p);
			}
		}
		
		// Parse Exterior Graphic
		tinyxml2::XMLElement* extElem = buildingElem->FirstChildElement("ExteriorGraphic");
		if (extElem && extElem->GetText()) {
			std::filesystem::path path = g_Globals->Application.MapsDirectory / extElem->GetText();
			building->SetExterior(TGA::Create(path.c_str()));
		}

		// Parse Interior Graphic
		tinyxml2::XMLElement* intElem = buildingElem->FirstChildElement("InteriorGraphic");
		if (intElem && intElem->GetText()) {
			std::filesystem::path path = g_Globals->Application.MapsDirectory / intElem->GetText();
			building->SetInterior(TGA::Create(path.c_str()));
		}
		
		buildings->Add(building);
	}
}

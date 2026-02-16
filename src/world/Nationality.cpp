#include "./Nationality.h"
#include "misc/tinyxml2.h"
#include "misc/TGA.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <filesystem>
#include <application/Globals.h>

using namespace tinyxml2;

Nationality::Nationality(void)
{
	VictoryLocation = nullptr;
	MiniMap = nullptr;
}

Nationality::~Nationality(void)
{
	if (VictoryLocation) {
		delete VictoryLocation;
		VictoryLocation = nullptr;
	}
	if (MiniMap) {
		delete MiniMap;
		MiniMap = nullptr;
	}
}

void
Nationality::Load(char *fileName, Array<Nationality> *nationalities)
{
	XMLDocument doc;
	if (doc.LoadFile(fileName) != XML_SUCCESS) {
		printf("Failed to load nationalities file: %s\n", fileName);
		return;
	}
	
	XMLElement* root = doc.FirstChildElement("Nationalities");
	if (!root) {
		printf("No Nationalities element found in %s\n", fileName);
		return;
	}
	
	for (XMLElement* natElem = root->FirstChildElement("Nationality");
		 natElem != nullptr;
		 natElem = natElem->NextSiblingElement("Nationality"))
	{
		Nationality* nationality = new Nationality();
		
		XMLElement* nameElem = natElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			nationality->Name = nameElem->GetText();
		}
		
		// Load VictoryLocation (flag) image
		XMLElement* flagElem = natElem->FirstChildElement("VictoryLocation");
		if (flagElem && flagElem->GetText()) {
			std::filesystem::path fullPath = g_Globals->Application.GraphicsDirectory / flagElem->GetText();
			nationality->VictoryLocation = TGA::Create(fullPath.c_str());
			if (!nationality->VictoryLocation) {
				printf("Failed to load victory location image: %s\n", fullPath.c_str());
			}
		}

		// Load MiniMap icon image
		XMLElement* miniElem = natElem->FirstChildElement("MiniMap");
		if (miniElem && miniElem->GetText()) {
			std::filesystem::path fullPath = g_Globals->Application.GraphicsDirectory / miniElem->GetText();
			nationality->MiniMap = TGA::Create(fullPath.c_str());
			if (!nationality->MiniMap) {
				printf("Failed to load minimap image: %s\n", fullPath.c_str());
			}
		}
		
		nationalities->Add(nationality);
	}
}

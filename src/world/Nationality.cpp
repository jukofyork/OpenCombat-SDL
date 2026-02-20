#include "./Nationality.h"
#include "misc/tinyxml2.h"
#include "misc/TGA.h"
#include "misc/Error.h"
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
Nationality::Load(const std::filesystem::path& fileName, std::vector<Nationality*> *nationalities)
{
	XMLDocument doc;
	if (doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
		ERROR("Failed to load nationalities file: " + fileName.string());
	}

	XMLElement* root = doc.FirstChildElement("Nationalities");
	if (!root) {
		ERROR("No Nationalities element found in " + fileName.string());
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
			nationality->VictoryLocation = TGA::Create(fullPath);
			if (!nationality->VictoryLocation) {
				ERROR("Failed to load victory location image: " + fullPath.string());
			}
		}

		// Load MiniMap icon image
		XMLElement* miniElem = natElem->FirstChildElement("MiniMap");
		if (miniElem && miniElem->GetText()) {
			std::filesystem::path fullPath = g_Globals->Application.GraphicsDirectory / miniElem->GetText();
			nationality->MiniMap = TGA::Create(fullPath);
			if (!nationality->MiniMap) {
				ERROR("Failed to load minimap image: " + fullPath.string());
			}
		}

		nationalities->push_back(nationality);
	}
}

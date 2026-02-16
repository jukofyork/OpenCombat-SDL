#include "ColorModifierManager.h"
#include <misc/tinyxml2.h>
#include <stdio.h>

// Instantiate the global color modifier
ColorModifiers g_ColorModifiers[MAX_COLOR_MODIFIERS];
int g_NumColorModifiers = 0;

ColorModifierManager::ColorModifierManager()
{
}

ColorModifierManager::~ColorModifierManager()
{
	// XXX/GWS: Todo
}

void
ColorModifierManager::Load(char *configFile)
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(configFile) != tinyxml2::XML_SUCCESS) {
		printf("Failed to load color modifier file: %s\n", configFile);
		return;
	}
	
	g_NumColorModifiers = 0;
	
	tinyxml2::XMLElement* root = doc.FirstChildElement("ColorModifiers");
	if (!root) return;
	
	for (tinyxml2::XMLElement* modElem = root->FirstChildElement("ColorModifier"); 
		 modElem != nullptr && g_NumColorModifiers < MAX_COLOR_MODIFIERS; 
		 modElem = modElem->NextSiblingElement("ColorModifier")) 
	{
		// Parse Name
		tinyxml2::XMLElement* nameElem = modElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			g_ColorModifiers[g_NumColorModifiers].Name = nameElem->GetText();
		}
		
		// Parse Body RGB
		tinyxml2::XMLElement* bodyElem = modElem->FirstChildElement("Body");
		if (bodyElem) {
			tinyxml2::XMLElement* redElem = bodyElem->FirstChildElement("Red");
			if (redElem && redElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Body.Red = 8 * atoi(redElem->GetText());
			}
			tinyxml2::XMLElement* greenElem = bodyElem->FirstChildElement("Green");
			if (greenElem && greenElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Body.Green = 8 * atoi(greenElem->GetText());
			}
			tinyxml2::XMLElement* blueElem = bodyElem->FirstChildElement("Blue");
			if (blueElem && blueElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Body.Blue = 8 * atoi(blueElem->GetText());
			}
		}
		
		// Parse Legs RGB
		tinyxml2::XMLElement* legsElem = modElem->FirstChildElement("Legs");
		if (legsElem) {
			tinyxml2::XMLElement* redElem = legsElem->FirstChildElement("Red");
			if (redElem && redElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Legs.Red = 8 * atoi(redElem->GetText());
			}
			tinyxml2::XMLElement* greenElem = legsElem->FirstChildElement("Green");
			if (greenElem && greenElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Legs.Green = 8 * atoi(greenElem->GetText());
			}
			tinyxml2::XMLElement* blueElem = legsElem->FirstChildElement("Blue");
			if (blueElem && blueElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Legs.Blue = 8 * atoi(blueElem->GetText());
			}
		}
		
		// Parse Head RGB
		tinyxml2::XMLElement* headElem = modElem->FirstChildElement("Head");
		if (headElem) {
			tinyxml2::XMLElement* redElem = headElem->FirstChildElement("Red");
			if (redElem && redElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Head.Red = 8 * atoi(redElem->GetText());
			}
			tinyxml2::XMLElement* greenElem = headElem->FirstChildElement("Green");
			if (greenElem && greenElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Head.Green = 8 * atoi(greenElem->GetText());
			}
			tinyxml2::XMLElement* blueElem = headElem->FirstChildElement("Blue");
			if (blueElem && blueElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Head.Blue = 8 * atoi(blueElem->GetText());
			}
		}
		
		// Parse Belt RGB
		tinyxml2::XMLElement* beltElem = modElem->FirstChildElement("Belt");
		if (beltElem) {
			tinyxml2::XMLElement* redElem = beltElem->FirstChildElement("Red");
			if (redElem && redElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Belt.Red = 8 * atoi(redElem->GetText());
			}
			tinyxml2::XMLElement* greenElem = beltElem->FirstChildElement("Green");
			if (greenElem && greenElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Belt.Green = 8 * atoi(greenElem->GetText());
			}
			tinyxml2::XMLElement* blueElem = beltElem->FirstChildElement("Blue");
			if (blueElem && blueElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Belt.Blue = 8 * atoi(blueElem->GetText());
			}
		}
		
		// Parse Boots RGB
		tinyxml2::XMLElement* bootsElem = modElem->FirstChildElement("Boots");
		if (bootsElem) {
			tinyxml2::XMLElement* redElem = bootsElem->FirstChildElement("Red");
			if (redElem && redElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Boots.Red = 8 * atoi(redElem->GetText());
			}
			tinyxml2::XMLElement* greenElem = bootsElem->FirstChildElement("Green");
			if (greenElem && greenElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Boots.Green = 8 * atoi(greenElem->GetText());
			}
			tinyxml2::XMLElement* blueElem = bootsElem->FirstChildElement("Blue");
			if (blueElem && blueElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Boots.Blue = 8 * atoi(blueElem->GetText());
			}
		}
		
		// Parse Weapon RGB
		tinyxml2::XMLElement* weaponElem = modElem->FirstChildElement("Weapon");
		if (weaponElem) {
			tinyxml2::XMLElement* redElem = weaponElem->FirstChildElement("Red");
			if (redElem && redElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Weapon.Red = 8 * atoi(redElem->GetText());
			}
			tinyxml2::XMLElement* greenElem = weaponElem->FirstChildElement("Green");
			if (greenElem && greenElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Weapon.Green = 8 * atoi(greenElem->GetText());
			}
			tinyxml2::XMLElement* blueElem = weaponElem->FirstChildElement("Blue");
			if (blueElem && blueElem->GetText()) {
				g_ColorModifiers[g_NumColorModifiers].Weapon.Blue = 8 * atoi(blueElem->GetText());
			}
		}
		
		g_NumColorModifiers++;
	}
}

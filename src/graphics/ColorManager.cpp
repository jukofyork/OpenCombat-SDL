#include "ColorManager.h"
#include "misc/tinyxml2.h"
#include "misc/Error.h"
#include <string>
#include <filesystem>

using namespace tinyxml2;

ColorManager::ColorManager()
{
}

ColorManager::~ColorManager()
{
}

void
ColorManager::CopyColor(const std::string& name, Color *dest)
{
	for(const auto& pair : _colors) {
		if(pair.first == name) {
			dest->alpha = pair.second.alpha;
			dest->red = pair.second.red;
			dest->green = pair.second.green;
			dest->blue = pair.second.blue;
			return;
		}
	}
}

void
ColorManager::Load(const std::filesystem::path& configFile)
{
	XMLDocument doc;
	if (doc.LoadFile(configFile.c_str()) != XML_SUCCESS) {
		ERROR("Failed to load color file: " + configFile.string());
	}
	
	XMLElement* root = doc.FirstChildElement("Colors");
	if (!root) return;
	
	for (XMLElement* colorElem = root->FirstChildElement("Color"); 
		 colorElem != nullptr; 
		 colorElem = colorElem->NextSiblingElement("Color")) 
	{
		Color color;
		color.alpha = color.red = color.green = color.blue = 0;
		std::string name;
		
		XMLElement* nameElem = colorElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			name = nameElem->GetText();
		}
		
		XMLElement* alphaElem = colorElem->FirstChildElement("Alpha");
		if (alphaElem && alphaElem->GetText()) {
			color.alpha = static_cast<unsigned char>(atoi(alphaElem->GetText()));
		}
		
		XMLElement* redElem = colorElem->FirstChildElement("Red");
		if (redElem && redElem->GetText()) {
			color.red = static_cast<unsigned char>(atoi(redElem->GetText()));
		}
		
		XMLElement* greenElem = colorElem->FirstChildElement("Green");
		if (greenElem && greenElem->GetText()) {
			color.green = static_cast<unsigned char>(atoi(greenElem->GetText()));
		}
		
		XMLElement* blueElem = colorElem->FirstChildElement("Blue");
		if (blueElem && blueElem->GetText()) {
			color.blue = static_cast<unsigned char>(atoi(blueElem->GetText()));
		}
		
		_colors.emplace_back(name, color);
	}
}

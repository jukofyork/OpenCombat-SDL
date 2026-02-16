#include "ColorManager.h"
#include "misc/tinyxml2.h"
#include <vector>
#include <stdio.h>
#include <string>
#include <filesystem>

using namespace tinyxml2;

ColorManager::ColorManager()
{
	_nColors = 0;
}

ColorManager::~ColorManager()
{
	// XXX/GWS: Todo
}

void
ColorManager::CopyColor(const std::string& name, Color *dest)
{
	for(int i = 0; i < _nColors; ++i) {
		if(_names[i] == name) {
			dest->alpha = _colors[i].alpha;
			dest->red = _colors[i].red;
			dest->green = _colors[i].green;
			dest->blue = _colors[i].blue;
			return;
		}
	}
}

void
ColorManager::Load(const std::filesystem::path& configFile)
{
	struct ColorAttributes {
		std::string Name;
		int a, r, g, b;
	};
	
	XMLDocument doc;
	if (doc.LoadFile(configFile.c_str()) != XML_SUCCESS) {
		printf("Failed to load color file: %s\n", configFile.c_str());
		return;
	}
	
	std::vector<ColorAttributes> dest;
	
	XMLElement* root = doc.FirstChildElement("Colors");
	if (!root) return;
	
	for (XMLElement* colorElem = root->FirstChildElement("Color"); 
		 colorElem != nullptr; 
		 colorElem = colorElem->NextSiblingElement("Color")) 
	{
		ColorAttributes attr;
		attr.a = attr.r = attr.g = attr.b = 0;
		
		XMLElement* nameElem = colorElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			attr.Name = nameElem->GetText();
		}
		
		XMLElement* alphaElem = colorElem->FirstChildElement("Alpha");
		if (alphaElem && alphaElem->GetText()) {
			attr.a = atoi(alphaElem->GetText());
		}
		
		XMLElement* redElem = colorElem->FirstChildElement("Red");
		if (redElem && redElem->GetText()) {
			attr.r = atoi(redElem->GetText());
		}
		
		XMLElement* greenElem = colorElem->FirstChildElement("Green");
		if (greenElem && greenElem->GetText()) {
			attr.g = atoi(greenElem->GetText());
		}
		
		XMLElement* blueElem = colorElem->FirstChildElement("Blue");
		if (blueElem && blueElem->GetText()) {
			attr.b = atoi(blueElem->GetText());
		}
		
		dest.push_back(attr);
	}
	
	for(size_t i = 0; i < dest.size(); ++i) {
		const ColorAttributes &attr = dest[i];
		_colors[_nColors].alpha = (unsigned char)attr.a;
		_colors[_nColors].red = (unsigned char)attr.r;
		_colors[_nColors].green = (unsigned char)attr.g;
		_colors[_nColors].blue = (unsigned char)attr.b;
		_names[_nColors++] = attr.Name;
	}
}

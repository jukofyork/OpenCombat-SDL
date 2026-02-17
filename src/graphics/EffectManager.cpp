#include "./EffectManager.h"
#include "misc/tinyxml2.h"

#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <dirent.h>
#include <assert.h>
#include <misc/TGA.h>
#include <graphics/Effect.h>
#include <application/Globals.h>

using namespace tinyxml2;

struct EffectAttributes
{
	EffectAttributes() { Dynamic=false;PlaceOnTurret=false; }
	std::string Name;
	std::vector<std::string> GraphicsFile;
	std::string Sound;
	long FrameHold;
	bool Dynamic;
	bool PlaceOnTurret;
};

EffectManager::EffectManager(void)
{
}

EffectManager::~EffectManager(void)
{
}

void
EffectManager::LoadEffects(const std::filesystem::path& fileName)
{
	XMLDocument doc;
	if (doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
		printf("Failed to load effects file: %s\n", fileName.c_str());
		return;
	}

	std::vector<EffectAttributes> dest;

	XMLElement* root = doc.FirstChildElement("Effects");
	if (!root) return;

	for (XMLElement* effectElem = root->FirstChildElement("Effect");
		 effectElem != nullptr;
		 effectElem = effectElem->NextSiblingElement("Effect"))
	{
		EffectAttributes attr;

		// Get attributes from <Effect> element
		const char* typeAttr = effectElem->Attribute("type");
		if (typeAttr && strcmp(typeAttr, "dynamic") == 0) {
			attr.Dynamic = true;
		}

		const char* placeAttr = effectElem->Attribute("place");
		if (placeAttr && std::string(placeAttr) == "turret") {
			attr.PlaceOnTurret = true;
		}

		XMLElement* nameElem = effectElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			attr.Name = nameElem->GetText();
		}

		XMLElement* soundElem = effectElem->FirstChildElement("Sound");
		if (soundElem && soundElem->GetText()) {
			attr.Sound = soundElem->GetText();
		}

		// Parse <Graphic> elements
		for (XMLElement* graphicElem = effectElem->FirstChildElement("Graphic");
			 graphicElem != nullptr;
			 graphicElem = graphicElem->NextSiblingElement("Graphic"))
		{
		if (graphicElem->GetText()) {
			attr.GraphicsFile.push_back((g_Globals->Application.GraphicsDirectory / "Effects" / graphicElem->GetText()).string());
		}
		}

		// Parse <Graphics> element with file pattern
		XMLElement* graphicsElem = effectElem->FirstChildElement("Graphics");
		if (graphicsElem && graphicsElem->GetText()) {
			GetFiles(&attr, graphicsElem->GetText());
		}

		XMLElement* frameElem = effectElem->FirstChildElement("FrameHold");
		if (frameElem && frameElem->GetText()) {
			attr.FrameHold = atoi(frameElem->GetText());
		}

		dest.push_back(attr);
	}

	for(size_t i = 0; i < dest.size(); ++i) {
		// Create the source TGA file
		Effect *e = new Effect(dest[i].Name);
		e->SetSound(dest[i].Sound);
		e->SetDynamic(dest[i].Dynamic);
		e->SetPlaceOnTurret(dest[i].PlaceOnTurret);

		for(size_t j = 0; j < dest[i].GraphicsFile.size(); ++j) {
			TGA *tga = TGA::Create(dest[i].GraphicsFile[j]);
			if (!tga) {
				printf("Failed to load effect frame: %s\n", dest[i].GraphicsFile[j].c_str());
				continue;
			}
			tga->SetTransparentColor(0,0,0);
			_sourceImages.push_back(tga);
			e->AddFrame(tga, dest[i].FrameHold);
	    }
		_effects.push_back(e);
   }
}

void EffectManager::GetFiles(EffectAttributes *attr, const std::string& searchStr)
{
	std::string searchDir = (g_Globals->Application.GraphicsDirectory / "Effects" / searchStr).string();

	// Find the wildcard position
	size_t wildcardPos = searchDir.find('*');
	if (wildcardPos == std::string::npos) return;

	std::string baseDir = searchDir.substr(0, wildcardPos);

	DIR* dir = opendir(baseDir.c_str());
	if (!dir) {
		assert(0);
		return;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		// Skip . and ..
		if (entry->d_name[0] == '.') continue;

		// Simple pattern matching - check if filename has extension
		std::string fileName = entry->d_name;
		if (fileName.find('.') != std::string::npos) {
			attr->GraphicsFile.push_back(baseDir + fileName);
		}
	}
	closedir(dir);
	
	// Sort the graphics files to ensure correct order
	if(!attr->GraphicsFile.empty()) {
		std::sort(attr->GraphicsFile.begin(), attr->GraphicsFile.end());
	}
}

Effect *
EffectManager::GetEffect(const std::string &effectName)
{
	for(auto* effect : _effects) {
		if(effectName == effect->GetName()) {
			return effect->Clone();
		}
	}
	return nullptr;
}

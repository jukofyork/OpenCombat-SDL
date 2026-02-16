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

	Array<EffectAttributes> dest;

	XMLElement* root = doc.FirstChildElement("Effects");
	if (!root) return;

	for (XMLElement* effectElem = root->FirstChildElement("Effect");
		 effectElem != nullptr;
		 effectElem = effectElem->NextSiblingElement("Effect"))
	{
		EffectAttributes* attr = new EffectAttributes();
		attr->Sound[0] = '\0';

		// Get attributes from <Effect> element
		const char* typeAttr = effectElem->Attribute("type");
		if (typeAttr && strcmp(typeAttr, "dynamic") == 0) {
			attr->Dynamic = true;
		}

		const char* placeAttr = effectElem->Attribute("place");
		if (placeAttr && std::string(placeAttr) == "turret") {
			attr->PlaceOnTurret = true;
		}

		XMLElement* nameElem = effectElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			attr->Name = nameElem->GetText();
		}

		XMLElement* soundElem = effectElem->FirstChildElement("Sound");
		if (soundElem && soundElem->GetText()) {
			attr->Sound = soundElem->GetText();
		}

		// Parse <Graphic> elements
		for (XMLElement* graphicElem = effectElem->FirstChildElement("Graphic");
			 graphicElem != nullptr;
			 graphicElem = graphicElem->NextSiblingElement("Graphic"))
		{
		if (graphicElem->GetText()) {
			attr->GraphicsFile.push_back((g_Globals->Application.GraphicsDirectory / "Effects" / graphicElem->GetText()).string());
		}
		}

		// Parse <Graphics> element with file pattern
		XMLElement* graphicsElem = effectElem->FirstChildElement("Graphics");
		if (graphicsElem && graphicsElem->GetText()) {
			GetFiles(attr, graphicsElem->GetText());
		}

		XMLElement* frameElem = effectElem->FirstChildElement("FrameHold");
		if (frameElem && frameElem->GetText()) {
			attr->FrameHold = atoi(frameElem->GetText());
		}

		dest.Add(attr);
	}

	for(int i = 0; i < dest.Count; ++i) {
		// Create the source TGA file
		Effect *e = new Effect(dest.Items[i]->Name);
		e->SetSound(dest.Items[i]->Sound);
		e->SetDynamic(dest.Items[i]->Dynamic);
		e->SetPlaceOnTurret(dest.Items[i]->PlaceOnTurret);

		for(size_t j = 0; j < dest.Items[i]->GraphicsFile.size(); ++j) {
			TGA *tga = TGA::Create(dest.Items[i]->GraphicsFile[j]);
			tga->SetTransparentColor(0,0,0);
			_sourceImages.Add(tga);

			// Let's find the hotspot for this effect. It is encoded in the
			// filename (format: name.x.y.tga)
			std::string fName = dest.Items[i]->GraphicsFile[j];
			size_t lastDot = fName.rfind('.');
			if (lastDot != std::string::npos) {
				std::string yStr = fName.substr(lastDot + 1);
				fName = fName.substr(0, lastDot);
				size_t secondDot = fName.rfind('.');
				if (secondDot != std::string::npos) {
					std::string xStr = fName.substr(secondDot + 1);
					int x = atoi(xStr.c_str());
					int y = atoi(yStr.c_str());
					tga->SetOrigin(x,y);
				}
			}
			e->AddFrame(tga, dest.Items[i]->FrameHold);
	    }
		_effects.Add(e);
   }
}

void EffectManager::GetFiles(EffectAttributes *attr, const char *searchStr)
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
	for(int i = 0; i < _effects.Count; ++i) {
		if(effectName == _effects.Items[i]->GetName()) {
			Effect *e = _effects.Items[i]->Clone();
			return e;
		}
	}
	return NULL;
}

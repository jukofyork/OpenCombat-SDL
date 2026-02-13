#include "./EffectManager.h"
#include "misc/tinyxml2.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>
#include <misc/TGA.h>
#include <graphics/Effect.h>
#include <application/Globals.h>

using namespace tinyxml2;

// Comparison function for sorting filenames
static int compareEffectFilenames(const void *a, const void *b) {
	const char *fa = (const char *)a;
	const char *fb = (const char *)b;
	return strcmp(fa, fb);
}

struct EffectAttributes
{
	EffectAttributes() { Dynamic=false;PlaceOnTurret=false;NumGraphicsFile=0; }
	char Name[MAX_NAME];
	char GraphicsFile[256][MAX_NAME];
	char Sound[MAX_NAME];
	int NumGraphicsFile;
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
EffectManager::LoadEffects(char *fileName)
{
	XMLDocument doc;
	if (doc.LoadFile(fileName) != XML_SUCCESS) {
		printf("Failed to load effects file: %s\n", fileName);
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
		if (placeAttr && strcmp(placeAttr, "turret") == 0) {
			attr->PlaceOnTurret = true;
		}

		XMLElement* nameElem = effectElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			strcpy(attr->Name, nameElem->GetText());
		}

		XMLElement* soundElem = effectElem->FirstChildElement("Sound");
		if (soundElem && soundElem->GetText()) {
			strcpy(attr->Sound, soundElem->GetText());
		}

		// Parse <Graphic> elements
		for (XMLElement* graphicElem = effectElem->FirstChildElement("Graphic");
			 graphicElem != nullptr;
			 graphicElem = graphicElem->NextSiblingElement("Graphic"))
		{
			if (graphicElem->GetText() && attr->NumGraphicsFile < 256) {
				sprintf(attr->GraphicsFile[attr->NumGraphicsFile++], "%s/Effects/%s",
						g_Globals->Application.GraphicsDirectory, graphicElem->GetText());
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

	char fName[256];

	for(int i = 0; i < dest.Count; ++i) {
		// Create the source TGA file
		Effect *e = new Effect(dest.Items[i]->Name);
		e->SetSound(dest.Items[i]->Sound);
		e->SetDynamic(dest.Items[i]->Dynamic);
		e->SetPlaceOnTurret(dest.Items[i]->PlaceOnTurret);

		for(int j = 0; j < dest.Items[i]->NumGraphicsFile; ++j) {
			sprintf(fName, "%s", dest.Items[i]->GraphicsFile[j]);
			TGA *tga = TGA::Create(fName);
			tga->SetTransparentColor(0,0,0);
			_sourceImages.Add(tga);

			// Let's find the hotspot for this effect. It is encoded in the
			// filename.
			char *last = strrchr(fName, '.');
			*last = '\0';
			char *second = strrchr(fName, '.');
			*second = '\0';
			char *third = strrchr(fName, '.');
			int x = atoi(third+1);
			int y = atoi(second+1);
			tga->SetOrigin(x,y);
			e->AddFrame(tga, dest.Items[i]->FrameHold);
	    }
		_effects.Add(e);
   }
}

void EffectManager::GetFiles(EffectAttributes *attr, const char *searchStr)
{
	char searchDir[512];
	sprintf(searchDir, "%s/Effects/%s", g_Globals->Application.GraphicsDirectory, searchStr);

	// Find the wildcard position
	char *wildcard = strchr(searchDir, '*');
	if (!wildcard) return;

	*wildcard = '\0';
	char *pattern = wildcard + 1;

	DIR* dir = opendir(searchDir);
	if (!dir) {
		assert(0);
		return;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		// Skip . and ..
		if (entry->d_name[0] == '.') continue;

		// Simple pattern matching - check if filename ends with pattern
		char* dot = strrchr(entry->d_name, '.');
		if (dot) {
			if (attr->NumGraphicsFile < 256) {
				sprintf(attr->GraphicsFile[attr->NumGraphicsFile++], "%s%s", searchDir, entry->d_name);
			}
		}
	}
	closedir(dir);
	
	// Sort the graphics files to ensure correct order (readdir doesn't guarantee order)
	if(attr->NumGraphicsFile > 0) {
		qsort(attr->GraphicsFile, attr->NumGraphicsFile, MAX_NAME, compareEffectFilenames);
	}
}

Effect *
EffectManager::GetEffect(char *effectName)
{
	for(int i = 0; i < _effects.Count; ++i) {
		if(strcmp(effectName, _effects.Items[i]->GetName()) == 0) {
			Effect *e = _effects.Items[i]->Clone();
			return e;
		}
	}
	return NULL;
}

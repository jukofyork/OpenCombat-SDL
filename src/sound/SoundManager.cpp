#include "./SoundManager.h"
#include "misc/tinyxml2.h"

#include <stdio.h>
#include <math.h>
#include <SDL2/SDL_mixer.h>
#include "sound/Sound.h"
#include "application/Globals.h"

SoundManager::SoundManager(void)
{
}

SoundManager::~SoundManager(void)
{
}

void
SoundManager::LoadSounds(char *fileName)
{
	struct SoundAttributes {
		char Name[MAX_NAME];
		char SoundFile[MAX_NAME];
	};

	using namespace tinyxml2;

	XMLDocument doc;
	if (doc.LoadFile(fileName) != XML_SUCCESS) {
		printf("Failed to load sounds file: %s\n", fileName);
		return;
	}
	
	Array<SoundAttributes> dest;
	
	XMLElement* root = doc.FirstChildElement("Sounds");
	if (!root) return;
	
	for (XMLElement* soundElem = root->FirstChildElement("Sound");
		 soundElem != nullptr;
		 soundElem = soundElem->NextSiblingElement("Sound"))
	{
		SoundAttributes* attr = new SoundAttributes();
		
		XMLElement* nameElem = soundElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			strcpy(attr->Name, nameElem->GetText());
		}
		
		XMLElement* fileElem = soundElem->FirstChildElement("File");
		if (fileElem && fileElem->GetText()) {
			sprintf(attr->SoundFile, "%s", fileElem->GetText());
		}
		
		dest.Add(attr);
	}

	// Okay, now iterate through all of the sound attributes and create
	// our sounds
	char fName[256];
 
	for(int i = 0; i < dest.Count; ++i) {
		// Create the source sound file
	   sprintf(fName, "%s/%s", g_Globals->Application.SoundsDirectory, dest.Items[i]->SoundFile);
	   Sound *s = new Sound(dest.Items[i]->Name, fName);
	   // Load the WAV file using SDL_mixer
	   s->_chunk = Mix_LoadWAV(s->_soundFileName);
	   if (s->_chunk == NULL) {
		   printf("Failed to load sound: %s - %s\n", s->_soundFileName, Mix_GetError());
	   }
	   _sounds.Add(s);
	}
}

Sound *
SoundManager::GetSound(char *widgetName)
{
	for(int i = 0; i < _sounds.Count; ++i) {
		if(strcmp(widgetName, _sounds.Items[i]->GetName()) == 0) {
			return _sounds.Items[i];
		}
	}
	return NULL;
}

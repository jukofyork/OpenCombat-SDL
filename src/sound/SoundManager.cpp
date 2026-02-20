#include "./SoundManager.h"
#include "misc/tinyxml2.h"
#include "misc/Error.h"

#include <string>
#include <filesystem>
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
SoundManager::LoadSounds(const std::filesystem::path& fileName)
{
	struct SoundAttributes {
		std::string Name;
		std::string SoundFile;
	};

	using namespace tinyxml2;

	XMLDocument doc;
	if (doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
		ERROR("Failed to load sounds file: " + fileName.string());
	}
	
	std::vector<SoundAttributes> dest;
	
	XMLElement* root = doc.FirstChildElement("Sounds");
	if (!root) return;
	
	for (XMLElement* soundElem = root->FirstChildElement("Sound");
		 soundElem != nullptr;
		 soundElem = soundElem->NextSiblingElement("Sound"))
	{
		SoundAttributes attr;
		
		XMLElement* nameElem = soundElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			attr.Name = nameElem->GetText();
		}
		
		XMLElement* fileElem = soundElem->FirstChildElement("File");
		if (fileElem && fileElem->GetText()) {
			attr.SoundFile = fileElem->GetText();
		}
		
		dest.push_back(attr);
	}

	// Okay, now iterate through all of the sound attributes and create
	// our sounds
	for(size_t i = 0; i < dest.size(); ++i) {
		// Create the source sound file
	   std::filesystem::path fName = g_Globals->Application.SoundsDirectory / dest[i].SoundFile;
	   Sound *s = new Sound(dest[i].Name, fName.string());
	   // Load the WAV file using SDL_mixer
	   s->_chunk = Mix_LoadWAV(s->_soundFileName.c_str());
	   if (s->_chunk == nullptr) {
		   ERROR("Failed to load sound: " + s->_soundFileName + " - " + Mix_GetError());
	   }
	   _sounds.push_back(s);
	}
}

Sound *
SoundManager::GetSound(const std::string &soundName)
{
	for(auto* sound : _sounds) {
		if(soundName == sound->GetName()) {
			return sound;
		}
	}
	ERROR("Sound not found: " + soundName);
}

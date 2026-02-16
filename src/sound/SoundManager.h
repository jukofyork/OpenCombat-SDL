#pragma once

#include <filesystem>
#include <string>
#include <vector>

class Sound;

class SoundManager
{
public:
	SoundManager(void);
	virtual ~SoundManager(void);

	// Loads a group of sounds into this sound manager
	virtual void LoadSounds(const std::filesystem::path& fileName);

	// Retrieves a sound from this manager
	virtual Sound *GetSound(const std::string &soundName);

protected:
	// The array of sounds we are managing
	std::vector<Sound*> _sounds;
};

#pragma once

#include <string>
#include <SDL2/SDL_mixer.h>

class Sound
{
public:
	Sound(const std::string &name, const std::string &soundFile);
	virtual ~Sound(void);

	// Gets the name of this sound
	inline const char *GetName() const { return _name.c_str(); }

	// Plays this sound
	void Play();

protected:
	// The name of this sound
	std::string _name;
	// The file of this sound
	std::string _soundFileName;

	// The SDL_mixer chunk for this sound
	Mix_Chunk *_chunk;

	friend class SoundManager;
};

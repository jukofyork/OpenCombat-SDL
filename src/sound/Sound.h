#pragma once

#include <misc/Structs.h>
#include <SDL2/SDL_mixer.h>

class Sound
{
public:
	Sound(char *name, char *soundFile);
	virtual ~Sound(void);

	// Gets the name of this sound
	inline char *GetName() { return _name; }

	// Plays this sound
	void Play();

protected:
	// The name of this sound
	char _name[MAX_NAME];
	// The file of this sound
	char _soundFileName[MAX_NAME];

	// The SDL_mixer chunk for this sound
	Mix_Chunk *_chunk;

	friend class SoundManager;
};

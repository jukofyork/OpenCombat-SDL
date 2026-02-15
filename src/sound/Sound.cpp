#include "./Sound.h"

Sound::Sound(const std::string &name, const std::string &soundFile)
{
	_name = name;
	_soundFileName = soundFile;
	_chunk = NULL;
}

Sound::~Sound(void)
{
	if (_chunk != NULL)
	{
		Mix_FreeChunk(_chunk);
		_chunk = NULL;
	}
}

void
Sound::Play()
{
	if (_chunk != NULL)
	{
		Mix_PlayChannel(-1, _chunk, 0);
	}
}

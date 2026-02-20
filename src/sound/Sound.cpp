#include "./Sound.h"

Sound::Sound(const std::string &name, const std::string &soundFile)
{
	_name = name;
	_soundFileName = soundFile;
	_chunk = nullptr;
}

Sound::~Sound(void)
{
	if (_chunk != nullptr)
	{
		Mix_FreeChunk(_chunk);
		_chunk = nullptr;
	}
}

void
Sound::Play()
{
	if (_chunk != nullptr)
	{
		Mix_PlayChannel(-1, _chunk, 0);
	}
}

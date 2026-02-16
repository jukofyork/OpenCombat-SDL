#pragma once

#include <filesystem>
#include <misc/Color.h>

#define MAX_COLOR_MODIFIERS 1024

class ColorModifierManager
{
public:
	ColorModifierManager();
	virtual ~ColorModifierManager(void);

	void Load(const std::filesystem::path& configFile);
};

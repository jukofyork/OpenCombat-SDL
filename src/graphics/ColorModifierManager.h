#pragma once

#include <filesystem>
#include <misc/Color.h>

constexpr int MAX_COLOR_MODIFIERS = 1024;

class ColorModifierManager
{
public:
	ColorModifierManager();
	virtual ~ColorModifierManager(void);

	void Load(const std::filesystem::path& configFile);
};

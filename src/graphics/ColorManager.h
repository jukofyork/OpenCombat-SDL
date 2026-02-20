#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <misc/Color.h>

constexpr int MAX_COLORS = 1024;

class ColorManager
{
public:
	ColorManager();
	virtual ~ColorManager(void);

	void Load(const std::filesystem::path& configFile);
	void CopyColor(const std::string& name, Color *dest);

private:
	std::array<Color, MAX_COLORS> _colors;
	std::array<std::string, MAX_COLORS> _names;
	int _nColors;
};

#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>
#include <misc/Color.h>

class ColorManager
{
public:
	ColorManager();
	virtual ~ColorManager(void);

	void Load(const std::filesystem::path& configFile);
	void CopyColor(const std::string& name, Color *dest);

private:
	std::vector<std::pair<std::string, Color>> _colors;
};

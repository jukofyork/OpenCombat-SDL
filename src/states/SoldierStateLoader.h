#pragma once

#include <filesystem>
#include <states/ObjectStates.h>

class SoldierStateLoader
{
public:
	static void Load(const std::filesystem::path& fileName, ObjectStates *states);
};

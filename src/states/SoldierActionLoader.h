#pragma once

#include <filesystem>
#include <states/ObjectActions.h>

class SoldierActionLoader
{
public:
	static void Load(const std::filesystem::path& fileName, ObjectActions *actions);
};
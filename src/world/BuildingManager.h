#pragma once

#include <filesystem>
#include <misc/Array.h>

class TGA;
class Building;

class BuildingManager
{
public:
	// Loads a group of widgets into this widget manager
	static void LoadBuildings(const std::filesystem::path& fileName, Array<Building> *buildings);
};

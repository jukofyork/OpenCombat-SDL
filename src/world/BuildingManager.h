#pragma once

#include <filesystem>
#include <vector>

class TGA;
class Building;

class BuildingManager
{
public:
	// Loads a group of widgets into this widget manager
	static void LoadBuildings(const std::filesystem::path& fileName, std::vector<Building*> *buildings);
};

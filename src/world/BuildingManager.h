#pragma once

#include <misc/Array.h>

class TGA;
class Building;

class BuildingManager
{
public:
	// Loads a group of widgets into this widget manager
	static void LoadBuildings(const char *fileName, Array<Building> *buildings);
};

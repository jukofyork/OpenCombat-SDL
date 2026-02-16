#pragma once

#include <string>
#include <misc/Structs.h>
#include <world/VictoryLocation.h>
#include <misc/Array.h>

struct MapAttributes
{
	std::string Name;
	std::string Background;
	std::string Overland;
	std::string Mini;
	std::string Elements;
	std::string Buildings;
	Array<VictoryLocation> VictoryLocations;
};

class MapManager
{
public:
	MapManager(void);
	virtual ~MapManager(void);

	// Creates a map template from the configuration
	// file
	static MapAttributes *Parse(char *configFile);
};

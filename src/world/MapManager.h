#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <misc/Structs.h>
#include <world/VictoryLocation.h>

struct MapAttributes
{
	std::string Name;
	std::string Background;
	std::string Overland;
	std::string Mini;
	std::string Elements;
	std::string Buildings;
	std::vector<VictoryLocation*> VictoryLocations;
};

class MapManager
{
public:
	MapManager(void);
	virtual ~MapManager(void);

	// Creates a map template from the configuration
	// file
	static MapAttributes *Parse(const std::filesystem::path& configFile);
};

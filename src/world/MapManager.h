#pragma once

#include <misc/Structs.h>
#include <world/VictoryLocation.h>
#include <misc/Array.h>

#ifndef MAX_NAME
#define MAX_NAME 256
#endif

struct MapAttributes
{
	char Name[MAX_NAME];
	char Background[MAX_NAME];
	char Overland[MAX_NAME];
	char Mini[MAX_NAME];
	char Elements[MAX_NAME];
	char Buildings[MAX_NAME];
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

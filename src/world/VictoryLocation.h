#pragma once

#include <string>
#include <misc/Structs.h>

class VictoryLocation
{
public:
	VictoryLocation(void);
	virtual ~VictoryLocation(void);

	std::string Name;
	int X;
	int Y;
	int Value;
	std::string LinksToMapName;
	std::string LinksToVictoryLocationName;
	int ControllingTeam;
};

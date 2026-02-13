#pragma once

#include <misc/Structs.h>

class VictoryLocation
{
public:
	VictoryLocation(void);
	virtual ~VictoryLocation(void);

	char Name[MAX_NAME];
	int X;
	int Y;
	int Value;
	char LinksToMapName[MAX_NAME];
	char LinksToVictoryLocationName[MAX_NAME];
	int ControllingTeam;
};

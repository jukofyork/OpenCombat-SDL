#include "./VictoryLocation.h"

VictoryLocation::VictoryLocation(void)
{
	Name[0] = '\0';
	X = 0;
	Y = 0;
	Value = 0;
	LinksToMapName[0] = '\0';
	LinksToVictoryLocationName[0] = '\0';
	ControllingTeam = -1;
}

VictoryLocation::~VictoryLocation(void)
{
}

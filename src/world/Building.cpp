#include <world/Building.h>

Building::Building()
{
	_interiorGraphic = nullptr;
	_exteriorGraphic = nullptr;
	Position.x = Position.y = 0;
}

Building::~Building()
{
}

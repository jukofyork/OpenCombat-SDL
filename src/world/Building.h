#pragma once

#include <misc/Structs.h>
#include <vector>
class TGA;

class Building
{
public:
	Building();
	virtual ~Building();

	// Our position
	Point Position;

	// An array of boundary points
	std::vector<Point> BoundaryPoints;

	// An array of tile indices that this building resides on
	int *Tiles;
	int NumTiles;

	// Adds a point to the boundary
	void AddBoundaryPoint(Point *p) { BoundaryPoints.push_back(*p); }

	// Sets the interior graphic
	void SetInterior(TGA *tga) { _interiorGraphic = tga; }
	TGA *GetInterior() { return _interiorGraphic; }

	// Sets the exterior graphic
	void SetExterior(TGA *tga) { _exteriorGraphic = tga; }

protected:
	friend class BuildingManager;

	// The interior and exterior graphics
	TGA *_interiorGraphic;
	TGA *_exteriorGraphic;
};

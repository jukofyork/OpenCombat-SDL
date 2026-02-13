#pragma once

#include <misc/Array.h>

class TGA;

// NationalityID is an index into the nationalities array
typedef int NationalityID;

class Nationality
{
public:
	Nationality(void);
	virtual ~Nationality(void);

	// Load nationalities from XML file
	static void Load(char *fileName, Array<Nationality> *nationalities);

	char Name[256];
	TGA *VictoryLocation;
	TGA *MiniMap;
};

#pragma once

#include <filesystem>
#include <string>
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
	static void Load(const std::filesystem::path& fileName, Array<Nationality> *nationalities);

	std::string Name;
	TGA *VictoryLocation;
	TGA *MiniMap;
};

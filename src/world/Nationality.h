#pragma once

#include <filesystem>
#include <string>
#include <vector>

class TGA;

// NationalityID is an index into the nationalities array
typedef int NationalityID;

class Nationality
{
public:
	Nationality(void);
	virtual ~Nationality(void);

	// Load nationalities from XML file
	static void Load(const std::filesystem::path& fileName, std::vector<Nationality*> *nationalities);

	std::string Name;
	TGA *VictoryLocation;
	TGA *MiniMap;
};

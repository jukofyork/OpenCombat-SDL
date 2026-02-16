#include "./LegacyMapLoader.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

/**
 * Legacy maps are defined in the Close Combat txt files. 
 * Legacy maps are broken up into 10x10 blocks, in 4x4
 * sections. So if the map size is 2400x2280 pixels, then in
 * the map file there will be 60x57 = 3420 indices. There are 16
 * entries per index line, which means each line represents 
 * a 4x4 block of 10x10 pixels.
 */
LegacyMapLoader::LegacyMapLoader(void)
{
	_nPixelsPerBlockX = 10;
	_nPixelsPerBlockY = 10;
	_nBlocksPerMacroblockX = 4;
	_nBlocksPerMacroblockY = 4;
}

LegacyMapLoader::~LegacyMapLoader(void)
{
}

/**
 * Load a legacy map file.
 */
void
LegacyMapLoader::Load(const std::filesystem::path& mapFile)
{
	std::ifstream fp(mapFile.c_str());
	assert(fp.is_open());

	std::string line;
	int mapIdx = 0;

	// Let's read the map index
	assert(std::getline(fp, line));
	mapIdx = atoi(line.c_str());

	// Let's read a blank line
	assert(std::getline(fp, line));

	// Let's read the macroblocks in the x direction
	assert(std::getline(fp, line));
	_nMacroblocksX = atoi(line.c_str());

	// Let's read the macroblocks in the y direction
	assert(std::getline(fp, line));
	_nMacroblocksY = atoi(line.c_str());

	// Now read two lines of I don't know what they are yet
	assert(std::getline(fp, line));
	assert(std::getline(fp, line));
	
	// And a line of column headers
	assert(std::getline(fp, line));
	
	// And the '&'
	assert(std::getline(fp, line));
	
	// Allocate the elements and elevation
	_elements = (unsigned short *) calloc(_nMacroblocksX*_nMacroblocksY*_nBlocksPerMacroblockX*_nBlocksPerMacroblockY, sizeof(short));
	_elevation = (unsigned char *) calloc(_nMacroblocksX*_nMacroblocksY*_nBlocksPerMacroblockX*_nBlocksPerMacroblockY, sizeof(char));

	// Now we are ready to start reading in data
	for(int j = 0; j < _nMacroblocksY; ++j)
	{
		for(int i = 0; i < _nMacroblocksX; ++i) {
			// Let's read in this line
			assert(std::getline(fp, line));

			// Now let's tokenize it based on '\t' characters
			// and extract all of the elements out first. Our first token
			// is the index
			std::stringstream lineStream(line);
			std::string token;

			// Read and discard the index field
			assert(std::getline(lineStream, token, '\t'));

			for(int n = 0; n < _nBlocksPerMacroblockY; ++n)
			{
				for(int m = 0; m < _nBlocksPerMacroblockX; ++m)
				{
					assert(std::getline(lineStream, token, '\t'));
					_elements[(n+j*_nBlocksPerMacroblockY)*_nMacroblocksX*_nBlocksPerMacroblockX + m + i*_nBlocksPerMacroblockX] = (unsigned short)atoi(token.c_str());
				}
			}

			// Now read in the elevations
			for(int n = 0; n < _nBlocksPerMacroblockY; ++n)
			{
				for(int m = 0; m < _nBlocksPerMacroblockX; ++m)
				{
					assert(std::getline(lineStream, token, '\t'));
					_elevation[(n+j*_nBlocksPerMacroblockY)*_nMacroblocksX*_nBlocksPerMacroblockX + m + i*_nBlocksPerMacroblockX] = (unsigned char)atoi(token.c_str());
				}
			}

			// Now make sure we are at the end of the line
			assert(!std::getline(lineStream, token, '\t'));
		}
	}

	fp.close();
}

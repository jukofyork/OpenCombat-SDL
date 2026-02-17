#include "./SoldierAnimationManager.h"
#include "misc/tinyxml2.h"

#include <stdio.h>
#include <math.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <misc/Color.h>
#include <misc/Structs.h>
#include <graphics/MaskFrame.h>
#include <application/Globals.h>

using namespace tinyxml2;

SoldierAnimationManager::SoldierAnimationManager(void)
{
}

SoldierAnimationManager::~SoldierAnimationManager(void)
{
}

void
SoldierAnimationManager::LoadAnimations(const std::filesystem::path& fileName)
{
	struct AnimationAttributes {
		std::string Name;
		int nDirections;
		int nFrames;
		int Time;
		std::string FirstDirection;
		unsigned int TransparentColor;
	};

	XMLDocument doc;
	if (doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
		printf("Failed to load animations file: %s\n", fileName.c_str());
		return;
	}
	
	std::vector<AnimationAttributes> dest;
	std::string directory;
	std::string image;
	std::string mask;

	XMLElement* root = doc.FirstChildElement("Animations");
	if (!root) return;

	// Get attributes from <Animations> element
	const char* dirAttr = root->Attribute("dir");
	if (dirAttr) directory = dirAttr;

	const char* imageAttr = root->Attribute("image");
	if (imageAttr) image = imageAttr;

	const char* maskAttr = root->Attribute("mask");
	if (maskAttr) mask = maskAttr;
	
	for (XMLElement* animElem = root->FirstChildElement("Animation"); 
		 animElem != nullptr; 
		 animElem = animElem->NextSiblingElement("Animation")) 
	{
		AnimationAttributes attr;
		memset(&attr, 0, sizeof(AnimationAttributes));
		
		XMLElement* nameElem = animElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			attr.Name = nameElem->GetText();
		}
		
		XMLElement* firstDirElem = animElem->FirstChildElement("FirstDirection");
		if (firstDirElem && firstDirElem->GetText()) {
			attr.FirstDirection = firstDirElem->GetText();
		}
		
		XMLElement* dirElem = animElem->FirstChildElement("Directions");
		if (dirElem && dirElem->GetText()) {
			attr.nDirections = atoi(dirElem->GetText());
		}
		
		XMLElement* framesElem = animElem->FirstChildElement("NumFrames");
		if (framesElem && framesElem->GetText()) {
			attr.nFrames = atoi(framesElem->GetText());
		}
		
		XMLElement* timeElem = animElem->FirstChildElement("Time");
		if (timeElem && timeElem->GetText()) {
			attr.Time = atoi(timeElem->GetText());
		}
		
		XMLElement* transElem = animElem->FirstChildElement("TransparentColor");
		if (transElem && transElem->GetText()) {
			attr.TransparentColor = (unsigned int)atoi(transElem->GetText());
		}
		
		dest.push_back(attr);
	}
	
	// Replace Windows FindFirstFile with POSIX opendir/readdir
	std::vector<std::string> files;
	std::vector<std::string> masks;
	std::filesystem::path searchPath = g_Globals->Application.GraphicsDirectory / directory / image;
	std::string searchDirStr = searchPath.string();
	
	// Extract directory portion from search pattern
	size_t lastSlashPos = searchDirStr.find_last_of('/');
	if (lastSlashPos != std::string::npos) {
		std::string searchDir = searchDirStr.substr(0, lastSlashPos);
		
		DIR* dir = opendir(searchDir.c_str());
		if (dir) {
			struct dirent* entry;
			while ((entry = readdir(dir)) != NULL) {
				// Skip . and .. entries
				if (entry->d_name[0] == '.') continue;
				
				std::string entryName(entry->d_name);
				
				// Check for .tga extension (case insensitive)
				size_t extPos = entryName.find_last_of('.');
				if (extPos == std::string::npos) continue;
				
				std::string extension = entryName.substr(extPos);
				// Convert to lowercase for comparison
				std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
				
				if (extension == ".tga") {
					// Check if it starts with 'spr' (sprite files, not 'msk' mask files)
					if (entryName.substr(0, 3) == "spr") {
						std::filesystem::path filePath = g_Globals->Application.GraphicsDirectory / directory / entryName;
						files.push_back(filePath.string());

						// Add mask file (replace first 3 chars 'spr' with 'msk')
						std::string maskFile = entryName;
						if (maskFile.length() >= 3) {
							maskFile[0] = 'm';
							maskFile[1] = 's';
							maskFile[2] = 'k';
						}
						std::filesystem::path maskPath = g_Globals->Application.GraphicsDirectory / directory / maskFile;
						masks.push_back(maskPath.string());
					}
				}
			}
			closedir(dir);
			
			// Sort the files array to ensure correct order (readdir doesn't guarantee order)
			if (files.size() > 1) {
				std::sort(files.begin(), files.end());
				std::sort(masks.begin(), masks.end());
			}
			
		} else {
			printf("Failed to open directory: %s\n", searchDir.c_str());
			assert(0);
		}
	}

   // Okay, now iterate through all of the animation attributes and create
   // our frames
	size_t numFiles = 0;
	for(size_t i = 0; i < dest.size(); ++i) {
		Animation *a = new Animation(dest[i].Name);

		// Find out what our first direction is
		Direction firstDir = North;

		// We need to create one frame for each file in the directory
		// Up to the number of frames we are supposed to read in
		for(int j = 0; j < dest[i].nFrames; ++j) {
			for(int k = 0; k < dest[i].nDirections; ++k) {
				// Create the source tga
				if (numFiles >= files.size()) break;
				const std::string& fName = files[numFiles];
				const std::string& mName = masks[numFiles++];
				TGA *tga = TGA::Create(fName);
				TGA *mtga = TGA::Create(mName);

				// Let's find the hotspot for this effect. It is encoded in the
				// filename. Format: spr.XX.YY.tga where XX and YY are coordinates
				size_t lastDot = fName.find_last_of('.');
				if (lastDot == std::string::npos) continue;
				size_t secondDot = fName.find_last_of('.', lastDot - 1);
				if (secondDot == std::string::npos) continue;
				size_t thirdDot = fName.find_last_of('.', secondDot - 1);
				if (thirdDot == std::string::npos) continue;
				
				int x = atoi(fName.substr(thirdDot + 1, secondDot - thirdDot - 1).c_str());
				int y = atoi(fName.substr(secondDot + 1, lastDot - secondDot - 1).c_str());
				tga->SetOrigin(x, y);
				mtga->SetOrigin(x, y);

				// Add to our sources
				_sourceImages.push_back(tga);
				_sourceImages.push_back(mtga);

				// Parse the transparent color
				Color c;
				c.Parse(dest[i].TransparentColor);

				MaskFrame *frame = new MaskFrame(tga, mtga, dest[i].Time, tga->GetWidth(), tga->GetHeight(), 0, 0, &c);
				a->AddFrame(frame, (Direction) (((int)firstDir+k) % NumDirections));
			}
		}
		_animations.push_back(a);
	}
}

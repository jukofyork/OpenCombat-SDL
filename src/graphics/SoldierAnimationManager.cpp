#include "./SoldierAnimationManager.h"
#include "misc/tinyxml2.h"

#include <stdio.h>
#include <math.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <filesystem>
#include <misc/Color.h>
#include <misc/Structs.h>
#include <graphics/MaskFrame.h>
#include <application/Globals.h>

using namespace tinyxml2;

// Comparison function for sorting filenames
static int compareFilenames(const void *a, const void *b) {
	const char *fa = *(const char **)a;
	const char *fb = *(const char **)b;
	return strcmp(fa, fb);
}

SoldierAnimationManager::SoldierAnimationManager(void)
{
}

SoldierAnimationManager::~SoldierAnimationManager(void)
{
}

void
SoldierAnimationManager::LoadAnimations(char *fileName)
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
	if (doc.LoadFile(fileName) != XML_SUCCESS) {
		printf("Failed to load animations file: %s\n", fileName);
		return;
	}
	
	Array<AnimationAttributes> dest;
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
		AnimationAttributes* attr = new AnimationAttributes();
		memset(attr, 0, sizeof(AnimationAttributes));
		
		XMLElement* nameElem = animElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			attr->Name = nameElem->GetText();
		}
		
		XMLElement* firstDirElem = animElem->FirstChildElement("FirstDirection");
		if (firstDirElem && firstDirElem->GetText()) {
			attr->FirstDirection = firstDirElem->GetText();
		}
		
		XMLElement* dirElem = animElem->FirstChildElement("Directions");
		if (dirElem && dirElem->GetText()) {
			attr->nDirections = atoi(dirElem->GetText());
		}
		
		XMLElement* framesElem = animElem->FirstChildElement("NumFrames");
		if (framesElem && framesElem->GetText()) {
			attr->nFrames = atoi(framesElem->GetText());
		}
		
		XMLElement* timeElem = animElem->FirstChildElement("Time");
		if (timeElem && timeElem->GetText()) {
			attr->Time = atoi(timeElem->GetText());
		}
		
		XMLElement* transElem = animElem->FirstChildElement("TransparentColor");
		if (transElem && transElem->GetText()) {
			attr->TransparentColor = (unsigned int)atoi(transElem->GetText());
		}
		
		dest.Add(attr);
	}
	
	// Replace Windows FindFirstFile with POSIX opendir/readdir
	Array<char> files;
	Array<char> masks;
	std::filesystem::path searchPath = g_Globals->Application.GraphicsDirectory / directory / image;
	std::string searchDirStr = searchPath.string();
	char searchDir[512];
	snprintf(searchDir, sizeof(searchDir), "%s", searchDirStr.c_str());
	
	// Extract directory portion from search pattern
	char* lastSlash = strrchr(searchDir, '/');
	if (lastSlash) {
		*lastSlash = '\0';
		char* searchPattern = lastSlash + 1;
		
		DIR* dir = opendir(searchDir);
		if (dir) {
			struct dirent* entry;
			while ((entry = readdir(dir)) != NULL) {
				// Skip . and .. entries
				if (entry->d_name[0] == '.') continue;
				
				// Check if filename matches pattern (simplified - check extension match)
				// The pattern ends with .tga, so we check if file ends with .tga
				char* fileExt = strrchr(entry->d_name, '.');
				if (!fileExt) continue;
				
					// Check for .tga extension (case insensitive)
					if (strcasecmp(fileExt, ".tga") == 0) {
						// Check if it starts with 'spr' (sprite files, not 'msk' mask files)
						if (strncmp(entry->d_name, "spr", 3) == 0) {
							std::filesystem::path filePath = g_Globals->Application.GraphicsDirectory / directory / entry->d_name;
							files.Add(strdup(filePath.c_str()));

							// Add mask file (replace first 3 chars with 'msk')
							std::string maskPath = searchDir;
							size_t lastSlash = maskPath.find_last_of('/');
							if (lastSlash != std::string::npos && lastSlash + 3 < maskPath.length()) {
								maskPath[lastSlash + 1] = 'm';
								maskPath[lastSlash + 2] = 's';
								maskPath[lastSlash + 3] = 'k';
							}
							masks.Add(strdup(maskPath.c_str()));
						}
					}
			}
			closedir(dir);
			
			// Sort the files array to ensure correct order (readdir doesn't guarantee order)
			if(files.Count > 0) {
				qsort(files.Items, files.Count, sizeof(char *), compareFilenames);
				qsort(masks.Items, masks.Count, sizeof(char *), compareFilenames);
			}
			
		} else {
			printf("Failed to open directory: %s\n", searchDir);
			assert(0);
		}
	}

   // Okay, now iterate through all of the animation attributes and create
   // our frames
	int numFiles = 0;
	for(int i = 0; i < dest.Count; ++i) {
		Animation *a = new Animation(dest.Items[i]->Name);

		// Find out what our first direction is
		Direction firstDir = North;

		// We need to create one frame for each file in the directory
		// Up to the number of frames we are supposed to read in
		for(int j = 0; j < dest.Items[i]->nFrames; ++j) {
			for(int k = 0; k < dest.Items[i]->nDirections; ++k) {
				// Create the source tga
				char *fName = files.Items[numFiles];
				char *mName = masks.Items[numFiles++];
				TGA *tga = TGA::Create(fName);
				TGA *mtga = TGA::Create(mName);

				// Let's find the hotspot for this effect. It is encoded in the
				// filename.
				char *last = strrchr(fName, '.');
				*last = '\0';
				char *second = strrchr(fName, '.');
				*second = '\0';
				char *third = strrchr(fName, '.');
				int x = atoi(third+1);
				int y = atoi(second+1);
				tga->SetOrigin(x,y);
				mtga->SetOrigin(x,y);

				// Add to our sources
				_sourceImages.Add(tga);
				_sourceImages.Add(mtga);

				// Parse the transparent color
				Color c;
				c.Parse(dest.Items[i]->TransparentColor);

				MaskFrame *frame = new MaskFrame(tga, mtga, dest.Items[i]->Time, tga->GetWidth(), tga->GetHeight(), 0, 0, &c);
				a->AddFrame(frame, (Direction) (((int)firstDir+k) % NumDirections));
			
				delete fName;
			}
		}
		_animations.Add(a);
	}
}

#if 0
Animation *
SoldierAnimationManager::GetAnimation(char *animationName)
{
	for(int i = 0; i < _animations.Count; ++i) {
		if(strcmp(animationName, _animations.Items[i]->GetName()) == 0) {
			Animation *a = _animations.Items[i]->Clone();
			return a;
		}
	}
	return NULL;
}
#endif

#include "./TGA.h"
#include "Error.h"
#include <string.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <assert.h>

/**
 * TGA pixel data is stored in {G,B,R,A} format
 */
typedef struct {
   char  idlength;
   char  colourmaptype;
   char  datatypecode;
   short int colourmaporigin;
   short int colourmaplength;
   char  colourmapdepth;
   short int x_origin;
   short int y_origin;
   short width;
   short height;
   char  bitsperpixel;
   char  imagedescriptor;
} HEADER;

static long m_tgaIndex = 0;

TGA::TGA(void)
{
	_data = nullptr;
	_originX = 0;
	_originY = 0;
	_idx = m_tgaIndex++;
}

TGA::~TGA(void)
{
	if(_data) {
		delete[] _data;
	}
}

TGA *
TGA::Create(const std::filesystem::path& filePath)
{
	HEADER header;
	TGA *tga;
    int n=0,i,j;
    unsigned int bytes2read;
	int skipover = 0;
	unsigned char p[5];
	unsigned char *ptr;
	int w=0, h=0;

	// Open the file
	std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
		ERROR("Failed to open TGA file: " + filePath.string());
		return nullptr;
    }

	// Create the return value
	tga = new TGA();

    // Read in the header
    header.idlength = static_cast<char>(file.get());
    header.colourmaptype = static_cast<char>(file.get());
    header.datatypecode = static_cast<char>(file.get());
    file.read(reinterpret_cast<char*>(&header.colourmaporigin), 2);
    file.read(reinterpret_cast<char*>(&header.colourmaplength), 2);
    header.colourmapdepth = static_cast<char>(file.get());
    file.read(reinterpret_cast<char*>(&header.x_origin), 2);
    file.read(reinterpret_cast<char*>(&header.y_origin), 2);
    file.read(reinterpret_cast<char*>(&header.width), 2);
    file.read(reinterpret_cast<char*>(&header.height), 2);
    header.bitsperpixel = static_cast<char>(file.get());
    header.imagedescriptor = static_cast<char>(file.get());
	tga->_width = header.width;
	tga->_height = header.height;
	tga->_depth = 4;

	// Stored as 32 bit ARGB
    tga->_data = new unsigned char[header.width*header.height*4]();
    if(tga->_data == nullptr) {
	   return nullptr;
    }
    ptr = tga->_data;
	
	skipover += header.idlength;
    skipover += header.colourmaptype * header.colourmaplength;
    file.seekg(skipover, std::ios::cur);

	bytes2read = header.bitsperpixel / 8;
    while(n < header.width * header.height) {
		if(header.datatypecode == 2) {
			file.read(reinterpret_cast<char*>(p), bytes2read);
			if (!file.good()) {
				return nullptr;
			}
			if(w >= header.width) {
				w -= header.width;
				++h;
			}
			//memcpy(ptr, p, bytes2read);
			//ptr += bytes2read;
			if(header.bitsperpixel == 32) {
				memcpy(&(ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth]), p, bytes2read);
			}
			else if(header.bitsperpixel == 24) {
				memcpy(&(ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth]), p, bytes2read);
				ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth+3] = 0xFF;				
			}
			else if(header.bitsperpixel == 16)
			{
				unsigned short pixel = ((unsigned short *)p)[0];
				ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth+0] = (unsigned char)((pixel << 3 & 0xff) + 7);
				ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth+1] = (unsigned char)(((pixel >> 5) << 3 & 0xff) + 7);
				ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth+2] = (unsigned char)(((pixel >> 10) << 3 & 0xff) + 7);
				ptr[(header.height-h-1)*header.width*tga->_depth + w*tga->_depth+3] = 0xFF;
			}
			n++;
			++w;
		} else if (header.datatypecode == 10) {
			// Compressed
			assert(0); // Not implemented yet, needs to flip the bits
			file.read(reinterpret_cast<char*>(p), bytes2read + 1);
			if (!file.good()) {
				return nullptr;
			}
	        j = p[0] & 0x7f;
			memcpy(ptr, &(p[1]), bytes2read);
			ptr += bytes2read;
			n++;
			
			if(p[0] & 0x80) {         /* RLE chunk */
				for (i=0;i<j;i++) {
					memcpy(ptr, &(p[1]), bytes2read);
					ptr += bytes2read;
					n++;
				}
			} else {                   /* Normal chunk */
				for (i=0;i<j;i++) {
					file.read(reinterpret_cast<char*>(p), bytes2read);
					if (!file.good()) {
						return nullptr;
					}
					memcpy(ptr, p, bytes2read);
					ptr += bytes2read;
                	n++;
				}
			}
        }
    }

	// Let's find our origin, if it is embedded in the filename.
	// Format: name.x.y.tga where x and y are origin coordinates
	// Use filename only (not full path) to avoid counting dots in directory names
	std::string fName = filePath.filename().string();
	
	// Strip the extension first (.tga)
	size_t extDot = fName.rfind('.');
	if (extDot != std::string::npos) {
		fName = fName.substr(0, extDot);
		// Find the Y coordinate (after the last remaining dot)
		size_t yDot = fName.rfind('.');
		if (yDot != std::string::npos) {
			std::string yStr = fName.substr(yDot + 1);
			// Find the X coordinate
			fName = fName.substr(0, yDot);
			size_t xDot = fName.rfind('.');
			if (xDot != std::string::npos) {
				std::string xStr = fName.substr(xDot + 1);
				int x = atoi(xStr.c_str());
				int y = atoi(yStr.c_str());
				tga->SetOrigin(x, y);
			}
		}
	}

	return tga;
}


#include "./AnimationManager.h"
#include "misc/tinyxml2.h"
#include <misc/Color.h>
#include <misc/Error.h>
#include <string>
#include <filesystem>

using namespace tinyxml2;

AnimationManager::AnimationManager(void)
{
}

AnimationManager::~AnimationManager(void)
{
}

void
AnimationManager::LoadAnimations(const std::filesystem::path& fileName)
{
    struct AnimationAttributes {
        std::string Name;
        std::string GraphicsFile;
        int nDirections;
        int nFrames;
        int Width;
        int Height;
        int Time;
        unsigned int TransparentColor;
    };

    XMLDocument doc;
    if (doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
        ERROR("Failed to load animations file: " + fileName.string());
    }
    
    std::vector<AnimationAttributes> dest;
    
    XMLElement* root = doc.FirstChildElement("Animations");
    if (!root) return;
    
    for (XMLElement* animElem = root->FirstChildElement("Animation"); 
         animElem != nullptr; 
         animElem = animElem->NextSiblingElement("Animation")) 
    {
        AnimationAttributes attr;
        attr.nDirections = 0;
        attr.nFrames = 0;
        attr.Width = 0;
        attr.Height = 0;
        attr.Time = 0;
        attr.TransparentColor = 0;
        
        XMLElement* nameElem = animElem->FirstChildElement("Name");
        if (nameElem && nameElem->GetText()) {
            attr.Name = nameElem->GetText();
        }
        
        XMLElement* graphicElem = animElem->FirstChildElement("Graphic");
        if (graphicElem && graphicElem->GetText()) {
            attr.GraphicsFile = graphicElem->GetText();
        }
        
        XMLElement* dirElem = animElem->FirstChildElement("Directions");
        if (dirElem && dirElem->GetText()) {
            attr.nDirections = atoi(dirElem->GetText());
        }
        
        XMLElement* framesElem = animElem->FirstChildElement("Frames");
        if (framesElem && framesElem->GetText()) {
            attr.nFrames = atoi(framesElem->GetText());
        }
        
        XMLElement* widthElem = animElem->FirstChildElement("Width");
        if (widthElem && widthElem->GetText()) {
            attr.Width = atoi(widthElem->GetText());
        }
        
        XMLElement* heightElem = animElem->FirstChildElement("Height");
        if (heightElem && heightElem->GetText()) {
            attr.Height = atoi(heightElem->GetText());
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
    
    // Okay, now iterate through all of the animation attributes and create
    // our frames
    for(size_t i = 0; i < dest.size(); ++i) {
        Animation *a = new Animation(dest[i].Name);

        // Create the source TGA file
        TGA *tga = TGA::Create(dest[i].GraphicsFile);
        _sourceImages.push_back(tga);

        // Parse the transparent color
        Color c;
        c.Parse(dest[i].TransparentColor);

        // Create all of the frames
        for(int dir = 0; dir < dest[i].nDirections; ++dir) {
            for(int n = 0; n < dest[i].nFrames; ++n) {
                Frame *frame = new Frame(tga, dest[i].Time, dest[i].Width, dest[i].Height, 
                    n*dest[i].Width, dir*dest[i].Height, &c);
                a->AddFrame(frame, static_cast<Direction>(dir));
            }
        }
        _animations.push_back(a);
    }
}

Animation *
AnimationManager::GetAnimation(const std::string& animationName)
{
    for(auto* animation : _animations) {
        if(animationName == animation->GetName()) {
            return animation->Clone();
        }
    }
	ERROR("Animation not found: " + animationName);
}

#include "./AnimationManager.h"
#include "misc/tinyxml2.h"
#include <misc/Color.h>
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
        printf("Failed to load animations file: %s\n", fileName.c_str());
        return;
    }
    
    Array<AnimationAttributes> dest;
    
    XMLElement* root = doc.FirstChildElement("Animations");
    if (!root) return;
    
    for (XMLElement* animElem = root->FirstChildElement("Animation"); 
         animElem != nullptr; 
         animElem = animElem->NextSiblingElement("Animation")) 
    {
        AnimationAttributes* attr = new AnimationAttributes();
        attr->nDirections = 0;
        attr->nFrames = 0;
        attr->Width = 0;
        attr->Height = 0;
        attr->Time = 0;
        attr->TransparentColor = 0;
        
        XMLElement* nameElem = animElem->FirstChildElement("Name");
        if (nameElem && nameElem->GetText()) {
            attr->Name = nameElem->GetText();
        }
        
        XMLElement* graphicElem = animElem->FirstChildElement("Graphic");
        if (graphicElem && graphicElem->GetText()) {
            attr->GraphicsFile = graphicElem->GetText();
        }
        
        XMLElement* dirElem = animElem->FirstChildElement("Directions");
        if (dirElem && dirElem->GetText()) {
            attr->nDirections = atoi(dirElem->GetText());
        }
        
        XMLElement* framesElem = animElem->FirstChildElement("Frames");
        if (framesElem && framesElem->GetText()) {
            attr->nFrames = atoi(framesElem->GetText());
        }
        
        XMLElement* widthElem = animElem->FirstChildElement("Width");
        if (widthElem && widthElem->GetText()) {
            attr->Width = atoi(widthElem->GetText());
        }
        
        XMLElement* heightElem = animElem->FirstChildElement("Height");
        if (heightElem && heightElem->GetText()) {
            attr->Height = atoi(heightElem->GetText());
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
    
    // Okay, now iterate through all of the animation attributes and create
    // our frames
    for(int i = 0; i < dest.Count; ++i) {
        Animation *a = new Animation(dest.Items[i]->Name);

        // Create the source TGA file
        TGA *tga = TGA::Create(dest.Items[i]->GraphicsFile);
        _sourceImages.Add(tga);

        // Parse the transparent color
        Color c;
        c.Parse(dest.Items[i]->TransparentColor);

        // Create all of the frames
        for(int dir = 0; dir < dest.Items[i]->nDirections; ++dir) {
            for(int n = 0; n < dest.Items[i]->nFrames; ++n) {
                Frame *frame = new Frame(tga, dest.Items[i]->Time, dest.Items[i]->Width, dest.Items[i]->Height, 
                    n*dest.Items[i]->Width, dir*dest.Items[i]->Height, &c);
                a->AddFrame(frame, (Direction) dir);
            }
        }
        _animations.Add(a);
    }
}

Animation *
AnimationManager::GetAnimation(const std::string& animationName)
{
    for(int i = 0; i < _animations.Count; ++i) {
        if(animationName == _animations.Items[i]->GetName()) {
            Animation *a = _animations.Items[i]->Clone();
            return a;
        }
    }
    return NULL;
}

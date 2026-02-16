#include "./ElementManager.h"
#include "misc/tinyxml2.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <filesystem>
#include <misc/TGA.h>
#include <world/Element.h>

using namespace tinyxml2;

ElementManager::ElementManager(void)
{
}

ElementManager::~ElementManager(void)
{
}

void
ElementManager::Load(const std::filesystem::path& configFile)
{
    XMLDocument doc;
    if (doc.LoadFile(configFile.c_str()) != XML_SUCCESS) {
        printf("Failed to load elements file: %s\n", configFile.c_str());
        return;
    }
    
    XMLElement* root = doc.FirstChildElement("Elements");
    if (!root) return;
    
    int elementIndex = 0;
    for (XMLElement* elem = root->FirstChildElement("Element");
         elem != nullptr;
         elem = elem->NextSiblingElement("Element"))
    {
        Element* element = new Element();
        element->Index = elementIndex++;
        
        XMLElement* nameElem = elem->FirstChildElement("Name");
        if (nameElem && nameElem->GetText()) {
            element->Name = nameElem->GetText();
        }
        
        XMLElement* heightElem = elem->FirstChildElement("Height");
        if (heightElem && heightElem->GetText()) {
            element->Height = atoi(heightElem->GetText());
        }
        
        XMLElement* blockElem = elem->FirstChildElement("Block_Height");
        if (blockElem && blockElem->GetText()) {
            element->BlocksHeight = (atoi(blockElem->GetText()) != 0);
        }
        
        XMLElement* passElem = elem->FirstChildElement("Passable");
        if (passElem && passElem->GetText()) {
            const char* passText = passElem->GetText();
            // Handle both "true"/"false" and "1"/"0"
            if (strcmp(passText, "true") == 0 || strcmp(passText, "1") == 0) {
                element->Passable = true;
            } else {
                element->Passable = false;
            }
        }
        
        // Parse Cover values
        XMLElement* coverProne = elem->FirstChildElement("Cover_Prone");
        if (coverProne && coverProne->GetText()) {
            element->Cover[Element::Prone] = (unsigned char)atoi(coverProne->GetText());
        }
        
        XMLElement* coverLow = elem->FirstChildElement("Cover_Low");
        if (coverLow && coverLow->GetText()) {
            element->Cover[Element::Low] = (unsigned char)atoi(coverLow->GetText());
        }
        
        XMLElement* coverMed = elem->FirstChildElement("Cover_Medium");
        if (coverMed && coverMed->GetText()) {
            element->Cover[Element::Medium] = (unsigned char)atoi(coverMed->GetText());
        }
        
        XMLElement* coverHigh = elem->FirstChildElement("Cover_High");
        if (coverHigh && coverHigh->GetText()) {
            element->Cover[Element::High] = (unsigned char)atoi(coverHigh->GetText());
        }
        
        // Parse Protection values
        XMLElement* protProne = elem->FirstChildElement("Protection_Prone");
        if (protProne && protProne->GetText()) {
            element->Protection[Element::Prone] = (unsigned short)atoi(protProne->GetText());
        }
        
        XMLElement* protLow = elem->FirstChildElement("Protection_Low");
        if (protLow && protLow->GetText()) {
            element->Protection[Element::Low] = (unsigned short)atoi(protLow->GetText());
        }
        
        XMLElement* protMed = elem->FirstChildElement("Protection_Medium");
        if (protMed && protMed->GetText()) {
            element->Protection[Element::Medium] = (unsigned short)atoi(protMed->GetText());
        }
        
        XMLElement* protHigh = elem->FirstChildElement("Protection_High");
        if (protHigh && protHigh->GetText()) {
            element->Protection[Element::High] = (unsigned short)atoi(protHigh->GetText());
        }
        
        XMLElement* protTop = elem->FirstChildElement("Protection_Top");
        if (protTop && protTop->GetText()) {
            element->Protection[Element::Top] = (unsigned short)atoi(protTop->GetText());
        }
        
        // Parse Hindrance values
        XMLElement* hindProne = elem->FirstChildElement("Hindrance_Prone");
        if (hindProne && hindProne->GetText()) {
            element->Hindrance[Element::Prone] = (unsigned char)atoi(hindProne->GetText());
        }
        
        XMLElement* hindLow = elem->FirstChildElement("Hindrance_Low");
        if (hindLow && hindLow->GetText()) {
            element->Hindrance[Element::Low] = (unsigned char)atoi(hindLow->GetText());
        }
        
        XMLElement* hindMed = elem->FirstChildElement("Hindrance_Medium");
        if (hindMed && hindMed->GetText()) {
            element->Hindrance[Element::Medium] = (unsigned char)atoi(hindMed->GetText());
        }
        
        XMLElement* hindHigh = elem->FirstChildElement("Hindrance_High");
        if (hindHigh && hindHigh->GetText()) {
            element->Hindrance[Element::High] = (unsigned char)atoi(hindHigh->GetText());
        }
        
        // Parse Movement values
        XMLElement* moveProne = elem->FirstChildElement("Soldier_Move_Prone");
        if (moveProne && moveProne->GetText()) {
            element->Movement[0] = (float)atof(moveProne->GetText());
        }
        
        XMLElement* moveCrouch = elem->FirstChildElement("Soldier_Move_Crouch");
        if (moveCrouch && moveCrouch->GetText()) {
            element->Movement[1] = (float)atof(moveCrouch->GetText());
        }
        
        XMLElement* moveStand = elem->FirstChildElement("Soldier_Move_Standing");
        if (moveStand && moveStand->GetText()) {
            element->Movement[2] = (float)atof(moveStand->GetText());
        }
        
        _elements.push_back(element);
    }
}

Element *
ElementManager::GetElement(int index)
{
    if (index < 0 || index >= static_cast<int>(_elements.size())) {
        printf("ERROR: GetElement(%d) - index out of bounds (size=%zu)\n", index, _elements.size());
        return nullptr;
    }
    return _elements[index];
}

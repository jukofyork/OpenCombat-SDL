#include "./MapManager.h"
#include "misc/tinyxml2.h"

#include <string>
#include <filesystem>
#include <stdio.h>
#include <math.h>
#include <application/Globals.h>

using namespace tinyxml2;

MapManager::MapManager(void)
{
}

MapManager::~MapManager(void)
{
}

MapAttributes *
MapManager::Parse(char *fileName)
{
    XMLDocument doc;
    if (doc.LoadFile(fileName) != XML_SUCCESS) {
        printf("Failed to load map file: %s\n", fileName);
        return nullptr;
    }
    
    XMLElement* root = doc.FirstChildElement("Map");
    if (!root) return nullptr;
    
    MapAttributes* attr = new MapAttributes();
    
    // Parse Map Name
    XMLElement* nameElem = root->FirstChildElement("Name");
    if (nameElem && nameElem->GetText()) {
        attr->Name = nameElem->GetText();
    }
    
    // Parse Background
    XMLElement* bgElem = root->FirstChildElement("Background");
    if (bgElem && bgElem->GetText()) {
        attr->Background = (g_Globals->Application.MapsDirectory / bgElem->GetText()).string();
    }

    // Parse Buildings
    XMLElement* buildingsElem = root->FirstChildElement("Buildings");
    if (buildingsElem && buildingsElem->GetText()) {
        attr->Buildings = (g_Globals->Application.MapsDirectory / buildingsElem->GetText()).string();
    }

    // Parse Elements
    XMLElement* elementsElem = root->FirstChildElement("Elements");
    if (elementsElem && elementsElem->GetText()) {
        attr->Elements = (g_Globals->Application.MapsDirectory / elementsElem->GetText()).string();
    }

    // Parse Mini
    XMLElement* miniElem = root->FirstChildElement("Mini");
    if (miniElem && miniElem->GetText()) {
        attr->Mini = (g_Globals->Application.MapsDirectory / miniElem->GetText()).string();
    }

    // Parse Overland
    XMLElement* overlandElem = root->FirstChildElement("Overland");
    if (overlandElem && overlandElem->GetText()) {
        attr->Overland = (g_Globals->Application.MapsDirectory / overlandElem->GetText()).string();
    }
    
    // Parse Victory Locations
    XMLElement* vlRoot = root->FirstChildElement("VictoryLocations");
    if (vlRoot) {
        for (XMLElement* vlElem = vlRoot->FirstChildElement("VL");
             vlElem != nullptr;
             vlElem = vlElem->NextSiblingElement("VL"))
        {
            VictoryLocation* vl = new VictoryLocation();
            
            XMLElement* vlNameElem = vlElem->FirstChildElement("Name");
            if (vlNameElem && vlNameElem->GetText()) {
                vl->Name = vlNameElem->GetText();
            }
            
            XMLElement* xElem = vlElem->FirstChildElement("X");
            if (xElem && xElem->GetText()) {
                vl->X = atoi(xElem->GetText());
            }
            
            XMLElement* yElem = vlElem->FirstChildElement("Y");
            if (yElem && yElem->GetText()) {
                vl->Y = atoi(yElem->GetText());
            }
            
            XMLElement* valueElem = vlElem->FirstChildElement("Value");
            if (valueElem && valueElem->GetText()) {
                vl->Value = atoi(valueElem->GetText());
            }
            
            XMLElement* linksElem = vlElem->FirstChildElement("LinksTo");
            if (linksElem) {
                XMLElement* mapNameElem = linksElem->FirstChildElement("MapName");
                if (mapNameElem && mapNameElem->GetText()) {
                    vl->LinksToMapName = mapNameElem->GetText();
                }
                
                XMLElement* vlNameElem = linksElem->FirstChildElement("VLName");
                if (vlNameElem && vlNameElem->GetText()) {
                    vl->LinksToVictoryLocationName = vlNameElem->GetText();
                }
            }
            
            attr->VictoryLocations.Add(vl);
        }
    }
    
    return attr;
}

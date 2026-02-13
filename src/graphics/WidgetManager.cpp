#include "./WidgetManager.h"
#include "misc/tinyxml2.h"

#include <stdio.h>
#include <math.h>
#include <misc/Stack.h>
#include <misc/TGA.h>
#include <graphics/Widget.h>
#include <application/Globals.h>

using namespace tinyxml2;

struct WidgetAttributes 
{
	char Name[MAX_NAME];
	char GraphicsFile[MAX_NAME];
	int Index;
};

WidgetManager::WidgetManager(void)
{
}

WidgetManager::~WidgetManager(void)
{
}

void
WidgetManager::LoadWidgets(char *fileName)
{
	XMLDocument doc;
	if (doc.LoadFile(fileName) != XML_SUCCESS) {
		printf("Failed to load widgets file: %s\n", fileName);
		return;
	}

	Array<WidgetAttributes> dest;

	XMLElement* root = doc.FirstChildElement("Widgets");
	if (!root) return;

	for (XMLElement* widgetElem = root->FirstChildElement("Widget"); 
		 widgetElem != nullptr; 
		 widgetElem = widgetElem->NextSiblingElement("Widget")) 
	{
		WidgetAttributes* attr = new WidgetAttributes();
		attr->Index = -1;

		XMLElement* nameElem = widgetElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			strcpy(attr->Name, nameElem->GetText());
		}

		XMLElement* graphicElem = widgetElem->FirstChildElement("Graphic");
		if (graphicElem && graphicElem->GetText()) {
			sprintf(attr->GraphicsFile, "%s", graphicElem->GetText());
		}

		XMLElement* indexElem = widgetElem->FirstChildElement("Index");
		if (indexElem && indexElem->GetText()) {
			attr->Index = atoi(indexElem->GetText());
		}

		dest.Add(attr);
	}

	// Okay, now iterate through all of the widget attributes and create
	// our widgets
	char fName[256];

	for(int i = 0; i < dest.Count; ++i) {
		// Create the source TGA file
		sprintf(fName, "%s/%s", g_Globals->Application.GraphicsDirectory, dest.Items[i]->GraphicsFile); 
		TGA *tga = TGA::Create(fName);
		_sourceImages.Add(tga);
		Widget *w = new Widget(dest.Items[i]->Name, tga);
		if(dest.Items[i]->Index == -1) {
			w->SetIndex(i);
		} else {
			w->SetIndex(dest.Items[i]->Index);
		}
		_widgets.Add(w);
	}
}

Widget *
WidgetManager::GetWidget(char *widgetName)
{
	for(int i = 0; i < _widgets.Count; ++i) {
		if(strcmp(widgetName, _widgets.Items[i]->GetName()) == 0) {
			Widget *w = _widgets.Items[i]->Clone();
			return w;
		}
	}
	return NULL;
}

Widget *
WidgetManager::GetWidget(int index)
{
	return GetWidget(index, true);
}

Widget *
WidgetManager::GetWidget(int index, bool clone)
{
	for(int i = 0; i < _widgets.Count; ++i) {
		if(index == _widgets.Items[i]->GetIndex()) {
			if(clone) {
				Widget *w = _widgets.Items[i]->Clone();
				return w;
			} else {
				return _widgets.Items[i];
			}
		}
	}
	return NULL;
}

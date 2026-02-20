#include "./WidgetManager.h"
#include "misc/tinyxml2.h"
#include "misc/Error.h"

#include <math.h>
#include <string>
#include <filesystem>
#include <misc/TGA.h>
#include <graphics/Widget.h>
#include <application/Globals.h>

using namespace tinyxml2;

struct WidgetAttributes
{
	std::string Name;
	std::string GraphicsFile;
	int Index;
};

WidgetManager::WidgetManager(void)
{
}

WidgetManager::~WidgetManager(void)
{
}

void
WidgetManager::LoadWidgets(const std::filesystem::path& fileName)
{
	XMLDocument doc;
	if (doc.LoadFile(fileName.c_str()) != XML_SUCCESS) {
		ERROR("Failed to load widgets file: " + fileName.string());
	}

	std::vector<WidgetAttributes> dest;

	XMLElement* root = doc.FirstChildElement("Widgets");
	if (!root) return;

	for (XMLElement* widgetElem = root->FirstChildElement("Widget"); 
		 widgetElem != nullptr; 
		 widgetElem = widgetElem->NextSiblingElement("Widget")) 
	{
		WidgetAttributes attr;
		attr.Index = -1;

		XMLElement* nameElem = widgetElem->FirstChildElement("Name");
		if (nameElem && nameElem->GetText()) {
			attr.Name = nameElem->GetText();
		}

		XMLElement* graphicElem = widgetElem->FirstChildElement("Graphic");
		if (graphicElem && graphicElem->GetText()) {
			attr.GraphicsFile = graphicElem->GetText();
		}

		XMLElement* indexElem = widgetElem->FirstChildElement("Index");
		if (indexElem && indexElem->GetText()) {
			attr.Index = atoi(indexElem->GetText());
		}

		dest.push_back(attr);
	}

	// Okay, now iterate through all of the widget attributes and create
	// our widgets
	for(size_t i = 0; i < dest.size(); ++i) {
		// Create the source TGA file
		std::filesystem::path fName = g_Globals->Application.GraphicsDirectory / dest[i].GraphicsFile;
		TGA *tga = TGA::Create(fName.c_str());
		_sourceImages.push_back(tga);
		Widget *w = new Widget(dest[i].Name, tga);
		if(dest[i].Index == -1) {
			w->SetIndex(static_cast<int>(i));
		} else {
			w->SetIndex(dest[i].Index);
		}
		_widgets.push_back(w);
	}
}

Widget *
WidgetManager::GetWidget(const std::string& widgetName)
{
	for(auto* widget : _widgets) {
		if(widgetName == widget->GetName()) {
			return widget->Clone();
		}
	}
	ERROR("Widget not found: " + widgetName);
}

Widget *
WidgetManager::GetWidget(int index)
{
	return GetWidget(index, true);
}

Widget *
WidgetManager::GetWidget(int index, bool clone)
{
	for(auto* widget : _widgets) {
		if(index == widget->GetIndex()) {
			if(clone) {
				return widget->Clone();
			} else {
				return widget;
			}
		}
	}
	return nullptr;
}

#pragma once

#include <filesystem>
#include <string>
#include <vector>

class TGA;
class Widget;

class WidgetManager
{
public:
	WidgetManager(void);
	virtual ~WidgetManager(void);

	// Loads a group of widgets into this widget manager
	void LoadWidgets(const std::filesystem::path& fileName);

	// Retrieves a widget from this manager
	Widget *GetWidget(const std::string& widgetName);

	// Retrieves a widget by index
	Widget *GetWidget(int index);
	Widget *GetWidget(int index, bool clone);

protected:
	// The array of widgets we are managing
	std::vector<Widget*> _widgets;

	// The array of source images for these widgets
	std::vector<TGA*> _sourceImages;
};

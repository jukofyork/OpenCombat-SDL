#pragma once

#include <filesystem>
#include <misc/Array.h>

class TGA;
class Element;

class ElementManager
{
public:
	ElementManager(void);
	virtual ~ElementManager(void);

	// Loads a group of elements from an XML file
	void Load(const std::filesystem::path& configFile);

	// Retrieves a widget by index
	Element *GetElement(int index);

protected:
	// The array of widgets we are managing
	Array<Element> _elements;
};

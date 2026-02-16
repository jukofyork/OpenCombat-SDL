#pragma once

#include <filesystem>
#include <vector>

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
	// The array of elements we are managing
	std::vector<Element*> _elements;
};

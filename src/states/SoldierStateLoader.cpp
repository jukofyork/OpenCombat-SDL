#include <states/SoldierStateLoader.h>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

// Helper function to trim whitespace from both ends of a string
static std::string trim(const std::string &s)
{
	const std::string ws = " \t\n\r\f\v";
	size_t start = s.find_first_not_of(ws);
	if (start == std::string::npos) return "";
	size_t end = s.find_last_not_of(ws);
	return s.substr(start, end - start + 1);
}

void
SoldierStateLoader::Load(const std::filesystem::path& fileName, ObjectStates *states)
{
	std::string line;

	// We need to read in the file and load the states deal thing
	std::ifstream fp(fileName.c_str());
	while(std::getline(fp, line)) {
		// Let's trim our string
		std::string trimmed = trim(line);
		if(!trimmed.empty()) {
			states->StateNames.push_back(trimmed);
			states->States.push_back((int)states->States.size());
		}
	}
	fp.close();
}

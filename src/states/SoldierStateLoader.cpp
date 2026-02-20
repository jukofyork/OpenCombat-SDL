#include <states/SoldierStateLoader.h>
#include <misc/StringUtils.h>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

void
SoldierStateLoader::Load(const std::filesystem::path& fileName, ObjectStates *states)
{
	std::string line;

	// We need to read in the file and load the states deal thing
	std::ifstream fp(fileName.c_str());
	while(std::getline(fp, line)) {
		// Let's trim our string
		std::string trimmed = StringUtils::trim(line);
		if(!trimmed.empty()) {
			states->StateNames.push_back(trimmed);
			states->States.push_back(static_cast<int>(states->States.size()));
		}
	}
	fp.close();
}

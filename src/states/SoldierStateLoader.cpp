#include <states/SoldierStateLoader.h>
#include <misc/Array.h>
#include <string>
#include <fstream>
#include <filesystem>

// Helper function to trim whitespace from both ends of a string
static std::string trim(const std::string &s)
{
	const char *ws = " \t\n\r\f\v";
	size_t start = s.find_first_not_of(ws);
	if (start == std::string::npos) return "";
	size_t end = s.find_last_not_of(ws);
	return s.substr(start, end - start + 1);
}

void
SoldierStateLoader::Load(const std::filesystem::path& fileName, ObjectStates *states)
{
	std::string line;
	Array<char> stateNames;

	// We need to read in the file and load the states deal thing
	std::ifstream fp(fileName.c_str());
	while(std::getline(fp, line)) {
		// Let's trim our string
		std::string trimmed = trim(line);
		if(!trimmed.empty()) {
			stateNames.Add(strdup(trimmed.c_str()));
		}
	}
	fp.close();

	// Now go back through the array and add to our dest
	states->NumStates = stateNames.Count;
	states->StateNames = (char **) calloc(states->NumStates, sizeof(char *));
	states->States = (int *) calloc(states->NumStates, sizeof(int));
	for(int i = 0; i < stateNames.Count; ++i) {
		states->States[i] = i;
		states->StateNames[i] = stateNames.Items[i];
	}
}

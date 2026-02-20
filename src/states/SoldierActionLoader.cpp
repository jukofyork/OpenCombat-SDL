#include <states/SoldierActionLoader.h>
#include <application/Globals.h>
#include <misc/StringUtils.h>
#include <assert.h>
#include <string.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>

static ObjectActions::StateIdx find_state(const std::string& stateName);

// Helper to split a string by delimiter
static void split(const std::string &s, char delim, std::vector<std::string> &result)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		std::string trimmed = StringUtils::trim(item);
		if (!trimmed.empty()) {
			result.push_back(trimmed);
		}
	}
}

void
SoldierActionLoader::Load(const std::filesystem::path& fileName, ObjectActions *actions)
{
	std::string line;
	std::vector<std::string> reqs;
	std::vector<std::string> changes;

	// We cannot load our actions if we do not already have a bunch of states
	assert(g_Globals->World.States.Soldiers.StateNames.size() > 0);

	// Let's loop through our input file
	std::ifstream fp(fileName.c_str());
	
	// Let's pick off the first line, which is our column headers
	assert(std::getline(fp, line));

	// Now let's go through each line
	actions->NumActions = 0;
	actions->Actions = nullptr;
	while(std::getline(fp, line)) 
	{
		// Let's tokenize this buffer based on the '\t' character
		// Note: file uses multiple tabs for alignment, so we need to skip empty tokens
		std::string values[5];
		int i = 0;
		std::stringstream linestream(line);
		std::string token;
		while (std::getline(linestream, token, '\t') && i < 5)
		{
			std::string trimmed = StringUtils::trim(token);
			if (!trimmed.empty()) {
				values[i++] = trimmed;
			}
		}

		// If we didnt get enough then stop looping
		if(i < 5) 
		{
			break;
		}

		// Now we need to create a new action based on our values
		// Allocate new array with proper C++ construction
		ObjectActions::Action *newActions = new ObjectActions::Action[actions->NumActions + 1];
		
		// Copy existing actions (if any)
		for(int j = 0; j < actions->NumActions; ++j) {
			newActions[j] = actions->Actions[j];
		}
		
		// Free old array
		delete[] actions->Actions;
		
		// Update to new array
		actions->Actions = newActions;
		actions->NumActions++;
		ObjectActions::Action *action = &(actions->Actions[actions->NumActions-1]);

		action->Name = values[0];
		action->Group = values[1];
		action->Time = atol(values[2].c_str());

		// Let's get our requirements
		reqs.clear();
		if (values[3] != "nil") {
			split(values[3], ',', reqs);
		}
		
		action->Requirements.clear();
		action->Requirements.reserve(reqs.size());
		for(i = 0; i < static_cast<int>(reqs.size()); ++i)
		{
			action->Requirements.push_back(find_state(reqs[i]));
		}

		// Let's get our changes
		changes.clear();
		std::stringstream changesStream(values[4]);
		while (std::getline(changesStream, token, ','))
		{
			std::string trimmed = StringUtils::trim(token);
			if(!trimmed.empty() && (trimmed[0] == '+' || trimmed[0] == '-'))
			{
				changes.push_back(trimmed);
			}
			// Ignore anything else
		}

		action->Adds.clear();
		action->Subtracts.clear();
		action->Adds.reserve(changes.size());
		action->Subtracts.reserve(changes.size());
		for(i = 0; i < static_cast<int>(changes.size()); ++i)
		{
			const std::string& changeStr = changes[i];
			char op = changeStr[0];
			std::string stateName = changeStr.substr(1);
			if(op == '+')
			{
				action->Adds.push_back(find_state(stateName));
			}
			else if(op == '-')
			{
				action->Subtracts.push_back(find_state(stateName));
			}
			else
			{
				assert(0);
			}
		}
	}
	fp.close();
}

ObjectActions::StateIdx
find_state(const std::string& stateName)
{
	for(size_t i = 0; i < g_Globals->World.States.Soldiers.StateNames.size(); ++i)
	{
		if(stateName == g_Globals->World.States.Soldiers.StateNames[i])
		{
			return (ObjectActions::StateIdx)i;
		}
	}
	assert(0);
	return 0;
}

#include <states/SoldierActionLoader.h>
#include <application/Globals.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>

static ObjectActions::StateIdx find_state(const std::string& stateName);

// Helper function to trim whitespace from both ends of a string
static std::string trim(const std::string &s)
{
	const std::string ws = " \t\n\r\f\v";
	size_t start = s.find_first_not_of(ws);
	if (start == std::string::npos) return "";
	size_t end = s.find_last_not_of(ws);
	return s.substr(start, end - start + 1);
}

// Helper to split a string by delimiter
static void split(const std::string &s, char delim, std::vector<std::string> &result)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		std::string trimmed = trim(item);
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
			std::string trimmed = trim(token);
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
		action->NumRequirements = 0;
		action->NumAdds = 0;
		action->NumSubtracts = 0;

		// Let's get our requirements
		reqs.clear();
		if (values[3] != "nil") {
			split(values[3], ',', reqs);
		}
		
		action->NumRequirements = (int)reqs.size();
		action->Requirements = (ObjectActions::StateIdx *) calloc(action->NumRequirements, sizeof(ObjectActions::StateIdx));
		for(i = 0; i < (int)reqs.size(); ++i)
		{
			action->Requirements[i] = find_state(reqs[i]);
		}

		// Let's get our changes
		int nAdds=0,nSubtracts=0;
		changes.clear();
		std::stringstream changesStream(values[4]);
		while (std::getline(changesStream, token, ','))
		{
			std::string trimmed = trim(token);
			if(!trimmed.empty() && trimmed[0] == '+')
			{
				++nAdds;
				changes.push_back(trimmed);
			}
			else if(!trimmed.empty() && trimmed[0] == '-')
			{
				++nSubtracts;
				changes.push_back(trimmed);
			}
			// Ignore anything else
		}

		action->NumAdds = 0;
		action->Adds = (ObjectActions::StateIdx *)calloc(nAdds, sizeof(ObjectActions::StateIdx));
		action->NumSubtracts = 0;
		action->Subtracts = (ObjectActions::StateIdx *)calloc(nSubtracts, sizeof(ObjectActions::StateIdx));
		for(i = 0; i < (int)changes.size(); ++i)
		{
			const std::string& changeStr = changes[i];
			char op = changeStr[0];
			std::string stateName = changeStr.substr(1);
			if(op == '+')
			{
				action->Adds[action->NumAdds++] = find_state(stateName);
			}
			else if(op == '-')
			{
				action->Subtracts[action->NumSubtracts++] = find_state(stateName);
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

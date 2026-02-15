#include <states/SoldierActionLoader.h>
#include <application/Globals.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include <sstream>

static ObjectActions::StateIdx find_state(char *stateName);

// Helper function to trim whitespace from both ends of a string
static std::string trim(const std::string &s)
{
	const char *ws = " \t\n\r\f\v";
	size_t start = s.find_first_not_of(ws);
	if (start == std::string::npos) return "";
	size_t end = s.find_last_not_of(ws);
	return s.substr(start, end - start + 1);
}

// Helper to split a string by delimiter
static void split(const std::string &s, char delim, Array<char> &result)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		std::string trimmed = trim(item);
		if (!trimmed.empty()) {
			result.Add(strdup(trimmed.c_str()));
		}
	}
}

void
SoldierActionLoader::Load(char *fileName, ObjectActions *actions)
{
	std::string line;
	Array<char> reqs;
	Array<char> changes;

	// We cannot load our actions if we do not already have a bunch of states
	assert(g_Globals->World.States.Soldiers.NumStates > 0);

	// Let's loop through our input file
	std::ifstream fp(fileName);
	
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
		ObjectActions::Action *action;
		if(actions->NumActions == 0)
		{
			actions->NumActions++;
			actions->Actions = (ObjectActions::Action *)calloc(1, sizeof(ObjectActions::Action));
			action = &(actions->Actions[0]);
		}
		else
		{
			actions->NumActions++;
			actions->Actions = (ObjectActions::Action *)realloc(actions->Actions, actions->NumActions*sizeof(ObjectActions::Action));
			action = &(actions->Actions[actions->NumActions-1]);
		}

		action->Name = strdup(values[0].c_str());
		action->Group = strdup(values[1].c_str());
		action->Time = atol(values[2].c_str());
		action->NumRequirements = 0;
		action->NumAdds = 0;
		action->NumSubtracts = 0;

		// Let's get our requirements
		reqs.Clear();
		if (values[3] != "nil") {
			split(values[3], ',', reqs);
		}
		
		action->NumRequirements = reqs.Count;
		action->Requirements = (ObjectActions::StateIdx *) calloc(action->NumRequirements, sizeof(ObjectActions::StateIdx));
		for(i = 0; i < reqs.Count; ++i)
		{
			action->Requirements[i] = find_state(reqs.Items[i]);
			free(reqs.Items[i]);
		}

		// Let's get our changes
		int nAdds=0,nSubtracts=0;
		changes.Clear();
		std::stringstream changesStream(values[4]);
		while (std::getline(changesStream, token, ','))
		{
			std::string trimmed = trim(token);
			if(!trimmed.empty() && trimmed[0] == '+')
			{
				++nAdds;
				changes.Add(strdup(trimmed.c_str()));
			} 
			else if(!trimmed.empty() && trimmed[0] == '-')
			{
				++nSubtracts;
				changes.Add(strdup(trimmed.c_str()));
			}
			// Ignore anything else
		}

		action->NumAdds = 0;
		action->Adds = (ObjectActions::StateIdx *)calloc(nAdds, sizeof(ObjectActions::StateIdx));
		action->NumSubtracts = 0;
		action->Subtracts = (ObjectActions::StateIdx *)calloc(nSubtracts, sizeof(ObjectActions::StateIdx));
		for(i = 0; i < changes.Count; ++i)
		{
			char *p = changes.Items[i];
			if(*p == '+')
			{
				action->Adds[action->NumAdds++] = find_state(p+1);
			}
			else if(*p == '-')
			{
				action->Subtracts[action->NumSubtracts++] = find_state(p+1);
			}
			else
			{
				assert(0);
			}
			free(p);
		}
	}
	fp.close();
}

ObjectActions::StateIdx 
find_state(char *stateName)
{
	for(int i = 0; i < g_Globals->World.States.Soldiers.NumStates; ++i)
	{
		if(strcmp(g_Globals->World.States.Soldiers.StateNames[i], stateName) == 0)
		{
			return i;
		}
	}
	assert(0);
	return 0;
}

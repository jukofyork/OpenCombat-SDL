#include <states/ObjectActions.h>
#include <assert.h>

// Checks our requirements for a given action. Returns -1 if we satisfy
// our requirements, otherwise the first action index we are missing
ObjectActions::ActionIdx 
ObjectActions::CheckRequirements(ActionIdx actionID, State *state)
{
	assert(actionID < NumActions);
	ObjectActions::Action *action = &(Actions[actionID]);
	for(size_t i = 0; i < action->Requirements.size(); ++i)
	{
		if(!state->IsSet(action->Requirements[i]))
		{
			// We do not have this state, so we need to find an action 
			// that adds this state
			for(int j = 0; j < NumActions; ++j)
			{
				for(size_t k = 0; k < Actions[j].Adds.size(); ++k)
				{
					if(action->Requirements[i] == Actions[j].Adds[k])
					{
						// Let's perform this action!
						return j;
					}
				}
			}
			
		}
	}
	return -1;
}

void 
ObjectActions::UpdateState(ActionIdx actionID, State *state)
{
	assert(actionID < NumActions);

	for(size_t i = 0; i < Actions[actionID].Adds.size(); ++i)
	{
		state->Set(Actions[actionID].Adds[i]);
	}

	for(size_t i = 0; i < Actions[actionID].Subtracts.size(); ++i)
	{
		state->UnSet(Actions[actionID].Subtracts[i]);
	}
}

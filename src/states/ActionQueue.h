#pragma once

#include <vector>
#include <assert.h>

// This class defines a queue of actions, where each action is defined
// by an integer index
class ActionQueue
{
public:
	ActionQueue()
	{
		_data.resize(1);
		_firstItem = 0;
		_lastItem = 0;
		_nItems = 0;
	}

	~ActionQueue()
	{
	}

	// Enqueues a new action idx
	void Enqueue(int actionIdx)
	{
		// Let's see if we have enough space to add another index
		if(_firstItem == _lastItem && _nItems > 0)
		{
			// We need more space - double the capacity
			int oldLength = (int)_data.size();
			int newLength = oldLength << 1;
			int toMove = oldLength - _lastItem - 1;
			_data.resize(newLength);
			if(toMove > 0)
			{
				// Move wrapped elements to end of new buffer
				for(int i = 0; i < toMove; ++i)
				{
					_data[newLength - toMove + i] = _data[_lastItem + 1 + i];
				}
			}
			_firstItem = newLength - toMove - 1;
		}
		_data[_firstItem] = actionIdx;
		--_firstItem;
		if(_firstItem < 0)
		{
			_firstItem += (int)_data.size();
		}
		++_nItems;
	}

	// Dequeues the first element
	bool Dequeue(int *rv)
	{
		if(_nItems > 0)
		{
			*rv = _data[_lastItem];
			--_lastItem;
			if(_lastItem < 0)
			{
				_lastItem += (int)_data.size();
			}
			--_nItems;
			return true;
		}
		else
		{
			return false;
		}
	}

	// Peeks at the first element
	bool Peek(int *rv)
	{
		if(_nItems < 0)
		{
			*rv = _data[_lastItem];
			return true;
		}
		else
		{
			return false;
		}
	}

	// Perform a self test of this module
	static bool SelfTest()
	{
		ActionQueue q1;

		for(int i = 0; i < 10; ++i)
		{
			q1.Enqueue(i);
		}

		int j = 0;
		int rv = -1;
		while(q1.Dequeue(&rv))
		{
			assert(rv == (j++));
		}
		assert(j == 10);

		// Now do it again
		assert(q1._nItems == 0);
		for(int i = 0; i < 17; ++i)
		{
			q1.Enqueue(i);
		}

		j = 0;
		rv = -1;
		while(q1.Dequeue(&rv))
		{
			assert(rv == (j++));
		}
		assert(j == 17);
		assert(q1._nItems == 0);
		return true;
	}

protected:
	std::vector<int> _data;
	int _firstItem;
	int _lastItem;
	int _nItems;
};

#include <states/State.h>
#include <cstdint>

State::State()
{
	_bits = 0;
}

bool
State::IsSet(unsigned int state)
{
uint64_t flag = 1;
	flag <<= state;
	return (_bits&flag) != 0;
}

void
State::Set(unsigned int state)
{
	uint64_t flag = 1;
	flag <<= state;
	_bits |= flag;
}

void
State::UnSet(unsigned int state)
{
	uint64_t flag = 1;
	flag <<= state;
	flag = ~flag;
	_bits &= flag;
}

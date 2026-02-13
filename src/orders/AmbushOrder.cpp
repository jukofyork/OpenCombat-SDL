#include "./AmbushOrder.h"

AmbushOrder::AmbushOrder(Direction dir)
{
	_orderType = Orders::Ambush;
	Heading = dir;
}

AmbushOrder::~AmbushOrder(void)
{
}

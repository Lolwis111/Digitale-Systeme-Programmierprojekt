#include "node.h"

node::node()
{
	left = NULL;
	right = NULL;
	_value = 0;
}

int node::getValue()
{
	return _value;
}

void node::setValue(int value)
{
	_value = value;
}
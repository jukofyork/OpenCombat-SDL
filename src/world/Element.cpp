#include "./Element.h"

Element::Element(void)
{
	Index = 0;
	Height = 0;
	BlocksHeight = false;
	Passable = false;
	for (int i = 0; i < 4; ++i) {
		Cover[i] = 0;
		Hindrance[i] = 0;
	}
	for (int i = 0; i < 5; ++i) {
		Protection[i] = 0;
	}
	for (int i = 0; i < 3; ++i) {
		Movement[i] = 0.0f;
	}
}

Element::~Element(void)
{
}

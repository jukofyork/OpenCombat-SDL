#pragma once

class CursorInterface
{
public:
	enum CursorType {
		// Order markers
		MarkBlue=0,
		MarkPurple,
		MarkRed,
		MarkYellow,
		MarkOrange,
		MarkBrown,
		MarkGreen,
		MarkGrey,
		// Crosshairs for hit chance (filled = over target, empty = no target)
		CrosshairsBlack,
		CrosshairsRed,
		CrosshairsYellow,
		CrosshairsGreen,
		CrosshairsEmptyBlack,
		CrosshairsEmptyRed,
		CrosshairsEmptyYellow,
		CrosshairsEmptyGreen,
		Regular,
		NumCursorTypes
	};

	virtual void ShowCursor(bool bShow, CursorType type) = 0;
};

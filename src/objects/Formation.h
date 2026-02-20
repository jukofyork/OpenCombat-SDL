#pragma once

#include <misc/Structs.h>

// Formation Types - Based on US Infantry Doctrine:
// 
// Column: Standard tactical formation where units follow one behind another
// with moderate spacing. Used for movement along routes where contact is
// possible but not expected. Allows quick transition to Line formation.
//
// File: Tight formation with units walking directly behind each other 
// ("follow the leader"). Used in dense terrain, low visibility, or when
// navigating obstacles. Maintains physical contact between team members.
// Key difference from Column is the tighter spacing and single-file nature.
//
// Line: Units positioned side-by-side perpendicular to the direction of
// movement. Used when presenting maximum firepower to the front, crossing
// danger areas, or assaulting. Leader typically centered or at one end.
//
// Reference: The principle difference between column and file is dispersion
// at the fire team level. File is much more compact with individuals closer
// to maintain contact in complex terrain. At platoon level, column allows
// sub-units to choose their own formation while maintaining vertical alignment.
//
// Source: alertjohn117, /r/WarCollege - "the principle difference between
// column and file is dispersion at the fire team level..."

class Formation
{
public:
	enum Type
	{
		Column,
		File,
		Line,
		/*Scatter,*/
		NumFormations
	};

	static void GetFormationPosition(Type formationType, int formationIdx, float formationSpread, int *x, int *y);
	static void GetFormationPosition(Type formationType, int formationIdx, float formationSpread, Point *src, Direction heading, int *x, int *y);
};

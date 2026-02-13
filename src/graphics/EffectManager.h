#pragma once

#include <misc/Array.h>

class Effect;
class TGA;
struct EffectAttributes;

class EffectManager
{
public:
	EffectManager(void);
	virtual ~EffectManager(void);

	void LoadEffects(char *fileName);
	Effect *GetEffect(char *effectName);
	
	void GetFiles(EffectAttributes *attr, const char *searchStr);

protected:
	// The array of effects
	Array<Effect> _effects;

	// The array of source images for these widgets
	Array<TGA> _sourceImages;
};

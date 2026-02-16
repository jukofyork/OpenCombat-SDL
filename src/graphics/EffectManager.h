#pragma once

#include <filesystem>
#include <string>
#include <misc/Array.h>

class Effect;
class TGA;
struct EffectAttributes;

class EffectManager
{
public:
	EffectManager(void);
	virtual ~EffectManager(void);

	void LoadEffects(const std::filesystem::path& fileName);
	Effect *GetEffect(const std::string &effectName);
	
	void GetFiles(EffectAttributes *attr, const char *searchStr);

protected:
	// The array of effects
	Array<Effect> _effects;

	// The array of source images for these widgets
	Array<TGA> _sourceImages;
};

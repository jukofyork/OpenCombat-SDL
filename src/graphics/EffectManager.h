#pragma once

#include <filesystem>
#include <string>
#include <vector>

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
	
	void GetFiles(EffectAttributes *attr, const std::string& searchStr);

protected:
	// The array of effects
	std::vector<Effect*> _effects;

	// The array of source images for these widgets
	std::vector<TGA*> _sourceImages;
};

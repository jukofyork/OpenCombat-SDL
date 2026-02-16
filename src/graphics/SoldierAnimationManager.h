#pragma once
#include <graphics/AnimationManager.h>
#include <misc/Array.h>

class SoldierAnimationManager : AnimationManager
{
public:
	SoldierAnimationManager(void);
	virtual ~SoldierAnimationManager(void);

	virtual void LoadAnimations(const std::filesystem::path& fileName);

};

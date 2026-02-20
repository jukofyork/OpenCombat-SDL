#pragma once

#include <array>
#include <string>
#include <misc/Structs.h>

class Weapon
{
public:
	Weapon(void);
	virtual ~Weapon(void);

	// Checks to see if we can fire this weapon
	bool CanFire();

	// Checks to see if the clip is empty
	bool IsEmpty();

	// Fires this weapon
	void Fire();

	// Reloads this weapon
	void Reload();

	// Checks to see if we are reloading the weapon
	bool IsReloading();

	// Simulates this weapon. Updates firing times and stuff like that
	void Simulate(long dt);

	inline const std::string& GetName() const { return _name; }
	inline int GetCurrentRounds() { return _numRounds; }
	inline const std::string& GetIconName() const { return _iconName; }
	inline int GetRoundsPerClip() { return _totalRounds; }
	inline int GetRoundsPerBurst() { return _roundsPerBurst; }

	// Set and get effects
	void SetEffect(const std::string &effectName);
	inline const std::string& GetEffect(Direction heading) const { return _effects[static_cast<size_t>(heading)]; }

	// Does this weapon cause a big boom?
	inline bool IsGroundShaker() { return _bGroundShaker; }
	inline void SetGroundShaker(bool b) { _bGroundShaker = b; }

protected:
	friend class WeaponManager;

	enum State {
		Reloading, Firing, Reloaded, NumStates
	};

	// The name of this weapon
	std::string _name;

	// The sound for this weapon
	std::string _sound;

	// The animation for this weapon
	std::string _animation;

	// The time it takes to fire the weapon
	int _timeToFire;

	// The number of rounds in the weapon
	int _numRounds;

	// The number of rounds this weapon can hold
	int _totalRounds;

	// The number of rounds in one burst
	int _roundsPerBurst;

	// The name of the icon for this weapon
	std::string _iconName;

	// The counter for our firing times
	long _counter;

	// Our current firing state
	State _state;

	// The reload time for a chamber
	int _reloadTimeChamber;

	// The reload time for a clip
	int _reloadTimeClip;
	
	// Effects
	std::array<std::string, static_cast<size_t>(Direction::NumDirections)> _effects;

	// Goes big boom
	bool _bGroundShaker;
};

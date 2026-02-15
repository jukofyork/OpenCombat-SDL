#pragma once

#include <misc/Array.h>

class TGA;
class Vehicle;
struct VehicleAttributes;
class WeaponManager;

class VehicleManager
{
public:
	VehicleManager(void);
	virtual ~VehicleManager(void);

	// Loads a group of widgets into this widget manager
	void Load(char *fileName);

	// Retrieves a Vehicle from this manager
	Vehicle *GetVehicle(const char *vehicleName);

protected:
	// The array of widgets we are managing
	Array<VehicleAttributes> _vehicles;
};

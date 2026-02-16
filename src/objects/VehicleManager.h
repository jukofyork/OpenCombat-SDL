#pragma once

#include <filesystem>
#include <vector>

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
	void Load(const std::filesystem::path& fileName);

	// Retrieves a Vehicle from this manager
	Vehicle *GetVehicle(const std::string& vehicleName);

protected:
	// The array of vehicles we are managing
	std::vector<VehicleAttributes*> _vehicles;
};

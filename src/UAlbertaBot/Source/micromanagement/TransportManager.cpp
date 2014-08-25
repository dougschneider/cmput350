#include "Common.h"
#include "TransportManager.h"
#include "SpecificUnitManager.h"

TransportManager::TransportManager()  { }

void TransportManager::executeMicro(const UnitVector & targets) 
{
	const UnitVector & transportUnits = getUnits();

	if (transportUnits.empty())
	{
		return;
	}

	BOOST_FOREACH(BWAPI::Unit * transportUnit, transportUnits)
	{
		if(unitManagers.find(transportUnit) == unitManagers.end())
			unitManagers[transportUnit] = SpecificUnitManager::getUnitManager(transportUnit);

		SpecificUnitManager * manager = unitManagers[transportUnit];

		Action action = manager->getBestAction(targets, squad);
		manager->takeAction(action);
	}
}

void TransportManager::setSquad(const UnitVector & units)
{
	squad = UnitVector(units);
}

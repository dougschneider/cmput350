#include "SupportManager.h"
#include "SpecificUnitManager.h"

bool SupportManager::isSupportUnit(BWAPI::Unit * unit)
{
	return SpecificUnitManager::isSupportUnit(unit);
}

void SupportManager::executeMicro(const UnitVector & targets)
{
    static int cleanCounter = 0;
	const UnitVector & supportUnits = getUnits();

	BOOST_FOREACH(BWAPI::Unit * supportUnit, supportUnits)
	{
		if(unitManagers.find(supportUnit) == unitManagers.end())
            unitManagers[supportUnit] = SpecificUnitManager::getUnitManager(supportUnit);

		SpecificUnitManager* manager = unitManagers[supportUnit];

		Action action = manager->getBestAction(targets, allies);
		manager->takeAction(action);
	}

    ++cleanCounter;

    // clear the unit manager every once in a while
    if(cleanCounter >= 200)
	{
        std::map<BWAPI::Unit *, SpecificUnitManager*>::iterator start = unitManagers.begin(),
                                                                end = unitManagers.end();
        for(; start != end; ++start)
            delete start->second;
        unitManagers.clear();
	}
}

void SupportManager::setAllies(const UnitVector & units)
{
	allies = UnitVector(units);
}
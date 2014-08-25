#include "SpecificUnitManager.h"
#include "MedicManager.h"
#include "ComsatManager.h"

bool SpecificUnitManager::isSupportUnit(BWAPI::Unit * unit)
{
	if(unit->getType() == BWAPI::UnitTypes::Terran_Medic ||
		    unit->getType() == BWAPI::UnitTypes::Terran_Comsat_Station)
	{
		return true;
	}

	// for now we assume only units with a specified manager are support units
	return false;
}

SpecificUnitManager* SpecificUnitManager::getUnitManager(BWAPI::Unit * unit)
{
	if(unit->getType() == BWAPI::UnitTypes::Terran_Medic)
		return new MedicManager(unit);
	else if(unit->getType() == BWAPI::UnitTypes::Terran_Comsat_Station)
        return new ComsatManager(unit);
	else
		return new SpecificUnitManager(unit);
}

Action SpecificUnitManager::getBestAction(const UnitVector & enemies, const UnitVector & allies)
{	
	// return the best ally if there is one
	BOOST_FOREACH(BWAPI::Unit * ally, allies)
	{
		if(ally != unit && canSupportUnit(ally))
			return Action(BWAPI::Orders::Follow, ally);
	}
	// otherwise return the best enemy
	BOOST_FOREACH(BWAPI::Unit * enemy, allies)
	{
		if(canAntiSupportUnit(enemy))
			return Action(BWAPI::Orders::AttackUnit, enemy);
	}

	return Action(BWAPI::Orders::None, NULL);
}

void SpecificUnitManager::takeAction(Action action)
{
    if(action.first == currentAction.first &&
            action.second == currentAction.second)
	{
        return;
	}
    currentAction = action;

	BWAPI::Unit * target = action.second;
	if(action.first == BWAPI::Orders::Follow)
		unit->follow(target);
	else if(action.first == BWAPI::Orders::AttackUnit)
		unit->attack(target);
}

bool SpecificUnitManager::canSupportUnit(BWAPI::Unit * target)
{
	// by default we assume we can aid units
	return true;
}

bool SpecificUnitManager::canAntiSupportUnit(BWAPI::Unit * target)
{
	// by default we assume we won't attack since this is a support unit
	return false;
}
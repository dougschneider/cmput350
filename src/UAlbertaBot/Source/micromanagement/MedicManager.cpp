#include "MedicManager.h"

MedicManager::MedicManager(BWAPI::Unit * unit):
	SpecificUnitManager(unit)
{
}

Action MedicManager::getBestAction(const UnitVector & enemies, const UnitVector & allies)
{
	BWAPI::Unit * best = NULL;
	int minHealth = 10000;
	BOOST_FOREACH(BWAPI::Unit * ally, allies)
	{
		int hp = ally->getHitPoints();
		if(canSupportUnit(ally) &&
				(hp < minHealth || best == NULL))
		{
			minHealth = hp;
			best = ally;
		}
	}
	if(best != NULL && best->getHitPoints() < best->getType().maxHitPoints())
		return Action(BWAPI::Orders::MedicHeal1, best);
	return Action(BWAPI::Orders::Follow, best);
}

void MedicManager::takeAction(Action action)
{
    if(action.first == currentAction.first &&
            action.second == currentAction.second)
	{
        return;
	}
    currentAction = action;

	BWAPI::Unit * target = action.second;
	if(action.first == BWAPI::Orders::MedicHeal1)
	{
		unit->useTech(BWAPI::TechTypes::Healing, target);
	}
	else
	{
		unit->follow(target);
	}
}

bool MedicManager::canSupportUnit(BWAPI::Unit * target)
{
	// we can support a unit if it's missing health and it's not ourselves
	if(target->getHitPoints() < target->getType().maxHitPoints() &&
			target != unit)
	{
		return true;
	}
	return false;
}

bool MedicManager::canAntiSupportUnit(BWAPI::Unit * target)
{
	return false;
}

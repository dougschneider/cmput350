#include "ComsatManager.h"
#include "../MapGrid.h"
#include "../InformationManager.h"

BWAPI::Unit* ComsatManager::currentlyScanningUnit = NULL;

ComsatManager::ComsatManager(BWAPI::Unit * unit):
	SpecificUnitManager(unit)
{
}

Action ComsatManager::getBestAction(const UnitVector & enemies, const UnitVector & allies)
{
    if(ComsatManager::currentlyScanningUnit != NULL && ComsatManager::currentlyScanningUnit != unit)
        return Action(NULL, NULL);
    ComsatManager::currentlyScanningUnit = NULL;

    std::map<BWAPI::Unit*, UnitInfo> unitInfo = InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy());
    std::map<BWAPI::Unit *, UnitInfo>::const_iterator it = unitInfo.begin(), end = unitInfo.end();
    for(; it != end; ++it)
	{
        BWAPI::Unit* enemy = it->first;
        if(enemy->isVisible() && !enemy->isDetected())
		{
            UnitVector nearby;
            MapGrid::Instance().GetUnits(nearby, enemy->getPosition(), 800, true, false);
            if(nearby.size() > 0)
			{
                ComsatManager::currentlyScanningUnit = unit;
                return Action(BWAPI::Orders::CastScannerSweep, enemy);
			}
		}
    }
    return Action(NULL, NULL);
}

void ComsatManager::takeAction(Action action)
{
    if(action.first == NULL)
	{
        return;
    }
	else if(action.first == BWAPI::Orders::CastScannerSweep)
	{
        unit->useTech(BWAPI::TechTypes::Scanner_Sweep, action.second);
	}
	else
	{
        assert(false);
	}
}

bool ComsatManager::canSupportUnit(BWAPI::Unit * target)
{
    return true;
}

bool ComsatManager::canAntiSupportUnit(BWAPI::Unit * target)
{
    return false;
}
#pragma once

#include <Common.h>

typedef std::pair<BWAPI::Order, BWAPI::Unit *> Action;

class SpecificUnitManager
{
public:

	SpecificUnitManager(BWAPI::Unit * unit) :
		unit(unit)
	{
	}

	virtual ~SpecificUnitManager() {}

	virtual Action getBestAction(const UnitVector & enemies, const UnitVector & allies);
	virtual void takeAction(Action action);

	static bool isSupportUnit(BWAPI::Unit * unit);
	static SpecificUnitManager* getUnitManager(BWAPI::Unit * unit);

protected:

	BWAPI::Unit * unit;
    Action currentAction;
	
	virtual bool canSupportUnit(BWAPI::Unit * target);
	virtual bool canAntiSupportUnit(BWAPI::Unit * target);
};


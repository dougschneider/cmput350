#pragma once

#include <Common.h>
#include "SpecificUnitManager.h"

class SpecificUnitManager;

class ComsatManager : public SpecificUnitManager
{

    static BWAPI::Unit * currentlyScanningUnit;
public:

	ComsatManager(BWAPI::Unit * unit);

	virtual Action getBestAction(const UnitVector & enemies, const UnitVector & allies);
	virtual void takeAction(Action action);

private:

	virtual bool canSupportUnit(BWAPI::Unit * target);
	virtual bool canAntiSupportUnit(BWAPI::Unit * target);
};


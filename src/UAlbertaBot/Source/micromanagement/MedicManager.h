#pragma once;

#include <Common.h>
#include "SpecificUnitManager.h"

class SpecificUnitManager;

class MedicManager : public SpecificUnitManager
{
public:

	MedicManager(BWAPI::Unit * unit);

	virtual Action getBestAction(const UnitVector & enemies, const UnitVector & allies);
	virtual void takeAction(Action action);

private:

	virtual bool canSupportUnit(BWAPI::Unit * target);
	virtual bool canAntiSupportUnit(BWAPI::Unit * target);
};
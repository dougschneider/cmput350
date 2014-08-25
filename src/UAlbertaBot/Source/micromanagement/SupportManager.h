#pragma once

#include <Common.h>
#include "MicroManager.h"

class MicroManager;
class SpecificUnitManager;

class SupportManager : public MicroManager
{
public:
	~SupportManager(void) {}

	static bool isSupportUnit(BWAPI::Unit * unit);

	void executeMicro(const UnitVector & targets);

	void setAllies(const UnitVector & units);

private:
	UnitVector allies;
    std::map<BWAPI::Unit *, SpecificUnitManager *> unitManagers;
};


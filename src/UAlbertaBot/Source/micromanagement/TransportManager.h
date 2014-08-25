#pragma once;

#include <Common.h>
#include "MicroManager.h"

class MicroManager;
class SpecificUnitManager;

class TransportManager : public MicroManager
{
public:
	TransportManager();
	~TransportManager() {}

	void executeMicro(const UnitVector & targets);
	void setSquad(const UnitVector & units);

private:
	UnitVector squad;
	std::map<BWAPI::Unit *, SpecificUnitManager *> unitManagers;
};

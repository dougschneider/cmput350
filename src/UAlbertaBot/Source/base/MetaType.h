#pragma once

#include "Common.h"

struct MetaType 
{
	enum type_enum { Unit, Tech, Upgrade, Command, Default };
	type_enum type;

	BWAPI::UnitCommandType commandType;
	BWAPI::UnitType unitType;
	BWAPI::TechType techType;
	BWAPI::UpgradeType upgradeType;

	MetaType () : type(MetaType::Default) {}
	MetaType (BWAPI::UnitType t) :        unitType(t),    type(MetaType::Unit) {}
	MetaType (BWAPI::TechType t) :        techType(t),    type(MetaType::Tech) {}
	MetaType (BWAPI::UpgradeType t) :     upgradeType(t), type(MetaType::Upgrade) {}
	MetaType (BWAPI::UnitCommandType t) : commandType(t), type(MetaType::Command) {}

	bool isUnit()		{ return type == Unit; }
	bool isTech()		{ return type == Tech; }
	bool isUpgrade()	{ return type == Upgrade; }
	bool isCommand()	{ return type == Command; }
	bool isBuilding()	{ return type == Unit && unitType.isBuilding(); }
	bool isRefinery()	{ return isBuilding() && unitType.isRefinery(); }

	int supplyRequired()
	{
		if (isUnit())
		{
			return unitType.supplyRequired();
		}
		else
		{
			return 0;
		}
	}

	int mineralPrice()
	{
		return isUnit() ? unitType.mineralPrice() : (isTech() ? techType.mineralPrice() : upgradeType.mineralPrice());
	}

	int gasPrice()
	{
		return isUnit() ? unitType.gasPrice() : (isTech() ? techType.gasPrice() : upgradeType.gasPrice());
	}

	BWAPI::UnitType whatBuilds()
	{
		return isUnit() ? unitType.whatBuilds().first : (isTech() ? techType.whatResearches() : upgradeType.whatUpgrades());
	}

    // returns true if the whatBuilds() building returned requires an
    // addon to produce this unit
    bool requiresAddon()
	{
        if(!isUnit())
		{
            return false;
		}

        std::map<BWAPI::UnitType, int>::const_iterator start = unitType.requiredUnits().begin(), end = unitType.requiredUnits().end();
        for(; start != end; ++start)
		{
            BWAPI::UnitType type = start->first;
            if(type.getID() == BWAPI::UnitTypes::Terran_Control_Tower ||
                    type.getID() == BWAPI::UnitTypes::Terran_Machine_Shop)
			{
                return true;
			}
		}

        return false;
	}

	std::string getName()
	{
		if (isUnit())
		{
			return unitType.getName();
		}
		else if (isTech())
		{
			return techType.getName();
		}
		else if (isUpgrade())
		{
			return upgradeType.getName();
		}
		else if (isCommand())
		{
			return commandType.getName();
		}
		else
		{
			assert(false);
			return "LOL";	
		}
	}
};

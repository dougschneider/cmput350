#include "Common.h"
#include "StrategyManager.h"

// constructor
StrategyManager::StrategyManager() 
	: firstAttackSent(false)
	, currentStrategy(0)
	, selfRace(BWAPI::Broodwar->self()->getRace())
	, enemyRace(BWAPI::Broodwar->enemy()->getRace())
{
	addStrategies();
	setStrategy();
}

// get an instance of this
StrategyManager & StrategyManager::Instance() 
{
	static StrategyManager instance;
	return instance;
}

void StrategyManager::addStrategies() 
{
	protossOpeningBook = std::vector<std::string>(NumProtossStrategies);
	terranOpeningBook  = std::vector<std::string>(NumTerranStrategies);
	zergOpeningBook    = std::vector<std::string>(NumZergStrategies);

	//protossOpeningBook[ProtossZealotRush]	= "0 0 0 0 1 0 0 3 0 0 3 0 1 3 0 4 4 4 4 4 1 0 4 4 4";
    protossOpeningBook[ProtossZealotRush]	= "0 0 0 0 1 0 3 3 0 0 4 1 4 4 0 4 4 0 1 4 3 0 1 0 4 0 4 4 4 4 1 0 4 4 4";
	//protossOpeningBook[ProtossZealotRush]	= "0";
	//protossOpeningBook[ProtossDarkTemplar]	= "0 0 0 0 1 3 0 7 5 0 0 12 3 13 0 22 22 22 22 0 1 0";
    protossOpeningBook[ProtossDarkTemplar]	=     "0 0 0 0 1 0 3 0 7 0 5 0 12 0 13 3 22 22 1 22 22 0 1 0";
	protossOpeningBook[ProtossDragoons]		= "0 0 0 0 1 0 0 3 0 7 0 0 5 0 0 3 8 6 1 6 6 0 3 1 0 6 6 6";

	terranOpeningBook[TerranMarineRush]				= "1 1 1 1 4 1 1 10 10 1 1 11 11 11 11 11 11 11 4 1 1 10 11 11 1 11 4 10 5 1 ";
	terranOpeningBook[TerranMarinesAndMedics]	= "1 1 1 1 1 4 1 10  1    1 10 1 11  4  1  1 11 11 11 16 10 1  5  4 11 11 11 11 11 11 11 2";
																						//"1 1 1 1 4 1 1 10 11 11 1 10 1  4 11 10  1 11 11 11 11 4  1 11 11 11 16";
	terranOpeningBook[TerranSiegePush]	= "1 1 1 1 1 4 1 10 1 1 10 1 11 4 1 1 11 11 11 16 5 10 4 1 11 11 11 11 11 11 11 2";
	terranOpeningBook[TerranNukes]	= "1 1 1 1 1 4 1 10 1 1 10 1 15 11 4 1 1 11 11 11 16 5 10 4 1 15 11 11 11 11 15 11 11 11 11 2";

	zergOpeningBook[ZergZerglingRush]					= "0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 0 0 0 1 6";

	if (selfRace == BWAPI::Races::Protoss)
	{
		results = std::vector<IntPair>(NumProtossStrategies);

		if (enemyRace == BWAPI::Races::Protoss)
		{
			usableStrategies.push_back(ProtossZealotRush);
			usableStrategies.push_back(ProtossDarkTemplar);
			usableStrategies.push_back(ProtossDragoons);
		}
		else if (enemyRace == BWAPI::Races::Terran)
		{
			usableStrategies.push_back(ProtossZealotRush);
			usableStrategies.push_back(ProtossDarkTemplar);
			usableStrategies.push_back(ProtossDragoons);
		}
		else if (enemyRace == BWAPI::Races::Zerg)
		{
			usableStrategies.push_back(ProtossZealotRush);
			usableStrategies.push_back(ProtossDragoons);
		}
		else
		{
			BWAPI::Broodwar->printf("Enemy Race Unknown");
			usableStrategies.push_back(ProtossZealotRush);
			usableStrategies.push_back(ProtossDragoons);
		}
	}
	else if (selfRace == BWAPI::Races::Terran)
	{
		results = std::vector<IntPair>(NumTerranStrategies);
		usableStrategies.push_back(TerranMarineRush);
		usableStrategies.push_back(TerranMarinesAndMedics);
		//usableStrategies.push_back(TerranSiegePush);
		usableStrategies.push_back(TerranNukes);
	}
	else if (selfRace == BWAPI::Races::Zerg)
	{
		results = std::vector<IntPair>(NumZergStrategies);
		usableStrategies.push_back(ZergZerglingRush);
	}

	if (Options::Modules::USING_STRATEGY_IO)
	{
		readResults();
	}
}

void StrategyManager::readResults()
{
	// read in the name of the read and write directories from settings file
	struct stat buf;

	// if the file doesn't exist something is wrong so just set them to default settings
	if (stat(Options::FileIO::FILE_SETTINGS, &buf) == -1)
	{
		readDir = "bwapi-data\\read\\";
		writeDir = "bwapi-data\\write\\";
	}
	else
	{
		std::ifstream f_in(Options::FileIO::FILE_SETTINGS);
		getline(f_in, readDir);
		getline(f_in, writeDir);
		f_in.close();
	}

	// the file corresponding to the enemy's previous results
	std::string readFile = readDir + BWAPI::Broodwar->enemy()->getName() + ".txt";

	// if the file doesn't exist, set the results to zeros
	if (stat(readFile.c_str(), &buf) == -1)
	{
		std::fill(results.begin(), results.end(), IntPair(0,0));
	}
	// otherwise read in the results
	else
	{
		std::ifstream f_in(readFile.c_str());
		std::string line;
		if (selfRace == BWAPI::Races::Protoss)
		{
			getline(f_in, line);
			results[ProtossZealotRush].first = atoi(line.c_str());
			getline(f_in, line);
			results[ProtossZealotRush].second = atoi(line.c_str());
			getline(f_in, line);
			results[ProtossDarkTemplar].first = atoi(line.c_str());
			getline(f_in, line);
			results[ProtossDarkTemplar].second = atoi(line.c_str());
			getline(f_in, line);
			results[ProtossDragoons].first = atoi(line.c_str());
			getline(f_in, line);
			results[ProtossDragoons].second = atoi(line.c_str());
		} else if (selfRace == BWAPI::Races::Terran)
		{
			//Reads all the strategies 
			//(makes it easier so you only need to change one line of code rather then several)
			std::vector<IntPair>::iterator strategy = results.begin();
			for (; strategy != results.end(); ++strategy)
			{
				getline(f_in, line);
				strategy->first = atoi(line.c_str());
				getline(f_in, line);
				strategy->second = atoi(line.c_str());
			}
		}
		f_in.close();
	}

	BWAPI::Broodwar->printf("Results (%s): (%d %d) (%d %d) (%d %d)", BWAPI::Broodwar->enemy()->getName().c_str(), 
		results[0].first, results[0].second, results[1].first, results[1].second, results[2].first, results[2].second);
}

void StrategyManager::writeResults()
{
	std::string writeFile = writeDir + BWAPI::Broodwar->enemy()->getName() + ".txt";
	std::ofstream f_out(writeFile.c_str());

	if (selfRace == BWAPI::Races::Protoss)
	{
		f_out << results[ProtossZealotRush].first   << "\n";
		f_out << results[ProtossZealotRush].second  << "\n";
		f_out << results[ProtossDarkTemplar].first  << "\n";
		f_out << results[ProtossDarkTemplar].second << "\n";
		f_out << results[ProtossDragoons].first     << "\n";
		f_out << results[ProtossDragoons].second    << "\n";
	} else if (selfRace == BWAPI::Races::Terran)
	{
		//Writes all the strategies 
		//(makes it easier so you only need to change one line of code rather then several)
		std::vector<IntPair>::iterator strategy = results.begin();
		for (; strategy != results.end(); ++strategy)
		{
			f_out << strategy->first   << "\n";
			f_out << strategy->second  << "\n";
		}
	}

	f_out.close();
}

void StrategyManager::setStrategy()
{
	// if we are using file io to determine strategy, do so
	if (Options::Modules::USING_STRATEGY_IO)
	{
		double bestUCB = -1;
		int bestStrategyIndex = 0;

		// UCB requires us to try everything once before using the formula
		for (size_t strategyIndex(0); strategyIndex<usableStrategies.size(); ++strategyIndex)
		{
			int sum = results[usableStrategies[strategyIndex]].first + results[usableStrategies[strategyIndex]].second;

			if (sum == 0)
			{
				currentStrategy = usableStrategies[strategyIndex];
				return;
			}
		}

		// if we have tried everything once, set the maximizing ucb value
		for (size_t strategyIndex(0); strategyIndex<usableStrategies.size(); ++strategyIndex)
		{
			double ucb = getUCBValue(usableStrategies[strategyIndex]);

			if (ucb > bestUCB)
			{
				bestUCB = ucb;
				bestStrategyIndex = strategyIndex;
			}
		}
		
		currentStrategy = usableStrategies[bestStrategyIndex];
	}
	else // otherwise return a random strategy
	{
		if(selfRace == BWAPI::Races::Protoss)
		{
			currentStrategy = ProtossZealotRush;
		}
		else if(selfRace == BWAPI::Races::Terran)
		{
			currentStrategy = TerranMarineRush;
		}
		else if(selfRace == BWAPI::Races::Zerg)
		{
			currentStrategy = ZergZerglingRush;
		}
		else
		{
			assert(false);
		}
	}
}

void StrategyManager::onEnd(const bool isWinner)
{
	// write the win/loss data to file if we're using IO
	if (Options::Modules::USING_STRATEGY_IO)
	{
		// if the game ended before the tournament time limit
		if (BWAPI::Broodwar->getFrameCount() < Options::Tournament::GAME_END_FRAME)
		{
			if (isWinner)
			{
				results[getCurrentStrategy()].first = results[getCurrentStrategy()].first + 1;
			}
			else
			{
				results[getCurrentStrategy()].second = results[getCurrentStrategy()].second + 1;
			}
		}
		// otherwise game timed out so use in-game score
		else
		{
			if (getScore(BWAPI::Broodwar->self()) > getScore(BWAPI::Broodwar->enemy()))
			{
				results[getCurrentStrategy()].first = results[getCurrentStrategy()].first + 1;
			}
			else
			{
				results[getCurrentStrategy()].second = results[getCurrentStrategy()].second + 1;
			}
		}
		
		writeResults();
	}
}

const double StrategyManager::getUCBValue(const size_t & strategy) const
{
	double totalTrials(0);
	for (size_t s(0); s<usableStrategies.size(); ++s)
	{
		totalTrials += results[usableStrategies[s]].first + results[usableStrategies[s]].second;
	}

	double C		= 0.7;
	double wins		= results[strategy].first;
	double trials	= results[strategy].first + results[strategy].second;

	double ucb = (wins / trials) + C * sqrt(std::log(totalTrials) / trials);

	return ucb;
}

const int StrategyManager::getScore(BWAPI::Player * player) const
{
	return player->getBuildingScore() + player->getKillScore() + player->getRazingScore() + player->getUnitScore();
}

const std::string StrategyManager::getOpeningBook() const
{
	if (selfRace == BWAPI::Races::Protoss)
	{
		return protossOpeningBook[currentStrategy];
	}
	else if (selfRace == BWAPI::Races::Terran)
	{
		return terranOpeningBook[currentStrategy];
	}
	else if (selfRace == BWAPI::Races::Zerg)
	{
		return zergOpeningBook[currentStrategy];
	} 
	else
	{
		assert(false); // Cyanide
	}

	// something wrong, return the terran one
	return terranOpeningBook[currentStrategy];
}

// when do we want to defend with our workers?
// this function can only be called if we have no fighters to defend with
const int StrategyManager::defendWithWorkers()
{
	if (!Options::Micro::WORKER_DEFENSE)
	{
		return false;
	}

	// our home nexus position
	BWAPI::Position homePosition = BWTA::getStartLocation(BWAPI::Broodwar->self())->getPosition();;

	// enemy units near our workers
	int enemyUnitsNearWorkers = 0;

    BWTA::Region* myRegion = BWTA::getRegion(homePosition);
	// fill the set with the types of units we're concerned about
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		// if it's a zergling or a worker we want to defend
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
		{
			if (BWTA::getRegion(BWAPI::TilePosition(unit->getPosition())) == myRegion)
			{
				enemyUnitsNearWorkers++;
			}
		}
	}

	// if there are enemy units near our workers, we want to defend
	return enemyUnitsNearWorkers;
}

// called by combat commander to determine whether or not to send an attack force
// freeUnits are the units available to do this attack
const bool StrategyManager::doAttack (const std::set<BWAPI::Unit *> & freeUnits)
{
	int ourForceSize = (int) freeUnits.size ();

	int numUnitsNeededForAttack = 1;

	bool doAttack;
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
	{
		doAttack = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) >= 1
					|| ourForceSize >= numUnitsNeededForAttack;
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran)
	{
		doAttack = ourForceSize >= numUnitsNeededForAttack;
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg)
	{
		doAttack = ourForceSize >= numUnitsNeededForAttack;
	}
	else
	{
		assert(false);
	}

	if (doAttack)
	{
		firstAttackSent = true;
	}

	return doAttack || firstAttackSent;
}

const bool StrategyManager::expandProtossZealotRush() const
{
	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
		return false;
	}

	int numNexus =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numZealots =			BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Zealot);
	int frame =					BWAPI::Broodwar->getFrameCount();

	// if there are more than 10 idle workers, expand
	if (WorkerManager::Instance().getNumIdleWorkers() > 10)
	{
		return true;
	}

	// 2nd Nexus Conditions:
	//		We have 12 or more zealots
	//		It is past frame 7000
	if ((numNexus < 2) && (numZealots > 12 || frame > 9000))
	{
		return true;
	}

	// 3nd Nexus Conditions:
	//		We have 24 or more zealots
	//		It is past frame 12000
	if ((numNexus < 3) && (numZealots > 24 || frame > 15000))
	{
		return true;
	}

	if ((numNexus < 4) && (numZealots > 24 || frame > 21000))
	{
		return true;
	}

	if ((numNexus < 5) && (numZealots > 24 || frame > 26000))
	{
		return true;
	}

	if ((numNexus < 6) && (numZealots > 24 || frame > 30000))
	{
		return true;
	}

	return false;
}

const MetaPairVector StrategyManager::getBuildOrderGoal()
{
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
	{
		if (getCurrentStrategy() == ProtossZealotRush)
		{
			return getProtossZealotRushBuildOrderGoal();
		}
		else if (getCurrentStrategy() == ProtossDarkTemplar)
		{
			return getProtossDarkTemplarBuildOrderGoal();
		}
		else if (getCurrentStrategy() == ProtossDragoons)
		{
			return getProtossDragoonsBuildOrderGoal();
		}

		// if something goes wrong, use zealot goal
		return getProtossZealotRushBuildOrderGoal();
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran)
	{
		if(getCurrentStrategy() == TerranMarineRush)
		{
			return getTerranMarineRushBuildOrderGoal();
		}
		else if(getCurrentStrategy() == TerranMarinesAndMedics)
		{
			return getTerranMarinesAndMedicsBuildOrderGoal();
		}
		else if(getCurrentStrategy() == TerranSiegePush)
		{
			return getTerranSiegePushBuildOrderGoal();
		}
		else if(getCurrentStrategy() == TerranNukes)
		{
			return getTerranNukesBuildOrderGoal();
		}

		return getTerranMarinesAndMedicsBuildOrderGoal();
	}
	else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg)
	{
		return getZergBuildOrderGoal();
	}
	else
	{
		assert(false);
		return getZergBuildOrderGoal();
	}
}

const MetaPairVector StrategyManager::getProtossDragoonsBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int dragoonsWanted = numDragoons > 0 ? numDragoons + 6 : 2;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 6;

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}
	else
	{
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}

		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || numDragoons > 6 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 2));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));

	return goal;
}

const MetaPairVector StrategyManager::getProtossDarkTemplarBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numDarkTeplar =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar);
	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int darkTemplarWanted = 0;
	int dragoonsWanted = numDragoons + 6;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 6;

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
		
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
	}

	if (numDragoons > 0)
	{
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		dragoonsWanted = numDragoons + 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, darkTemplarWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));
	
	return goal;
}

const MetaPairVector StrategyManager::getProtossZealotRushBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numZealots =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Zealot);
	int numDragoons =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted =		BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber =				BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);

	int zealotsWanted = numZealots + 8;
	int dragoonsWanted = numDragoons;
	int gatewayWanted = 3;
	int probesWanted = numProbes + 4;

	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
		
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

	if (numNexusAll >= 2 || BWAPI::Broodwar->getFrameCount() > 9000)
	{
		gatewayWanted = 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Assimilator, 1));
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1));
	}

	if (numCyber > 0)
	{
		dragoonsWanted = numDragoons + 2;
		goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
	}

	if (numNexusCompleted >= 3)
	{
		gatewayWanted = 8;
		dragoonsWanted = numDragoons + 6;
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
	}

	if (numNexusAll > 1)
	{
		probesWanted = numProbes + 6;
	}

	if (expandProtossZealotRush())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon,	dragoonsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot,	zealotsWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Gateway,	gatewayWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe,	std::min(90, probesWanted)));

	return goal;
}

const MetaPairVector StrategyManager::getTerranMarineRushBuildOrderGoal() const
{
	MetaPairVector goal; // the goal to return

	int numMarines = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);

	int marinesWanted = numMarines + 4;

	goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Marine,	marinesWanted));

	return goal;
}

const bool StrategyManager::expandTerranMnMRush() const
{
	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
		return false;
	}

	int numCC =						BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Command_Center);
	int numMarines =			BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int frame =						BWAPI::Broodwar->getFrameCount();

	// if there are more than 10 idle workers, expand
	if (WorkerManager::Instance().getNumIdleWorkers() > 12)
	{
		return true;
	}

	// 2nd CC Conditions:
	//		We have 24 or more marines
	//		It is past frame 7000
	if ((numCC < 2) && (numMarines > 24 && frame > 12000))
	{
		return true;
	}

	// 3nd Nexus Conditions:
	//		We have 36 or more marines
	//		It is past frame 12000
	if ((numCC < 3) && (numMarines > 36 || frame > 17000))
	{
		return true;
	}

	if ((numCC < 4) && (numMarines > 36 || frame > 23000))
	{
		return true;
	}

	if ((numCC < 5) && (numMarines > 36 || frame > 28000))
	{
		return true;
	}

	if ((numCC < 6) && (numMarines > 36 || frame > 32000))
	{
		return true;
	}

	return false;
}

const MetaPairVector StrategyManager::getTerranMarinesAndMedicsBuildOrderGoal() const
{
	MetaPairVector goal; // the goal to return

	int numMarines =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int numMedics =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Medic);
	int numFirebats =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Firebat);
	int numCC =						BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Command_Center);
	
	int marinesWanted = numMarines + 6;
	int medicsWanted = std::min(numMedics + 3, (int) ((marinesWanted / 10.0) * 3)); // ~3 medics for every 10 marines

	if (BWAPI::Broodwar->enemy()->allUnitCount(BWAPI::UnitTypes::Protoss_Zealot) > 3)
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Academy) == 0)
		{
			goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Academy, 1));
		}
		else
		{
			marinesWanted -= 3;
			medicsWanted -= 1;
			goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Firebat, numFirebats + 3));
		}
	}

	goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Marine,	marinesWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Medic,		medicsWanted));

	if (numMarines > 17 || 
			(BWAPI::Broodwar->self()->minerals() - BuildMan.getReservedMinerals() > 800))
	{
		if (!BuildMan.isBeingBuilt(BWAPI::UnitTypes::Terran_Barracks))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Barracks, 5));
		if (!BuildMan.isBeingBuilt(BWAPI::UnitTypes::Terran_Bunker))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Bunker, 2));
	}
	else
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Barracks, 3));
	}

	if (canResearch(BWAPI::UpgradeTypes::U_238_Shells) &&
			numMarines > 19)
	{
		goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::U_238_Shells, 1));
	}
	// don't add at the same time as u238 so that we don't build 2 academies
	if (canResearch(BWAPI::TechTypes::Stim_Packs) &&
			BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::U_238_Shells) == 1 &&
			numMarines > 22)
	{
		goal.push_back(std::pair<MetaType, int>(BWAPI::TechTypes::Stim_Packs, 1));
	}
	if (canResearch(BWAPI::UpgradeTypes::Caduceus_Reactor) &&
			BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Stim_Packs) &&
			numMarines > 22 && numMedics > 7)
	{
		goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Caduceus_Reactor, 1));
	}

	int weap = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Terran_Infantry_Weapons);
	int armo = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Terran_Infantry_Armor);
	if (numMarines > 22 + (weap * 5) && weap < 3) // upgrade weaps every 18 marines
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Engineering_Bay) == 0 &&
				!BuildMan.isBeingBuilt(BWAPI::UnitTypes::Terran_Engineering_Bay))
		{
			goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Engineering_Bay, 1));
		}
		else
		{
			if (weap > 0 && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Science_Facility) == 0 &&
					!BuildMan.isBeingBuilt(BWAPI::UnitTypes::Terran_Science_Facility))
			{
				goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Science_Facility, 1));
			}
			else if (canResearch(BWAPI::UpgradeTypes::Terran_Infantry_Weapons))
			{
				goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Terran_Infantry_Weapons, 1));
			}
		}
	}
	if (numMarines > 22 + (armo * 5) && // upgrade armos every 18 marines but after weaps
			weap > armo &&
			canResearch(BWAPI::UpgradeTypes::Terran_Infantry_Armor))
	{
		goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Terran_Infantry_Armor, 1));
	}

	if (expandTerranMnMRush())
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Command_Center, numCC + 1));
	else
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Command_Center, std::min(1, numCC)));

	return goal;
}

const MetaPairVector StrategyManager::getTerranSiegePushBuildOrderGoal() const
{
	MetaPairVector goal;

	int numMarines = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);

	int marinesWanted = numMarines + 12;
	int siegeTanksWanted = 4;

	goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Marine, marinesWanted));
    goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode,	siegeTanksWanted));
    goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Factory,               1));
    goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Machine_Shop,          1));
    if(!BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode))
    {
        goal.push_back(MetaPair(BWAPI::TechTypes::Tank_Siege_Mode, 1));
    }

	return goal;
}

const MetaPairVector StrategyManager::getTerranNukesBuildOrderGoal() const
{
    MetaPairVector goal;

	int numMarines =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int numMissileTurrets =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Missile_Turret);
	
	int marinesWanted = numMarines + 12;
	int missileTurretsWanted = 5 - numMissileTurrets;

	goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Marine, marinesWanted));
	goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Bunker, 4));
	if(BWAPI::Broodwar->getFrameCount() > 7000 && missileTurretsWanted)
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Missile_Turret, missileTurretsWanted));

    return goal;
}

const MetaPairVector StrategyManager::getTerranStormDrop() const
{
	MetaPairVector goal; // the goal to return

	int numMarines =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);

	if (numMarines > 20 || 
			(BWAPI::Broodwar->self()->minerals() - BuildMan.getReservedMinerals() > 900))
	{
		if (!BuildMan.isBeingBuilt(BWAPI::UnitTypes::Terran_Barracks))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Barracks, 1));
		if (!BuildMan.isBeingBuilt(BWAPI::UnitTypes::Terran_Factory))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Factory, 1));
		if (!BuildMan.isBeingBuilt(BWAPI::UnitTypes::Terran_Starport))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Starport, 1));
		if (!BuildMan.isBeingBuilt(BWAPI::UnitTypes::Terran_Control_Tower))
			goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Control_Tower, 1));

		// add check if no dropships are in base, then create more dropships
		goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Dropship, 2));
	}

	return goal;
}

const MetaPairVector StrategyManager::getZergBuildOrderGoal() const
{
	// the goal to return
	std::vector< std::pair<MetaType, UnitCountType> > goal;
	
	int numMutas  =				BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
	int numHydras  =			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk);

	int mutasWanted = numMutas + 6;
	int hydrasWanted = numHydras + 6;

	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, 4));

	return (const std::vector< std::pair<MetaType, UnitCountType> >)goal;
}

const int StrategyManager::getCurrentStrategy()
{
	return currentStrategy;
}

const bool StrategyManager::canResearch(BWAPI::TechType research) const
{
	return !BWAPI::Broodwar->self()->isResearching(research) &&
				 !BWAPI::Broodwar->self()->hasResearched(research);
}

const bool StrategyManager::canResearch(BWAPI::UpgradeType research) const
{
	return !BWAPI::Broodwar->self()->isUpgrading(research) &&
				 (BWAPI::Broodwar->self()->getUpgradeLevel(research) < 1);
					//BWAPI::Broodwar->self()->getMaxUpgradeLevel(research));
}

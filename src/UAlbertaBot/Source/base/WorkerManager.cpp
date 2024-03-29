#include "Common.h"
#include "WorkerManager.h"

WorkerManager::WorkerManager() 
    : workersPerRefinery(3) 
{
    previousClosestWorker = NULL;
}

void WorkerManager::update() 
{
	// worker bookkeeping
	updateWorkerStatus();

	// set the gas workers
	handleGasWorkers();

	// handle idle workers
	handleIdleWorkers();

	// handle move workers
	handleMoveWorkers();

	// handle combat workers
	handleCombatWorkers();

	drawResourceDebugInfo();
	//drawWorkerInformation(450,20);

	workerData.drawDepotDebugInfo();
}

void WorkerManager::updateWorkerStatus() 
{
	// for each of our Workers
	BOOST_FOREACH (BWAPI::Unit * worker, workerData.getWorkers())
	{
		if (!worker->isCompleted())
		{
			continue;
		}

		// if it's idle
		if (worker->isIdle() && 
			(workerData.getWorkerJob(worker) != WorkerData::Build) && 
			(workerData.getWorkerJob(worker) != WorkerData::Move) &&
			(workerData.getWorkerJob(worker) != WorkerData::Scout)) 
		{
			//printf("Worker %d set to idle", worker->getID());
			// set its job to idle
			workerData.setWorkerJob(worker, WorkerData::Idle, NULL);
		}

		// if its job is gas
		if (workerData.getWorkerJob(worker) == WorkerData::Gas)
		{
			BWAPI::Unit * refinery = workerData.getWorkerResource(worker);

			// if the refinery doesn't exist anymore
			if (!refinery || !refinery->exists() ||	refinery->getHitPoints() <= 0)
			{
				setMineralWorker(worker);
			}
		}
	}
}

void WorkerManager::handleGasWorkers() 
{
	// for each unit we have
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		// if that unit is a refinery
		if (unit->getType().isRefinery() && unit->isCompleted())
		{
			// get the number of workers currently assigned to it
			int numAssigned = workerData.getNumAssignedWorkers(unit);

			// if it's less than we want it to be, fill 'er up
			for (int i=0; i<(workersPerRefinery-numAssigned); ++i)
			{
				BWAPI::Unit * gasWorker = getGasWorker(unit);
				if (gasWorker)
				{
					workerData.setWorkerJob(gasWorker, WorkerData::Gas, unit);
				}
			}
		}
	}
}

void WorkerManager::handleIdleWorkers() 
{
	// for each of our workers
	BOOST_FOREACH (BWAPI::Unit * worker, workerData.getWorkers())
	{
		// if it is idle
		if (workerData.getWorkerJob(worker) == WorkerData::Idle) 
		{
            bool assigned = false;
            // send workers to repair
            if(worker->getType() == BWAPI::UnitTypes::Terran_SCV)
			{
                BOOST_FOREACH(BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits())
                {
                    if((u->getType() == BWAPI::UnitTypes::Terran_Bunker ||
                            u->getType() == BWAPI::UnitTypes::Terran_Missile_Turret) &&
                            u->getHitPoints() < u->getType().maxHitPoints() &&
                            !u->isBeingConstructed() &&
							workerData.getWorkerRepairing(u) == NULL)
                    {
                        setRepairWorker(worker, u);
                        assigned = true;
                        break;
                    }
                }
			}
			// send it to the nearest mineral patch
            if(!assigned)
			{
                setMineralWorker(worker);
			}
		}
	}
}

void WorkerManager::handleMineralWorkers()
{
	// for each of our workers
	BOOST_FOREACH (BWAPI::Unit * worker, workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Minerals &&
			    worker->getType() == BWAPI::UnitTypes::Terran_SCV) 
		{
            BOOST_FOREACH(BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits())
            {
                if((u->getType() == BWAPI::UnitTypes::Terran_Bunker ||
                        u->getType() == BWAPI::UnitTypes::Terran_Missile_Turret) &&
                        u->getHitPoints() < u->getType().maxHitPoints() &&
                        !u->isBeingConstructed() &&
                        workerData.getWorkerRepairing(u) == NULL)
                {
                    setRepairWorker(worker, u);
                    break;
                }
            }
        }
	}
}

// bad micro for combat workers
void WorkerManager::handleCombatWorkers()
{
	BOOST_FOREACH (BWAPI::Unit * worker, workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Combat)
		{
			BWAPI::Broodwar->drawCircleMap(worker->getPosition().x(), worker->getPosition().y(), 4, BWAPI::Colors::Yellow, true);
			BWAPI::Unit * target = getClosestEnemyUnit(worker);

			if (target)
			{
				smartAttackUnit(worker, target);
			}
		}
	}
}

BWAPI::Unit * WorkerManager::getClosestEnemyUnit(BWAPI::Unit * worker)
{
	BWAPI::Unit * closestUnit = NULL;
	double closestDist = 10000;

	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		double dist = unit->getDistance(worker);

		if ((dist < 400) && (!closestUnit || (dist < closestDist)))
		{
			closestUnit = unit;
			closestDist = dist;
		}
	}

	return closestUnit;
}

void WorkerManager::finishedWithCombatWorkers()
{
	BOOST_FOREACH (BWAPI::Unit * worker, workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Combat)
		{
			setMineralWorker(worker);
		}
	}
}

BWAPI::Unit * WorkerManager::getClosestMineralWorkerTo(BWAPI::Unit * enemyUnit)
{
    BWAPI::Unit * closestMineralWorker = NULL;
    double closestDist = 100000;

    if (previousClosestWorker)
    {
        if (previousClosestWorker->getHitPoints() > 0)
        {
            return previousClosestWorker;
        }
    }

    // for each of our workers
	BOOST_FOREACH (BWAPI::Unit * worker, workerData.getWorkers())
	{
		// if it is a move worker
        if (workerData.getWorkerJob(worker) == WorkerData::Minerals) 
		{
			double dist = worker->getDistance(enemyUnit);

            if (!closestMineralWorker || dist < closestDist)
            {
                closestMineralWorker = worker;
                dist = closestDist;
            }
		}
	}

    previousClosestWorker = closestMineralWorker;
    return closestMineralWorker;
}

void WorkerManager::handleMoveWorkers() 
{
	// for each of our workers
	BOOST_FOREACH (BWAPI::Unit * worker, workerData.getWorkers())
	{
		// if it is a move worker
		if (workerData.getWorkerJob(worker) == WorkerData::Move) 
		{
			WorkerMoveData data = workerData.getWorkerMoveData(worker);
			
			worker->move(data.position);
		}
	}
}

void WorkerManager::setRepairWorker(BWAPI::Unit * unit, BWAPI::Unit * target)
{
	if (unit == NULL) 
	{
		assert(false);
	}

	workerData.setWorkerJob(unit, WorkerData::Repair, target);
}

// set a worker to mine minerals
void WorkerManager::setMineralWorker(BWAPI::Unit * unit)
{
	if (unit == NULL) 
	{
		assert(false);
	}

	// check if there is a mineral available to send the worker to
	BWAPI::Unit * depot = getClosestDepot(unit);

	// if there is a valid mineral
	if (depot)
	{
		// update workerData with the new job
		workerData.setWorkerJob(unit, WorkerData::Minerals, depot);
	}
	else
	{
		// BWAPI::Broodwar->printf("No valid depot for mineral worker");
	}
}

BWAPI::Unit * WorkerManager::getClosestDepot(BWAPI::Unit * worker)
{
	if (worker == NULL) 
	{
		assert(false);
	}

	BWAPI::Unit * closestDepot = NULL;
	double closestDistance = 0;

	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType().isResourceDepot() && unit->isCompleted() && !workerData.depotIsFull(unit))
		{
			double distance = unit->getDistance(worker);
			if (!closestDepot || distance < closestDistance)
			{
				closestDepot = unit;
				closestDistance = distance;
			}
		}
	}

	return closestDepot;
}

// other managers that need workers call this when they're done with a unit
void WorkerManager::finishedWithWorker(BWAPI::Unit * unit) 
{
	if (unit == NULL)
	{
		BWAPI::Broodwar->printf("finishedWithWorker() called with NULL unit");
		return; 
	}

	//BWAPI::Broodwar->printf("BuildingManager finished with worker %d", unit->getID());
	if (workerData.getWorkerJob(unit) != WorkerData::Scout)
	{
		workerData.setWorkerJob(unit, WorkerData::Idle, NULL);
	}
}

BWAPI::Unit * WorkerManager::getGasWorker(BWAPI::Unit * refinery)
{
	if (refinery == NULL) 
	{
		assert(false);
	}

	BWAPI::Unit * closestWorker = NULL;
	double closestDistance = 0;

	BOOST_FOREACH (BWAPI::Unit * unit, workerData.getWorkers())
	{
		if (workerData.getWorkerJob(unit) == WorkerData::Minerals)
		{
			double distance = unit->getDistance(refinery);
			if (!closestWorker || distance < closestDistance)
			{
				closestWorker = unit;
				closestDistance = distance;
			}
		}
	}

	return closestWorker;
}

// gets a builder for BuildingManager to use
// if setJobAsBuilder is true (default), it will be flagged as a builder unit
// set 'setJobAsBuilder' to false if we just want to see which worker will build a building
BWAPI::Unit * WorkerManager::getBuilder(Building & b, bool setJobAsBuilder)
{
	// variables to hold the closest worker of each type to the building
	BWAPI::Unit * closestMovingWorker = NULL;
	BWAPI::Unit * closestMiningWorker = NULL;
	double closestMovingWorkerDistance = 0;
	double closestMiningWorkerDistance = 0;

	// look through each worker that had moved there first
	BOOST_FOREACH (BWAPI::Unit * unit, workerData.getWorkers())
	{
		// mining worker check
		if (unit->isCompleted() && (workerData.getWorkerJob(unit) == WorkerData::Minerals))
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(BWAPI::Position(b.finalPosition));
			if (!closestMiningWorker || distance < closestMiningWorkerDistance)
			{
				closestMiningWorker = unit;
				closestMiningWorkerDistance = distance;
			}
		}

		// moving worker check
		if (unit->isCompleted() && (workerData.getWorkerJob(unit) == WorkerData::Move))
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(BWAPI::Position(b.finalPosition));
			if (!closestMovingWorker || distance < closestMovingWorkerDistance)
			{
				closestMovingWorker = unit;
				closestMovingWorkerDistance = distance;
			}
		}
	}

	// if we found a moving worker, use it, otherwise using a mining worker
	BWAPI::Unit * chosenWorker = closestMovingWorker ? closestMovingWorker : closestMiningWorker;

	// if the worker exists (one may not have been found in rare cases)
	if (chosenWorker && setJobAsBuilder)
	{
		workerData.setWorkerJob(chosenWorker, WorkerData::Build, b.type);
	}

	return chosenWorker; // return the worker
}

// sets a worker as a scout
void WorkerManager::setScoutWorker(BWAPI::Unit * worker)
{
	if (worker == NULL) 
	{
		assert(false);
	}

	workerData.setWorkerJob(worker, WorkerData::Scout, NULL);
}

// gets a worker which will move to a current location
BWAPI::Unit * WorkerManager::getMoveWorker(BWAPI::Position p)
{
	// set up the pointer
	BWAPI::Unit * closestWorker = NULL;
	double closestDistance = 0;

	// for each worker we currently have
	BOOST_FOREACH (BWAPI::Unit * unit, workerData.getWorkers())
	{
		// only consider it if it's a mineral worker
		if (unit->isCompleted() && workerData.getWorkerJob(unit) == WorkerData::Minerals)
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(p);
			if (!closestWorker || distance < closestDistance)
			{
				closestWorker = unit;
				closestDistance = distance;
			}
		}
	}

	return closestWorker; // return the worker
}

// sets a worker to move to a given location
void WorkerManager::setMoveWorker(int mineralsNeeded, int gasNeeded, BWAPI::Position p)
{
	// set up the pointer
	BWAPI::Unit * closestWorker = NULL;
	double closestDistance = 0;

	// for each worker we currently have
	BOOST_FOREACH (BWAPI::Unit * unit, workerData.getWorkers())
	{
		// only consider it if it's a mineral worker
		if (unit->isCompleted() && workerData.getWorkerJob(unit) == WorkerData::Minerals)
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(p);
			if (!closestWorker || distance < closestDistance)
			{
				closestWorker = unit;
				closestDistance = distance;
			}
		}
	}

	if (closestWorker)
	{
		//BWAPI::Broodwar->printf("Setting worker job Move for worker %d", closestWorker->getID());
		workerData.setWorkerJob(closestWorker, WorkerData::Move, WorkerMoveData(mineralsNeeded, gasNeeded, p));
	}
	else
	{
		//BWAPI::Broodwar->printf("Error, no worker found");
	}
}

// will we have the required resources by the time a worker can travel a certain distance
bool WorkerManager::willHaveResources(int mineralsRequired, int gasRequired, double distance)
{
	// if we don't require anything, we will have it
	if (mineralsRequired <= 0 && gasRequired <= 0)
	{
		return true;
	}

	// the speed of the worker unit
	double speed = BWAPI::Broodwar->self()->getRace().getWorker().topSpeed();

	// how many frames it will take us to move to the building location
	// add a second to account for worker getting stuck. better early than late
	double framesToMove = (distance / speed) + 50;

	// magic numbers to predict income rates
	double mineralRate = getNumMineralWorkers() * 0.045;
	double gasRate     = getNumGasWorkers() * 0.07;

	// calculate if we will have enough by the time the worker gets there
	if (mineralRate * framesToMove >= mineralsRequired &&
			gasRate * framesToMove >= gasRequired)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void WorkerManager::setCombatWorker(BWAPI::Unit * worker)
{
	if (worker == NULL) 
	{
		assert(false);
	}

	workerData.setWorkerJob(worker, WorkerData::Combat, NULL);
}

void WorkerManager::onUnitMorph(BWAPI::Unit * unit)
{
	if (unit == NULL) 
	{
		assert(false);
	}

	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getHitPoints() >= 0)
	{
		workerData.addWorker(unit);
	}

	// if something morphs into a building, it was a worker?
	if (unit->getType().isBuilding() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getPlayer()->getRace() == BWAPI::Races::Zerg)
	{
		//BWAPI::Broodwar->printf("A Drone started building");
		workerData.workerDestroyed(unit);
	}
}

void WorkerManager::onUnitShow(BWAPI::Unit * unit)
{
	if (unit == NULL) 
	{
		assert(false);
	}

	// add the depot if it exists
	if (unit->getType().isResourceDepot() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		workerData.addDepot(unit);
	}

	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getHitPoints() >= 0)
	{
		//BWAPI::Broodwar->printf("A worker was shown %d", unit->getID());
		workerData.addWorker(unit);
	}
}

void WorkerManager::rebalanceWorkers()
{
	// for each worker
	BOOST_FOREACH (BWAPI::Unit * worker, workerData.getWorkers())
	{
		// we only care to rebalance mineral workers
		if (!workerData.getWorkerJob(worker) == WorkerData::Minerals)
		{
			continue;
		}

		// get the depot this worker works for
		BWAPI::Unit * depot = workerData.getWorkerDepot(worker);

		// if there is a depot and it's full
		if (depot && workerData.depotIsFull(depot))
		{
			// set the worker to idle
			workerData.setWorkerJob(worker, WorkerData::Idle, NULL);
		}
		// if there's no depot
		else if (!depot)
		{
			// set the worker to idle
			workerData.setWorkerJob(worker, WorkerData::Idle, NULL);
		}
	}
}

void WorkerManager::onUnitDestroy(BWAPI::Unit * unit) 
{
	if (unit == NULL) 
	{
		assert(false);
	}

	// remove the depot if it exists
	if (unit->getType().isResourceDepot() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		workerData.removeDepot(unit);
	}

	// if the unit that was destroyed is a worker
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self()) 
	{
		// tell the worker data it was destroyed
		workerData.workerDestroyed(unit);
	}

	if (unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field)
	{
		//BWAPI::Broodwar->printf("A mineral died, rebalancing workers");
		rebalanceWorkers();
	}
}

void WorkerManager::smartAttackUnit(BWAPI::Unit * attacker, BWAPI::Unit * target)
{
	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || attacker->isAttackFrame())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit &&	currentCommand.getTarget() == target)
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->attack(target);
}

void WorkerManager::drawResourceDebugInfo() 
{
	BOOST_FOREACH (BWAPI::Unit * worker, workerData.getWorkers()) 
	{
		char job = workerData.getJobCode(worker);

		BWAPI::Position pos = worker->getTargetPosition();

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(worker->getPosition().x(), worker->getPosition().y() - 5, "\x07%c", job);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(worker->getPosition().x(), worker->getPosition().y(), pos.x(), pos.y(), BWAPI::Colors::Cyan);

		BWAPI::Unit * depot = workerData.getWorkerDepot(worker);
		if (depot)
		{
			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(worker->getPosition().x(), worker->getPosition().y(), depot->getPosition().x(), depot->getPosition().y(), BWAPI::Colors::Orange);
		}
	}
}

void WorkerManager::drawWorkerInformation(int x, int y) 
{
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Workers %d", workerData.getNumMineralWorkers());
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+20, "\x04 UnitID");
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+50, y+20, "\x04 State");

	int yspace = 0;

	BOOST_FOREACH (BWAPI::Unit * unit, workerData.getWorkers())
	{
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+40+((yspace)*10), "\x03 %d", unit->getID());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+50, y+40+((yspace++)*10), "\x03 %c", workerData.getJobCode(unit));
	}
}

bool WorkerManager::isFree(BWAPI::Unit * worker)
{
	return workerData.getWorkerJob(worker) == WorkerData::Minerals || workerData.getWorkerJob(worker) == WorkerData::Idle;
}

bool WorkerManager::isWorkerScout(BWAPI::Unit * worker)
{
	return (workerData.getWorkerJob(worker) == WorkerData::Scout);
}

bool WorkerManager::isBuilder(BWAPI::Unit * worker)
{
	return (workerData.getWorkerJob(worker) == WorkerData::Build);
}

bool WorkerManager::isMineralWorker(BWAPI::Unit * worker)
{
	return (workerData.getWorkerJob(worker) == WorkerData::Minerals);
}

bool WorkerManager::isGasWorker(BWAPI::Unit * worker)
{
	return (workerData.getWorkerJob(worker) == WorkerData::Gas);
}

bool WorkerManager::isGathererWorker(BWAPI::Unit * worker)
{
	return (isMineralWorker(worker) || isGasWorker(worker));
}

int WorkerManager::getNumMineralWorkers() 
{
	return workerData.getNumMineralWorkers();	
}

int WorkerManager::getNumIdleWorkers() 
{
	return workerData.getNumIdleWorkers();	
}

int WorkerManager::getNumGasWorkers() 
{
	return workerData.getNumGasWorkers();
}

WorkerManager & WorkerManager::Instance() 
{
	static WorkerManager instance;
	return instance;
}

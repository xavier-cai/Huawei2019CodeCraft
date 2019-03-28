#ifndef DEAD_LOCK_SOLVER_H
#define DEAD_LOCK_SOLVER_H

#include "sim-scenario.h"
#include <map>
#include <set>
#include <list>
#include "memory-pool.h"
#include "callback.h"

class DeadLockSolver
{
public:
    DeadLockSolver();
    void Initialize(const int& time, SimScenario& scenario);
    bool HandleDeadLock(int& time, SimScenario& scenario);
    bool IsCarTraceLockedInBackup(SimCar* car) const; //interface for keep trace, check it if you want to change path of car
    bool IsGarageLockedInBackup(const int& time) const; //check it when handle car get out garage 
    void SetSelectedRoadCallback(const Callback::Handle3<int, const std::list<int>&, int, int>& cb);

private:
    MemoryPool m_memoryPool;

    SimScenario m_backupScenario;
    int m_backupTime;

    int m_deadLockTime;
    int* m_deadLockTraceIndexes; //for recover to dead lock, need to keep the trace
    std::map< int, std::set<int> > m_deadLockMemory; //car id -> used next road id

    Callback::Handle3<int, const std::list<int>&, int, int> m_selectedRoadCallback; //for selecting new road to break dead lock

};//class DeadLockSolver

#endif
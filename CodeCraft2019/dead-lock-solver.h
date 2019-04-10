#ifndef DEAD_LOCK_SOLVER_H
#define DEAD_LOCK_SOLVER_H

#include "sim-scenario.h"
#include <map>
#include <set>
#include <list>
#include <utility>
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
    void SetSelectedRoadCallback(const Callback::Handle3<std::pair<int, bool>, SimScenario&, const std::list<int>&, SimCar*>& cb); //return selection & is car trace handled
    const int& GetDeadLockTime() const;
    bool NeedUpdate(const int& time) const;

private:
    bool DoHandleDeadLock(int& time, SimScenario& scenario);

    MemoryPool m_memoryPool;

    SimScenario* m_backupScenario;
    int m_backupTime;

    int m_deadLockTime;
    int m_firstLockOnTime;
    int* m_deadLockTraceIndexes; //for recover to dead lock, need to keep the trace
    std::map< int, std::set<int> > m_deadLockMemory; //car id -> used next road id
    int m_depth;
    bool m_actived;
    DeadLockSolver* m_subSolver;

    Callback::Handle3<std::pair<int, bool>, SimScenario&, const std::list<int>&, SimCar*> m_selectedRoadCallback; //for selecting new road to break dead lock

};//class DeadLockSolver

#endif
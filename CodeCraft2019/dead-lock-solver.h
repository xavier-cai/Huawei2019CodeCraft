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
    const int& GetDeadLockTime() const;
    bool NeedUpdate(const int& time) const;
    void Backup(const int& time, const SimScenario& scenario);

    void SetSelectedRoadCallback(const Callback::Handle3<std::pair<int, bool>, SimScenario&, const std::vector<int>&, SimCar*>& cb); //return selection & is car trace handled
    void SetOperationDelaySingleRoad(const bool& enable);

private:
    int DoHandleDeadLock(int& time, SimScenario& scenario);
    void UpdateFirstLockOnTime(const int& time); //update first lock on time to smaller one
    bool OperationDelay(const int& time, SimScenario& scenario, std::list<SimCar*>& deadLockCars);
    bool OperationChangeTrace(const int& time, SimScenario& scenario, std::list<SimCar*>& cars);

    MemoryPool m_memoryPool;
    std::map<int, SimScenario*> m_backups; //backup of scenario, used for time leap

    int m_deadLockTime; //the time of dead lock
    int m_firstLockOnTime; //the time of world line changed after operations, El Psy Congroo! used for time leap
    int* m_deadLockTraceIndexes; //for recover to dead lock, need to keep the trace
    int m_depth; //depth of dead lock solver
    bool m_actived; //flag: is this solver solving the dead lock?
    DeadLockSolver* m_subSolver; //create a sub solver when dead lock occurs in smaller time than before

    bool m_isSingleRoadDelay;
    std::map< int, std::set<int> > m_deadLockMemory; //car id -> used next road id
    Callback::Handle3<std::pair<int, bool>, SimScenario&, const std::vector<int>&, SimCar*> m_selectedRoadCallback; //for selecting new road to break dead lock

};//class DeadLockSolver

#endif
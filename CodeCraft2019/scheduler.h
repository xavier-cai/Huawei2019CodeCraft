#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "sim-scenario.h"
#include "simulator.h"
#include "map-array.h"
#include "memory-pool.h"
#include "timer.h"

class Scheduler
{
protected:
    Scheduler();

public:
    virtual ~Scheduler();

    void Initialize(SimScenario& scenario);
    void Update(int& time, SimScenario& scenario); //before simulator update
    void HandleGetoutGarage(const int& time, SimScenario& scenario, SimCar* car);
    void HandleBecomeFirstPriority(const int& time, SimScenario& scenario, SimCar* car);
    void HandleResult(int& time, Simulator::UpdateResult& result); //after simulator update

protected:
    virtual void DoInitialize(SimScenario& scenario);
    virtual void DoUpdate(int& time, SimScenario& scenario);
    virtual void DoHandleGetoutGarage(const int& time, SimScenario& scenario, SimCar* car);
    virtual void DoHandleBecomeFirstPriority(const int& time, SimScenario& scenario, SimCar* car);
    virtual void DoHandleResult(int& time, Simulator::UpdateResult& result);

    MemoryPool m_memoryPool;

    /* for calculating algorithm time cost */
    void UpdateTimeCostBegin(const std::string& id);
    void UpdateTimeCostEnd(const std::string& id);

private:
    /* for calculating algorithm time cost */
    struct CostStatistic
    {
        CostStatistic();
        int UpdateCount;
        double UpdateAverage;
        double UpdateMax;
        double UpdateMin;
        TimerHandle Handle;
    };//struct CostStatistic
    std::map<std::string, CostStatistic> m_statistic;

};//class Scheduler

#endif
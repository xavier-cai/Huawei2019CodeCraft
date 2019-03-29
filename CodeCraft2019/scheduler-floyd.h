#ifndef SCHEDULER_FLOYD_H
#define SCHEDULER_FLOYD_H

#include "scheduler.h"
#include <list>
#include "dead-lock-solver.h"

class SchedulerFloyd : public Scheduler
{
public:
    SchedulerFloyd();

    static double lenthWeight;
    static double carNumWeight;
    static double carLimit;
    static double lanesWeight;

protected:
    virtual void DoInitialize(SimScenario& scenario) override;
    virtual void DoUpdate(int& time, SimScenario& scenario) override;
    virtual void DoHandleGetoutGarage(const int& time, SimScenario& scenario, SimCar* car) override;
    virtual void DoHandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result) override;

private:
    double** crosslength;
    int** crosspath;
    std::list<int>** minpath;
    double minspeed;
    //int* crossspeed;

    DeadLockSolver m_deadLockSolver;
    int SelectBestRoad(const std::list<int>& list, int from, int to);

};//class SchedulerFloyd

#endif
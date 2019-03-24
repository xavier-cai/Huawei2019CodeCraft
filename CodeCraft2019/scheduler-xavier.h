#ifndef SCHEDULER_XAVIER_H
#define SCHEDULER_XAVIER_H

#include "scheduler.h"
#include <set>
#include <list>

class SchedulerXavier : public Scheduler
{
protected:
    virtual void DoInitialize(SimScenario& scenario) override;

private:
    int UpdatePathAndCost(int current, int target, std::set<int>& bans, std::list<int>& list);

    bool*** m_state;
    double*** m_cost;
    std::list<int>*** m_path;

};//class SchedulerXavier

#endif
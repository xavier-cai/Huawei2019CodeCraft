#ifndef SCHEDULER_ANSWER_H
#define SCHEDULER_ANSWER_H

#include "scheduler.h"
#include <iostream>

class SchedulerAnswer : public Scheduler
{
protected:
    virtual void DoInitialize(SimScenario& scenario) override;

private:
    SimScenario* m_scenario;
    bool HandleAnswer(std::istream& is);

};//class SchedulerAnswer

#endif
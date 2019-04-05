#ifndef SCHEDULER_ANSWER_H
#define SCHEDULER_ANSWER_H

#include "scheduler.h"
#include <iostream>
#include <fstream>

class SchedulerAnswer : public Scheduler
{
public:
    SchedulerAnswer(const std::string& traceFile = "");
    virtual ~SchedulerAnswer();

protected:
    virtual void DoInitialize(SimScenario& scenario) override;
    virtual void DoHandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result) override;

private:
    SimScenario* m_scenario;
    bool HandleAnswer(std::istream& is);
    std::ofstream* m_traceFile;

};//class SchedulerAnswer

#endif
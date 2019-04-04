#include "scheduler.h"
#include "log.h"
#include "timer.h"
#include "scenario.h"

Scheduler::Scheduler()
{ }

Scheduler::~Scheduler()
{
    m_memoryPool.Release();
}

void Scheduler::Initialize(SimScenario& scenario)
{
    DoInitialize(scenario);
}

void Scheduler::Update(int& time, SimScenario& scenario)
{
    DoUpdate(time, scenario);
}

void Scheduler::HandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result)
{
    if (Log::Get<LoadState>())
    {
        m_loadState.Update(time, scenario);
        if (scenario.IsComplete())
            m_loadState.Print(scenario);
    }
    DoHandleResult(time, scenario, result);
}

void Scheduler::HandleGetoutGarage(const int& time, SimScenario& scenario, SimCar* car)
{
    DoHandleGetoutGarage(time, scenario, car);
}

void Scheduler::HandleBecomeFirstPriority(const int& time, SimScenario& scenario, SimCar* car)
{
    DoHandleBecomeFirstPriority(time, scenario, car);
}

void Scheduler::DoInitialize(SimScenario& scenario)
{ }

void Scheduler::DoUpdate(int& time, SimScenario& scenario)
{ }

void Scheduler::DoHandleGetoutGarage(const int& time, SimScenario& scenario, SimCar* car)
{ }

void Scheduler::DoHandleBecomeFirstPriority(const int& time, SimScenario& scenario, SimCar* car)
{ }

void Scheduler::DoHandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result)
{ }

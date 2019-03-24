#include "scheduler.h"
#include "log.h"
#include "timer.h"
#include "scenario.h"

Scheduler::Scheduler()
{ }

Scheduler::~Scheduler()
{
    for (auto ite = m_statistic.begin(); ite != m_statistic.end(); ite++)
    {
        LOG("Scheduler statistic : " << ite->first);
        LOG("\t Update count : " << ite->second.UpdateCount);
        if (ite->second.UpdateCount > 0)
        {
            LOG("\t Average cost : " << ite->second.UpdateAverage);
            LOG("\t Minimum cost : " << ite->second.UpdateMin);
            LOG("\t Maximum cost : " << ite->second.UpdateMax);
        }
    }
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

void Scheduler::HandleResult(int& time, Simulator::UpdateResult& result)
{
    DoHandleResult(time, result);
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

void Scheduler::DoHandleResult(int& time, Simulator::UpdateResult& result)
{ }

void Scheduler::UpdateTimeCostBegin(const std::string& id)
{
    m_statistic[id].Handle = Timer::Record();
}

void Scheduler::UpdateTimeCostEnd(const std::string& id)
{
    auto find = m_statistic.find(id);
    ASSERT(find != m_statistic.end());
    double time = find->second.Handle.GetSpendTime();
    if (find->second.UpdateCount == 0)
    {
        find->second.UpdateAverage = time;
        find->second.UpdateMax = time;
        find->second.UpdateMin = time;
    }
    else
    {
        find->second.UpdateAverage = find->second.UpdateAverage / (find->second.UpdateCount + 1) * find->second.UpdateCount + time / (find->second.UpdateCount + 1);
        if (time > find->second.UpdateMax) find->second.UpdateMax = time;
        if (time < find->second.UpdateMin) find->second.UpdateMin = time;
    }
    find->second.UpdateCount++;
}

Scheduler::CostStatistic::CostStatistic()
    : UpdateCount(0), UpdateAverage(0), UpdateMax(0), UpdateMin(0), Handle(Timer::Record())
{ }

#include "scheduler.h"
#include "log.h"
#include "timer.h"
#include "scenario.h"
#include "assert.h"

Scheduler::Scheduler()
    : m_traceFile(0)
{ }

Scheduler::~Scheduler()
{
    if (m_traceFile != 0)
    {
        m_traceFile->flush();
        m_traceFile->close();
        delete m_traceFile;
    }
    m_memoryPool.Release();
}

void Scheduler::EnableTrace(const std::string& traceFile)
{
    ASSERT(traceFile.length() > 0);
    if (m_traceFile != 0)
    {
        m_traceFile->flush();
        m_traceFile->close();
        delete m_traceFile;
    }
    m_traceFile = new std::ofstream(traceFile.c_str());
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
    if (m_traceFile != 0)
    {
        std::ostream& os = *m_traceFile;
        os << "time:" << time << "\n";
        for (auto roadIte = scenario.Roads().begin(); roadIte != scenario.Roads().end(); ++roadIte)
        {
            os << "(" << roadIte->first << ",";
            os << "forward,[";
            int length = roadIte->second.GetRoad()->GetLength();
            for (int i = 1; i <= roadIte->second.GetRoad()->GetLanes(); ++i)
            {
                os << (i == 1 ? "" : ",") << "[";
                auto& cars = roadIte->second.GetCars(i);
                int index = length;
                for (auto carIte = cars.begin(); carIte != cars.end(); ++carIte)
                {
                    int pos = scenario.Cars()[(*carIte)->GetId()].GetCurrentPosition();
                    for (; index > pos; --index)
                        os << (index == length ? "" : ",") << "-1";
                    os << (index == length ? "" : ",") << (*carIte)->GetId();
                    --index;
                }
                for (; index > 0; --index)
                    os << (index == length ? "" : ",") << "-1";
                os << "]";
            }
            os << "])";
            if (roadIte->second.GetRoad()->GetIsTwoWay())
            {
                os << "\n(" << roadIte->first << ",";
                os << "backward,[";
                for (int i = 1; i <= roadIte->second.GetRoad()->GetLanes(); ++i)
                {
                    os << (i == 1 ? "" : ",") << "[";
                    auto& cars = roadIte->second.GetCarsOpposite(i);
                    int index = length;
                    for (auto carIte = cars.begin(); carIte != cars.end(); ++carIte)
                    {
                        int pos = scenario.Cars()[(*carIte)->GetId()].GetCurrentPosition();
                        for (; index > pos; --index)
                            os << (index == length ? "" : ",") << "-1";
                        os << (index == length ? "" : ",") << (*carIte)->GetId();
                        --index;
                    }
                    for (; index > 0; --index)
                        os << (index == length ? "" : ",") << "-1";
                    os << "]";
                }
                os << "])";
            }
            os << std::endl;
        }
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

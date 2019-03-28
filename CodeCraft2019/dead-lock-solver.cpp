#include "dead-lock-solver.h"
#include "scenario.h"
#include "log.h"
#include "assert.h"
#include "simulator.h"
#include "random.h"

DeadLockSolver::DeadLockSolver()
    : m_backupTime(-1), m_deadLockTime(-1), m_deadLockTraceIndexes(0)
{ }

void DeadLockSolver::Initialize(const int& time, SimScenario& scenario)
{
    m_backupScenario = scenario;
    m_backupTime = time;

    m_deadLockTraceIndexes = m_memoryPool.NewArray<int>(Scenario::Cars().size());
    for(unsigned int i = 0; i < Scenario::Cars().size(); ++i)
    {
        m_deadLockTraceIndexes[i] = 0;
    }
}

bool DeadLockSolver::HandleDeadLock(int& time, SimScenario& scenario)
{
    LOG("dead lock detected @" << time);
    if (time < m_deadLockTime)
        return false; //give up
    else if (time > m_deadLockTime) //another dead lock
        m_deadLockMemory.clear(); //reset

    //remember the trace
    for(auto ite = scenario.Cars().begin(); ite != scenario.Cars().end(); ite++)
    {
        m_deadLockTraceIndexes[_a(ite->second.GetCar()->GetId())] = ite->second.GetCurrentTraceIndex();
    }

    std::list<SimCar*> cars;
    Simulator::Instance.GetDeadLockCars(time, scenario, cars);
    double operatorFactor = 0.2;
    int operatorCounter = std::max(1, (int)(cars.size() * operatorFactor));
    while (operatorCounter > 0)
    {
        //ASSERT(cars.size() > 0);
        if (cars.size() == 0)
        {
            LOG("unsolved dead lock");
            return false;
        }
        for (auto ite = cars.begin(); ite != cars.end(); )
        {
            double rng = Random::Uniform();
            SimCar* car = *ite;
                
            if (rng < 0.2) //change path
            {
                int nextRoad = (*ite)->GetNextRoadId();
                int from = car->GetCurrentCross()->GetId();
                int to = car->GetCar()->GetToCrossId();
                auto& carTrace = car->GetTrace();
                auto& memory = m_deadLockMemory[car->GetCar()->GetId()];
                std::list<int> selections;
                for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
                {
                    Road* road = Scenario::GetCross(from)->GetRoad((Cross::DirectionType)i);
                    if (road != 0 && road->GetId() != car->GetCurrentRoad()->GetId()
                        && road->GetId() != nextRoad && memory.find(road->GetId()) == memory.end())
                    {
                        if (road->GetStartCrossId() == from ||
                            (road->GetEndCrossId() == from && road->GetIsTwoWay()))
                        {
                            selections.push_back(road->GetId());
                        }
                    }
                }
                int selected = -1;
                if (selections.size() > 0)
                {
                    if (m_selectedRoadCallback.IsNull())
                    {
                        int index = Random::Uniform(0, selections.size());
                        auto ite = selections.begin();
                        for (int i = 0; i < index; i++)
                        {
                            ++ite;
                            ASSERT(ite != selections.end());
                        }
                        selected = *ite;
                    }
                    else
                    {
                        selected = m_selectedRoadCallback.Invoke(selections, from, to);
                    }
                }
                if (selected >= 0)
                {
                    carTrace.Clear(car->GetCurrentTraceNode());
                    carTrace.AddToTail(selected);
                    --operatorCounter;
                    LOG ("reset trace of car [" << car->GetCar()->GetId() << "] ");
                    ite = cars.erase(ite);
                    //the car need go through the selected road
                    m_deadLockTraceIndexes[_a(car->GetCar()->GetId())] = car->GetCurrentTraceIndex() + 1;
                    memory.insert(selected);
                    continue;
                }
                else
                {
                    LOG ("cannot do anything for trace of car [" << car->GetCar()->GetId() << "] ");
                    ite = cars.erase(ite);
                    continue;
                }
                //else
                //{
                //    car->SetRealTime(car->GetRealTime() + 1);
                //}
            }
            //else if (rng < 0.2) //delay
            //{
            //    car->SetRealTime(time);
            //    LOG ("reset real time of car [" << car->GetCar()->GetId() << "] " << " to " << car->GetRealTime());
            //    --operatorCounter;
            //}
            ++ite;
        }
    }
        
    //retry
    m_deadLockTime = time;
    time = m_backupTime;
    scenario = m_backupScenario;
    return true;
}

bool DeadLockSolver::IsCarTraceLockedInBackup(SimCar* car) const
{
    return m_deadLockTraceIndexes[_a(car->GetCar()->GetId())] > car->GetCurrentTraceIndex();
}

bool DeadLockSolver::IsGarageLockedInBackup(const int& time) const
{
    return time < m_deadLockTime;
}

void DeadLockSolver::SetSelectedRoadCallback(const Callback::Handle3<int, const std::list<int>&, int, int>& cb)
{
    m_selectedRoadCallback = cb;
}
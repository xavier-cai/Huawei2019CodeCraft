#include "dead-lock-solver.h"
#include "scenario.h"
#include "log.h"
#include "assert.h"
#include "simulator.h"
#include "random.h"
#include <algorithm>

DeadLockSolver::DeadLockSolver()
    :  m_backupScenario(0), m_backupTime(-1), m_deadLockTime(-1), m_firstLockOnTime(-1), m_deadLockTraceIndexes(0), m_depth(0), m_actived(true), m_subSolver(0)
{ }

void DeadLockSolver::Initialize(const int& time, SimScenario& scenario)
{
    if (m_depth == 0)
    {
        m_backupScenario = m_memoryPool.Manage(new SimScenario(scenario));
        m_backupTime = time;
    }

    int size = Scenario::CalculateIndexArraySize(Scenario::Cars());
    m_deadLockTraceIndexes = m_memoryPool.NewArray<int>(size);
    for(int i = 0; i < size; ++i)
    {
        m_deadLockTraceIndexes[i] = 0;
    }
}

bool DeadLockSolver::DoHandleDeadLock(int& time, SimScenario& scenario)
{
    LOG("dead lock detected @" << time << " worker depth " << m_depth);
    m_deadLockTime = time;
    m_actived = true;
    if (time < m_deadLockTime) //backdrop dead lock
    {
        if (m_depth >= 5)
            return false; //give up
        //create another solver
        if (m_subSolver == 0)
        {
            m_subSolver = m_memoryPool.New<DeadLockSolver>();
            m_subSolver->m_depth = m_depth + 1;
            m_subSolver->Initialize(time, scenario);
            m_subSolver->SetSelectedRoadCallback(m_selectedRoadCallback);
        }
        m_actived = false;
        return m_subSolver->HandleDeadLock(time, scenario);
    }
    else if (time > m_deadLockTime) //another new dead lock
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
    int operatorCounterMax = operatorCounter;
    while (operatorCounter > 0)
    {
        //ASSERT(cars.size() > 0);
        if (cars.size() == 0)
        {
            if (operatorCounter == operatorCounterMax)
            {
                LOG("unsolved dead lock");
                return false;
            }
            LOG("uncomplete dead lock");
            return true;
        }
        for (auto ite = cars.begin(); ite != cars.end(); )
        {
            double rng = Random::Uniform();
            SimCar* car = *ite;
                
            if (rng < operatorFactor) //change path
            {
                if ((*ite)->GetCar()->GetIsPreset())
                {
                    ite = cars.erase(ite);
                    continue;
                }

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
                bool pushTrace = true;
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
                        auto result = m_selectedRoadCallback.Invoke(selections, from, car);
                        selected = result.first;
                        pushTrace = !result.second;
                    }
                }
                if (selected >= 0)
                {
                    if (pushTrace)
                    {
                        carTrace.Clear(car->GetCurrentTraceNode());
                        carTrace.AddToTail(selected);
                    }
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

    m_firstLockOnTime = -1;
    for (auto ite = scenario.Cars().begin(); ite != scenario.Cars().end(); ++ite)
    {
        if (!ite->second.GetIsInGarage() && !ite->second.GetIsReachedGoal())
            if (m_firstLockOnTime < 0 || ite->second.GetLockOnNextRoadTime() < m_firstLockOnTime)
                m_firstLockOnTime = ite->second.GetLockOnNextRoadTime();
    }
    ASSERT(m_firstLockOnTime >= 0);
    return true;
}

bool DeadLockSolver::HandleDeadLock(int& time, SimScenario& scenario)
{
    if (DoHandleDeadLock(time, scenario))
    {
        //retry
        time = m_backupTime;
        scenario = *m_backupScenario;
        return true;
    }
    return false;
}

bool DeadLockSolver::IsCarTraceLockedInBackup(SimCar* car) const
{
    if (m_actived)
    {
        return m_deadLockTraceIndexes[_a(car->GetCar()->GetId())] > car->GetCurrentTraceIndex();
    }
    ASSERT(m_subSolver != 0);
    return m_subSolver->IsCarTraceLockedInBackup(car);
}

bool DeadLockSolver::IsGarageLockedInBackup(const int& time) const
{
    if (m_actived)
    {
        return time < m_deadLockTime;
    }
    ASSERT(m_subSolver != 0);
    return m_subSolver->IsGarageLockedInBackup(time);
}

void DeadLockSolver::SetSelectedRoadCallback(const Callback::Handle3<std::pair<int, bool>, const std::list<int>&, int, SimCar*>& cb)
{
    m_selectedRoadCallback = cb;
    if (m_subSolver != 0)
        m_subSolver->SetSelectedRoadCallback(cb);
}

const int& DeadLockSolver::GetDeadLockTime() const
{
    if (m_actived)
    {
        return m_deadLockTime;
    }
    ASSERT(m_subSolver != 0);
    return m_subSolver->GetDeadLockTime();
}

bool DeadLockSolver::NeedUpdate(const int& time) const
{
    if (m_actived)
    {
        return time >= m_firstLockOnTime;
    }
    ASSERT(m_subSolver != 0);
    return m_subSolver->NeedUpdate(time);
}
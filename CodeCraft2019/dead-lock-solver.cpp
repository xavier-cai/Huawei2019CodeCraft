#include "dead-lock-solver.h"
#include "log.h"
#include "assert.h"
#include "simulator.h"
#include "random.h"
#include <algorithm>

DeadLockSolver::DeadLockSolver()
    : m_deadLockTime(-1), m_firstLockOnTime(-1), m_deadLockTraceIndexes(0), m_depth(0), m_actived(true), m_subSolver(0)
{ }

void DeadLockSolver::Initialize(const int& time, SimScenario& scenario)
{
    if (m_depth == 0)
    {
        Backup(time, scenario);
    }

    int size = Scenario::Cars().size();
    m_deadLockTraceIndexes = m_memoryPool.NewArray<int>(size);
    for(int i = 0; i < size; ++i)
    {
        m_deadLockTraceIndexes[i] = 0;
    }
}


void DeadLockSolver::OperationDelay(const int& time, SimScenario& scenario, std::list<SimCar*>& deadLockCars)
{
    std::list<SimCar*> newCars;
    for (auto ite = deadLockCars.begin(); ite != deadLockCars.end(); ++ite)
    {
        SimCar* car = *ite;
        SimCar* waitingCar = car->GetWaitingCar(time);
        ASSERT(waitingCar != 0);
        if (car->GetCurrentCross() != waitingCar->GetCurrentCross())
        {
            SimRoad* road = scenario.Roads()[waitingCar->GetCurrentRoad()->GetId()];
            for (int i = 1; i <= road->GetRoad()->GetLanes(); ++i)
            {
                auto& list = road->GetCars(i, !waitingCar->GetCurrentDirection());
                for (auto iteList = list.begin(); iteList != list.end(); ++iteList)
                {
                    SimCar* thisCar = scenario.Cars()[(*iteList)->GetId()];
                    if (!thisCar->GetCar()->GetIsPreset()
                        && thisCar->GetCurrentTraceIndex() == 1 && !thisCar->GetIsLockOnNextRoad()
                        && !thisCar->GetCar()->GetIsVip()
                        //&& thisCar.GetCar()->GetMaxSpeed() <= 4
                        )
                        newCars.push_back(thisCar);
                }
            }
        }
    }

    double operationFactor = 0.5;
    int operationNum = std::max(1, int(newCars.size() * operationFactor));
    if (newCars.size() > 0)
    {
        int counter = 0;
        int operationCounter = 0;
        double interval = newCars.size() * 1.0 / operationNum;
        for (auto ite = newCars.begin(); ite != newCars.end(); ++ite, ++counter)
        {
            if (counter > (operationCounter * interval))
            {
                int delay = Random::Uniform(0, 5);
                SimCar* car = *ite;
                car->SetRealTime(time + delay);
                m_deadLockTraceIndexes[car->GetCar()->GetId()] = 0;
                LOG("reset real time of car [" << car->GetCar()->GetId() << " to " << car->GetRealTime());
                ++operationCounter;
            }
        }
    }
}

bool DeadLockSolver::DoHandleDeadLock(int& time, SimScenario& scenario)
{
    LOG("dead lock detected @" << time << " worker depth " << m_depth);
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
        m_deadLockTime = time;
        return m_subSolver->DoHandleDeadLock(time, scenario);
    }
    else
    {
        if (time > m_deadLockTime) //another new dead lock
            m_deadLockMemory.clear(); //reset
        m_deadLockTime = time;
    }

    //remember the trace
    for(uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car == 0) continue;
        m_deadLockTraceIndexes[i] = car->GetCurrentTraceIndex();
    }

    std::list<SimCar*> cars;
    Simulator::Instance.GetDeadLockCars(time, scenario, cars);

    OperationDelay(time, scenario, cars);

    double operatorFactor = 0.5;
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
            break;
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
                std::vector<int> selections;
                for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
                {
                    Road* road = Scenario::Crosses()[from]->GetRoad((Cross::DirectionType)i);
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
                        auto result = m_selectedRoadCallback.Invoke(scenario, selections, car);
                        selected = result.first;
                        pushTrace = !result.second;
                    }
                }
                if (selected >= 0)
                {
                    if (pushTrace)
                    {
                        carTrace.Clear(car->GetCurrentTraceIndex());
                        carTrace.AddToTail(selected);
                    }
                    --operatorCounter;
                    LOG ("reset trace of car [" << car->GetCar()->GetId() << "] ");
                    ite = cars.erase(ite);
                    //the car need go through the selected road
                    m_deadLockTraceIndexes[car->GetCar()->GetId()] = car->GetCurrentTraceIndex() + 1;
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
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car != 0)
            if (!car->GetIsInGarage() && !car->GetIsReachedGoal())
                if (m_firstLockOnTime < 0 || car->GetLockOnNextRoadTime() < m_firstLockOnTime)
                    m_firstLockOnTime = car->GetLockOnNextRoadTime();
    }
    ASSERT(m_firstLockOnTime >= 0);
    return true;
}

bool DeadLockSolver::HandleDeadLock(int& time, SimScenario& scenario)
{
    if (DoHandleDeadLock(time, scenario))
    {
        //retry
        ASSERT(m_backups.size() > 0);
        ASSERT(m_backups.begin()->first == 0);
        ASSERT(m_firstLockOnTime >= 0);
        for (auto ite = --m_backups.end(); ; --ite)
        {
            if (ite->first <= m_firstLockOnTime)
            {
                time = ite->first;
                scenario = *(ite->second);
                return true;
            }
        }
        ASSERT(false);
    }
    return false;
}

bool DeadLockSolver::IsCarTraceLockedInBackup(SimCar* car) const
{
    if (m_actived)
    {
        return m_deadLockTraceIndexes[car->GetCar()->GetId()] > car->GetCurrentTraceIndex();
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

void DeadLockSolver::SetSelectedRoadCallback(const Callback::Handle3<std::pair<int, bool>, SimScenario&, const std::vector<int>&, SimCar*>& cb)
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

void DeadLockSolver::Backup(const int& time, const SimScenario& scenario)
{
    m_backups[time] = m_memoryPool.Manage(new SimScenario(scenario));
}
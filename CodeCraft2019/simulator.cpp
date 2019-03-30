#include "simulator.h"
#include "scheduler.h"
#include "scenario.h"
#include "assert.h"
#include "log.h"
#include <algorithm>

Simulator Simulator::Instance;

Simulator::Simulator()
    : m_scheduler(0), m_scheduledCarsN(0), m_reachedCarsN(0), m_conflictFlag(false)
{
    SimCar::SetUpdateStateNotifier(Callback::Create(&Simulator::HandleUpdateState, this));
}

void Simulator::SetScheduler(Scheduler* scheduler)
{
    m_scheduler = scheduler;
}

void Simulator::HandleUpdateState(const SimCar::SimState& state)
{
    m_conflictFlag = false;
    if(state == SimCar::SCHEDULED)
        ++m_scheduledCarsN;
}

void Simulator::NotifyFirstPriority(const int& time, SimScenario& scenario, SimCar* car) const
{
    if (!car->GetIsLockOnNextRoad())
    {
        int length = std::min(car->GetCurrentRoad()->GetLimit(), car->GetCar()->GetMaxSpeed());
        if (car->GetCurrentPosition() + length <= car->GetCurrentRoad()->GetLength()) //can not pass
            return;
        if (m_scheduler != 0)
            m_scheduler->HandleBecomeFirstPriority(time, scenario, car);
        car->LockOnNextRoad();
    }
}

void Simulator::NotifyScheduleStart()
{
    m_reachedCarsN = 0;
    m_scheduledCarsN = 0;
}

void Simulator::NotifyScheduleCycleStart()
{
    m_conflictFlag = true;
}

bool Simulator::GetIsDeadLock(SimScenario& scenario) const
{
    return m_conflictFlag && scenario.GetOnRoadCarsN() > 0;
}

bool Simulator::GetIsCompleted(SimScenario& scenario) const
{
    return (m_scheduledCarsN - m_reachedCarsN) == scenario.GetOnRoadCarsN();
}

int Simulator::GetPositionInNextRoad(const int& time, SimScenario& scenario, SimCar* car) const
{
    int currentLimit = std::min(car->GetCurrentRoad()->GetLimit(), car->GetCar()->GetMaxSpeed());
    int s1 = std::min(currentLimit, car->GetCurrentRoad()->GetLength() - car->GetCurrentPosition());
    if (car->GetCurrentPosition() + currentLimit <= car->GetCurrentRoad()->GetLength()) //can not reach the next road
        return 0;
    int maxS2 = car->GetCar()->GetMaxSpeed() - s1;
    if (car->GetCurrentCross()->GetId() == car->GetCar()->GetToCrossId()) //reach goal
        return maxS2;
    
    /*
    //check if may pass the cross
    {
        bool mayPass = false;
        DirectionType_Foreach(dir,
            Road* road = car->GetCurrentCross()->GetRoad(dir);
            if (road != 0 && road->GetId() != car->GetCurrentRoad()->GetId() && road->GetLimit() > s1)
            {
                mayPass = true;
                break;
            }
        );
        if (!mayPass)
            return 0;
    }
    */
    
    NotifyFirstPriority(time, scenario, car);
    ASSERT(car->GetNextRoadId() >= 0);
    int nextLimit = std::min(car->GetCar()->GetMaxSpeed(), Scenario::GetRoad(car->GetNextRoadId())->GetLimit());
    int s2 = std::min(maxS2, nextLimit - s1);
    if (s2 <= 0)
        return 0;
    return s2;
}

//car on first priority
SimCar* Simulator::PeekFirstPriorityCarOnRoad(const int& time, SimScenario& scenario, SimRoad* road, const int& crossId) const
{
    SimCar* ret = 0;
    int position = road->GetRoad()->GetLength() + 1;
    for (int i = 1; i <= road->GetRoad()->GetLanes(); ++i)
    {
        auto& list = road->GetCarsTo(i, crossId);
        if (list.size() > 0)
        {
            SimCar* car = &scenario.Cars()[(*list.begin())->GetId()];
            if (car->GetSimState(time) != SimCar::SCHEDULED && !car->GetIsIgnored())
            {
                int limit = std::min(car->GetCar()->GetMaxSpeed(), road->GetRoad()->GetLimit());
                ASSERT(car->GetCurrentPosition() + limit > road->GetRoad()->GetLength());//first priority
                if (ret == 0 || car->GetCurrentPosition() > position)
                {
                    ret = car;
                    position = car->GetCurrentPosition();
                }
            }
        }
    }
    if (ret != 0)
    {
        GetPositionInNextRoad(time, scenario, ret);//for notify first priority if needed
    }
    return ret;
}

//for checking means only the car which will pass the cross
SimCar* Simulator::CheckFirstPriorityCarOnRoad(const int& time, SimScenario& scenario, SimRoad* road, const int& crossId) const
{
    SimCar* ret = 0;
    int position = road->GetRoad()->GetLength() + 1;
    for (int i = 1; i <= road->GetRoad()->GetLanes(); ++i)
    {
        auto& list = road->GetCarsTo(i, crossId);
        if (list.size() > 0)
        {
            SimCar* car = &scenario.Cars()[(*list.begin())->GetId()];
            if (car->GetSimState(time) != SimCar::SCHEDULED)
            {
                /* only care the first priority which may pass the cross */
                int limit = std::min(car->GetCar()->GetMaxSpeed(), road->GetRoad()->GetLimit());
                if (car->GetCurrentPosition() + limit > road->GetRoad()->GetLength()) //may pass cross
                //if (GetPositionInNextRoad(time, scenario, car) > 0) //will pass cross
                {
                    if (ret == 0 || car->GetCurrentPosition() > position)
                    {
                        ret = car;
                        position = car->GetCurrentPosition();
                    }
                }
            }
        }
    }
    if (ret != 0)
    {
        GetPositionInNextRoad(time, scenario, ret); //update the road lock if needed
        //NotifyFirstPriority(time, scenario, ret);
    }
    return ret;
}

//pass cross or just forward, return [true] means scheduled; [false] means waiting, require the car is the first one on its lane
bool Simulator::PassCrossOrJustForward(const int& time, SimScenario& scenario, SimCar* car)
{
    SimRoad* road = &scenario.Roads()[car->GetCurrentRoad()->GetId()];
    ///logic moved
    Cross* cross = car->GetCurrentCross();
    auto& carlist = road->GetCarsTo(car->GetCurrentLane(), cross->GetId());
    ASSERT(car->GetCar() == *carlist.begin());
    int s2 = GetPositionInNextRoad(time, scenario, car);
    int nextRoadId = car->GetNextRoadId();
    if (nextRoadId == -1) //reach goal
    {
        ASSERT(s2 > 0);
        road->RunOut(car->GetCurrentLane(), !car->GetCurrentDirection());
        car->UpdateReachGoal(time);
        scenario.ReachGoal();
        ++m_reachedCarsN;
        return true;
    }
    
    //try pass cross
    SimRoad* nextRoad = &scenario.Roads()[nextRoadId];
    bool isFromOrTo = nextRoad->IsFromOrTo(cross->GetId());
    auto direction = cross->GetTurnDirection(road->GetRoad()->GetId(), nextRoadId);
    if (direction != Cross::DIRECT) //check DIRECT priority
    {
        Road* peer = cross->GetTurnDestination(nextRoadId, !Cross::DIRECT);
        if(peer != 0 && peer->CanReachTo(cross->GetId()))
        {
            SimCar* checkCar = Simulator::CheckFirstPriorityCarOnRoad(time, scenario, &scenario.Roads()[peer->GetId()], cross->GetId());
            if (checkCar != 0 && (checkCar->GetNextRoadId() == nextRoadId || checkCar->GetCar()->GetToCrossId() == cross->GetId())) //direct or reach goal
            {
                car->UpdateWaiting(time, checkCar);
                return false;
            }
        }
    }
    if (direction == Cross::RIGHT) //check LEFT priority
    {
        Road* peer = cross->GetTurnDestination(nextRoadId, !Cross::LEFT);
        if(peer != 0 && peer->CanReachTo(cross->GetId()))
        {
            SimCar* checkCar = Simulator::CheckFirstPriorityCarOnRoad(time, scenario, &scenario.Roads()[peer->GetId()], cross->GetId());
            if (checkCar != 0 && checkCar->GetNextRoadId() == nextRoadId)
            {
                car->UpdateWaiting(time, checkCar);
                return false;
            }
        }
    }

    int nextPosition = GetPositionInNextRoad(time, scenario, car);
    if (nextPosition <= 0) //just forward
    {
        //ASSERT(false);//the car which can not pass the cross should forward in another function
        int maxLength = std::min(car->GetCar()->GetMaxSpeed(), road->GetRoad()->GetLimit());
        int newPosition = std::min(car->GetCurrentPosition() + maxLength, road->GetRoad()->GetLength());
        ASSERT(newPosition == road->GetRoad()->GetLength());//the car which may pass the cross but can not pass it will stop at the head
        car->UpdatePosition(time, newPosition);
        return true;
    }

    //it's time to pass the cross!
    bool updatedState = false;
    for (int i = 1; i <= nextRoad->GetRoad()->GetLanes(); ++i)
    {
        auto& inlist = nextRoad->GetCarsFrom(i, cross->GetId());
        SimCar* lastcar = 0;
        if (inlist.size() > 0)
            lastcar = &scenario.Cars()[(*--inlist.end())->GetId()];
        //need wait
        if (lastcar != 0 && lastcar->GetCurrentPosition() <= nextPosition && lastcar->GetSimState(time) != SimCar::SCHEDULED)
        {
            car->UpdateWaiting(time, lastcar);
            updatedState = true;
            break;
        }
        //pass the cross
        if (lastcar == 0 || lastcar->GetCurrentPosition() != 1)
        {
            //go on the new road & remove from old road
            nextRoad->RunIn(road->RunOut(car->GetCurrentLane(), !car->GetCurrentDirection()), i, !isFromOrTo);
            int newPosition = lastcar == 0 ? nextPosition : std::min(lastcar->GetCurrentPosition() - 1, nextPosition);
            car->UpdateOnRoad(time, nextRoad->GetRoad(), i, isFromOrTo, newPosition);
            updatedState = true;
            //break;
            return true;
        }
        //try next lane...
    }
    if (!updatedState) //this means no empty place for this car in next road
    {
        car->UpdatePosition(time, road->GetRoad()->GetLength()); //just move forward
        return true;
    }
    return false;
}

void UpdateCarsInLane(const int& time, SimScenario& scenario, SimRoad* &road, const int& lane, const bool& opposite, const bool& canBreak)
{
    auto& cars = opposite ? road->GetCarsOpposite(lane) : road->GetCars(lane);
    SimCar* frontCar = 0;
    for (auto carIte = cars.begin(); carIte != cars.end(); ++carIte)
    {
        SimCar* car = &scenario.Cars()[(*carIte)->GetId()];
        SimCar::SimState state = car->GetSimState(time);
        if (car->GetIsIgnored())
            break;
        if (state != SimCar::SCHEDULED)
        {
            int speed = std::min(car->GetCurrentRoad()->GetLimit(), car->GetCar()->GetMaxSpeed());
            int nexPosition = car->GetCurrentPosition() + speed;
            if (carIte == cars.begin())
            {
                ASSERT(frontCar == 0);
                if (nexPosition <= car->GetCurrentRoad()->GetLength()) //have no possible passing cross
                //if (Simulator::GetPositionInNextRoad(time, scenario, car) <= 0) //will not pass cross
                {
                    car->UpdatePosition(time, std::min(nexPosition, road->GetRoad()->GetLength()));
                }
                else //may pass cross
                {
                    if (canBreak)
                        break;
                }
            }
            else
            {
                ASSERT(frontCar != 0);
                int frontPosition = frontCar->GetCurrentPosition();
                if (frontCar->GetSimState(time) != SimCar::SCHEDULED && nexPosition >= frontPosition) //need wait
                {
                    car->UpdateWaiting(time, frontCar);
                    if (canBreak)
                        break;
                }
                else
                {
                    car->UpdatePosition(time, std::min(nexPosition, frontPosition - 1));
                }
            }
        }
        frontCar = car;
    }
}

void UpdateCarsInRoad(const int& time, SimScenario& scenario, SimRoad* road)
{
    int lanes = road->GetRoad()->GetLanes();
    for (int i = 0; i < lanes * 2; ++i)
    {
        bool opposite = i >= lanes;
        if (opposite && !road->GetRoad()->GetIsTwoWay())
            break;
        UpdateCarsInLane(time, scenario, road, (i % lanes) + 1, opposite, false);
    }
}

Simulator::UpdateResult Simulator::Update(const int& time, SimScenario& scenario)
{
    Simulator::UpdateResult result;
    result.Conflict = false;

    NotifyScheduleStart();
    for (auto roadIte = scenario.Roads().begin(); roadIte != scenario.Roads().end(); ++roadIte)
    {
        SimRoad* road = &roadIte->second;
        UpdateCarsInRoad(time, scenario, road);
    }
    std::list<Cross*> crosses;
    for (auto ite = Scenario::Crosses().begin(); ite != Scenario::Crosses().end(); ++ite)
        crosses.push_back(ite->second);
    while(crosses.size() > 0)
    {
        NotifyScheduleCycleStart();
        for (auto crossIte = crosses.begin(); crossIte != crosses.end();)
        {
            int crossId = (*crossIte)->GetId();
            std::map<int, SimRoad*> roads; //roads in this cross
            DirectionType_Foreach(dir, 
                int id = (*crossIte)->GetRoadId(dir);
                if (id >= 0)
                {
                    SimRoad* road = &scenario.Roads()[id];
                    if (road->GetRoad()->GetEndCrossId() == crossId ||
                        (road->GetRoad()->GetStartCrossId() == crossId && road->GetRoad()->GetIsTwoWay()))
                        roads[id] = road;
                }
            );
            //while (roads.size() > 0)
            {
                //bool crossConflict = true;
                for (auto roadIte = roads.begin(); roadIte != roads.end();)
                {
                    SimRoad* road = roadIte->second;
                    //try pass cross (only the first priority can pass the cross)
                    SimCar* firstPriority = 0;
                    while((firstPriority = Simulator::PeekFirstPriorityCarOnRoad(time, scenario, road, crossId)) != 0)
                    {
                        int lane = firstPriority->GetCurrentLane();
                        bool opposite = !firstPriority->GetCurrentDirection();
                        if (!Simulator::PassCrossOrJustForward(time, scenario, firstPriority))
                        {
                            ASSERT (firstPriority->GetSimState(time) == SimCar::WAITING && firstPriority->GetWaitingCar(time) != 0);
                            break;
                        }
                        //UpdateCarsInRoad(time, scenario, road);
                        UpdateCarsInLane(time, scenario, road, lane, opposite, true);
                        //crossConflict = false;
                    }
                    if (firstPriority == 0) //complete
                        roadIte = roads.erase(roadIte);
                    else
                        ++roadIte;
                }
                //if(crossConflict)
                    //break;
            }
            if(roads.size() == 0)
                crossIte = crosses.erase(crossIte);
            else
                ++crossIte;
        }
        if (GetIsCompleted(scenario)) //complete
        {
            result.Conflict = false;
            break;
        }
        if (GetIsDeadLock(scenario)) //conflict
        {
            PrintDeadLock(time, scenario);
            result.Conflict = true;
            break;
        }
    }
    if (!result.Conflict)
    {
        GetOutFromGarage(time, scenario);
    }
    //ASSERT(result.Conflict || scheduledCarsN - reachedCarsN == scenario.GetOnRoadCarsN());
    return result;
}

void Simulator::GetOutFromGarage(const int& time, SimScenario& scenario) const
{
    int getOutCounter = 0;
    //cars in garage
    for (auto ite = Scenario::Crosses().begin(); ite != Scenario::Crosses().end(); ++ite)
    {
        int crossId = ite->second->GetId();
        auto findGarage = scenario.Garages().find(crossId);
        if (findGarage != scenario.Garages().end()) //find it!
        {
            std::map< int, std::vector<std::list<SimCar*>::iterator> > cangoCars;
            for (auto garageCarIte = findGarage->second.begin(); garageCarIte != findGarage->second.end(); ++garageCarIte)
            {
                SimCar* car = *garageCarIte;
                if (car->GetRealTime() <= time) //can get out
                    cangoCars[car->GetRealTime()].push_back(garageCarIte);
            }
            for (auto cangoTimeIte = cangoCars.begin(); cangoTimeIte != cangoCars.end(); ++cangoTimeIte)
            {
                const int& cangoTime = cangoTimeIte->first;
                for (auto cangoCarIte = cangoTimeIte->second.begin(); cangoCarIte != cangoTimeIte->second.end(); ++cangoCarIte)
                {
                    SimCar* car = **cangoCarIte;
                    if (m_scheduler != 0)
                        m_scheduler->HandleGetoutGarage(time, scenario, car);
                    ASSERT(car->GetRealTime() == cangoTime || car->GetRealTime() > time);
                    if (car->GetRealTime() > time)
                        continue;

                    /* it's time to go! */
                    bool goout = false;
                    int roadId = car->GetNextRoadId();
                    ASSERT(roadId >= 0);
                    SimRoad* road = &scenario.Roads()[roadId];
                    int maxLength = std::min(car->GetCar()->GetMaxSpeed(), road->GetRoad()->GetLimit());
                    for (int i = 1; i <= road->GetRoad()->GetLanes(); ++i)
                    {
                        auto& cars = road->GetCarsFrom(i, crossId);
                        if (cars.size() > 0)
                        {
                            SimCar* lastCar = &scenario.Cars()[(*--cars.end())->GetId()];
                            ASSERT(lastCar->GetSimState(time) == SimCar::SCHEDULED);
                            ASSERT(lastCar->GetCurrentPosition() > 0);
                            if (lastCar->GetCurrentPosition() == 1)
                                continue;
                            maxLength = std::min(maxLength, lastCar->GetCurrentPosition() - 1);
                        }
                        bool isFromOrTo = road->IsFromOrTo(crossId);
                        car->UpdateOnRoad(time, road->GetRoad(), i, isFromOrTo, maxLength);
                        road->RunIn(car->GetCar(), i, !isFromOrTo);
                        car->SetRealTime(time);
                        goout = true;
                        break;
                    }
                    if (!goout)
                    {
                        car->UpdateStayInGarage(time);
                        car->SetRealTime(time + 1); //cheater
                    }
                    else //go out
                    {
                        findGarage->second.erase(*cangoCarIte);
                        ++getOutCounter;
                        scenario.GetoutOnRoad();
                    }
                }
            }
            if (findGarage->second.size() == 0) //no car in garage
                scenario.Garages().erase(findGarage);
        }
    }
    if (getOutCounter > 0)
        LOG("@" << time << " number of cars get on road form garage : " << getOutCounter);
}

void Simulator::GetDeadLockCars(const int& time, SimScenario& scenario, std::list<SimCar*>& result, int n) const
{
    for (auto crossIte = Scenario::Crosses().begin(); crossIte != Scenario::Crosses().end(); ++crossIte)
    {
        int crossId = crossIte->second->GetId();
        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; ++i)
        {
            int id = crossIte->second->GetRoadId((Cross::DirectionType)i);
            if (id >= 0)
            {
                SimRoad* road = &scenario.Roads()[id];
                if (road->GetRoad()->GetEndCrossId() == crossId ||
                    (road->GetRoad()->GetStartCrossId() == crossId && road->GetRoad()->GetIsTwoWay()))
                {
                    SimCar* firstPriority = PeekFirstPriorityCarOnRoad(time, scenario, road, crossId);
                    if (firstPriority != 0 && firstPriority->GetSimState(time) != SimCar::SCHEDULED)
                    {
                        ASSERT(firstPriority->GetSimState(time) == SimCar::WAITING);
                        //if (firstPriority->GetWaitingCar(time)->GetCurrentCross()->GetId() != crossId)
                        {
                            result.push_back(firstPriority);
                            if (n > 0)
                                if (--n <= 0)
                                    return;
                        }
                    }
                }
            }
        }
    }
}

void Simulator::PrintCrossState(const int& time, SimScenario& scenario, Cross* cross) const
{
    int crossId = cross->GetId();
    for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; ++i)
    {
        int id = cross->GetRoadId((Cross::DirectionType)i);
        if (id >= 0)
        {
            SimRoad* road = &scenario.Roads()[id];
            if (road->GetRoad()->GetEndCrossId() == crossId ||
                (road->GetRoad()->GetStartCrossId() == crossId && road->GetRoad()->GetIsTwoWay()))
            {
                LOG("Road [" << road->GetRoad()->GetId() << "] direction : " << i << " lanes " << road->GetRoad()->GetLanes() << " limit " << road->GetRoad()->GetLimit());
                for (int j = 1; j <= road->GetRoad()->GetLanes(); ++j)
                {
                    LOG("\tLane " << j);
                    auto& cars = road->GetCarsTo(j, crossId);
                    for (auto carIte = cars.begin(); carIte != cars.end(); ++carIte)
                    {
                        SimCar* car = &scenario.Cars()[(*carIte)->GetId()];
                        if (car->GetSimState(time) == SimCar::SCHEDULED || GetPositionInNextRoad(time, scenario, car) <= 0)
                            break;
                        LOG("car [" << car->GetCar()->GetId() << "]"
                            << " pos " << car->GetCurrentPosition()
                            << " speed " << car->GetCar()->GetMaxSpeed()
                            << " dir " << cross->GetTurnDirection(road->GetRoad()->GetId(), car->GetNextRoadId())
                            );
                    }
                }
            }
        }
    }
}

void Simulator::PrintDeadLock(const int& time, SimScenario& scenario) const
{
    if (LOG_IS_ENABLE)
    {
        std::list<SimCar*> result;
        GetDeadLockCars(time, scenario, result);
        for (auto ite = result.begin(); ite != result.end(); ite++)
        {
            SimCar*& car = *ite;
            Cross* cross = car->GetCurrentCross();
            Road* road = car->GetCurrentRoad();
            LOG("id " << car->GetCar()->GetId()
                << " cross " << cross->GetId()
                << " road " << road->GetId()
                << " lane " << car->GetCurrentLane()
                << " position " << car->GetCurrentPosition()
                << " next " << car->GetNextRoadId()
                << " dir " << (car->GetNextRoadId() < 0 ? Cross::DIRECT : cross->GetTurnDirection(road->GetId(), car->GetNextRoadId()))
                << " waiting for " << car->GetWaitingCar(time)->GetCar()->GetId()
                );
        }
    }
}
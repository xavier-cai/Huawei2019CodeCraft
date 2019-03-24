#include "simulator.h"

#ifndef SIMULATOR_CPP
#define SIMULATOR_CPP
bool Simulator::ConflictFlag = false;
int Simulator::ScheduledCarsN = 0;
int Simulator::ReachedCarsN = 0;
void NotifyUpdateStateHandler(const SimCar::SimState& state)
{
    Simulator::ConflictFlag = false;
    if(state == SimCar::SCHEDULED)
        Simulator::ScheduledCarsN++;
}
#endif //#ifndef SIMULATOR_CPP

#include "scheduler.h"
Scheduler* Simulator::SchedulerPtr = 0;

Simulator Simulator::InstanceAgent;

Simulator::Simulator()
    : m_scenario(*static_cast<SimScenario*>(0))
{ }

Simulator::Simulator(const int& time, SimScenario& scenario)
    : m_time(time), m_scenario(scenario)
{
    SimCar::SetUpdateStateNotifier(&NotifyUpdateStateHandler);
}

Simulator::UpdateResult Simulator::UpdateSelf(const int& time, SimScenario& scenario)
{
    Simulator sim(time, scenario);
    return sim.UpdateSelf();
}



#include "pmap.h"
#include <algorithm>
#include <set>
#include "scenario.h"
#include "assert.h"
#include "log.h"

template <typename _T>
std::_List_iterator<_T> operator + (const std::_List_iterator<_T>& ite, int n)
{
    auto copy = ite;
    bool opposite = n < 0;
    if (opposite) n = -n;
    while(n-- > 0)
        opposite ? copy-- : copy++;
    return copy;
}

template <typename _T>
std::_List_iterator<_T> operator - (const std::_List_iterator<_T>& ite, int n)
{
    return ite + -n;
}

std::ostream& operator << (std::ostream& os, const Simulator::TrunType& turn)
{
    switch (turn)
    {
    case Simulator::LEFT: os << "Left"; break;
    case Simulator::DIRECT: os << "Direct"; break;
    case Simulator::RIGHT: os << "Right"; break;
    default: ASSERT(false); break;
    }
    return os;
}

std::pmap< Simulator::TrunType, int, int> directions;
std::pmap< int, int, Simulator::TrunType> roadPeers;
std::pmap< int, int, Simulator::TrunType> roadPeersOpposite;
void InitializeDirections()
{
    directions.clear();
    roadPeers.clear();
    roadPeersOpposite.clear();
    for (auto crossIte = Scenario::Crosses().begin(); crossIte != Scenario::Crosses().end(); crossIte++)
    {
        Cross* cross = crossIte->second;
        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
        {
            Road* from = cross->GetRoad((Cross::DirectionType)i);
            if (from != 0 && from->GetId() >= 0)
            {
                for (int j = (int)Simulator::LEFT; j <= (int)Simulator::RIGHT; j++)
                {
                    Road* to = cross->GetRoad((Cross::DirectionType)((j + i + 1) % ((int)Cross::WEST + 1)));
                    if (to != 0 && to->GetId() >= 0)
                    {
                        if ((from->GetEndCrossId() == cross->GetId() ||
                            (from->GetStartCrossId() == cross->GetId() && from->GetIsTwoWay()))
                            &&
                            (to->GetStartCrossId() == cross->GetId() ||
                            (to->GetEndCrossId() == cross->GetId() && to->GetIsTwoWay())))
                        {
                            directions[std::make_pair(from->GetId(), to->GetId())] = (Simulator::TrunType)j;
                            bool isFromOrTo = to->GetStartCrossId() == cross->GetId();
                            ASSERT(isFromOrTo || to->GetEndCrossId() == cross->GetId());
                            (isFromOrTo ? roadPeers : roadPeersOpposite)[std::make_pair(to->GetId(), (Simulator::TrunType)j)] = from->GetId();
                        }
                    }
                }
            }
        }
    }
}

void Simulator::Initialize()
{
    InitializeDirections();
    SimCar::SetUpdateStateNotifier(&NotifyUpdateStateHandler);
}

Simulator::TrunType Simulator::GetDirection(int from, int to)
{
    auto find = directions.find(std::make_pair(from, to));
    ASSERT(find != directions.end());
    return find->second;
}
int Simulator::GetRoadPeer(int to, Simulator::TrunType dir, bool opposite)
{
    auto& map = (opposite ? roadPeersOpposite : roadPeers);
    auto find = map.find(std::make_pair(to, dir));
    if (find == map.end())
        return -1;
    return find->second;
}

void Simulator::NotifyFirstPriority(const int& time, SimScenario& scenario, SimCar* car)
{
    if (!car->GetIsLockOnNextRoad())
    {
        int length = std::min(car->GetCurrentRoad()->GetLimit(), car->GetCar()->GetMaxSpeed());
        if (car->GetCurrentPosition() + length <= car->GetCurrentRoad()->GetLength()) //can not pass
            return;
        if (Simulator::SchedulerPtr != 0)
            Simulator::SchedulerPtr->HandleBecomeFirstPriority(time, scenario, car);
        car->LockOnNextRoad();
    }
}

int Simulator::GetPositionInNextRoad(const int& time, SimScenario& scenario, SimCar* car)
{
    int currentLimit = std::min(car->GetCurrentRoad()->GetLimit(), car->GetCar()->GetMaxSpeed());
    int s1 = std::min(currentLimit, car->GetCurrentRoad()->GetLength() - car->GetCurrentPosition());
    if (car->GetCurrentPosition() + currentLimit <= car->GetCurrentRoad()->GetLength()) //can not reach the next road
        return 0;
    int maxS2 = car->GetCar()->GetMaxSpeed() - s1;
    if (car->GetCurrentCross()->GetId() == car->GetCar()->GetToCrossId() || car->GetNextRoadId() < 0) //reach goal
        return maxS2;
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
    NotifyFirstPriority(time, scenario, car);
    int nextLimit = std::min(car->GetCar()->GetMaxSpeed(), Scenario::GetRoad(car->GetNextRoadId())->GetLimit());
    int s2 = std::min(maxS2, nextLimit - s1);
    if (s2 <= 0)
        return 0;
    return s2;
}

//car on first priority
SimCar* Simulator::PeekFirstPriorityCarOnRoad(const int& time, SimScenario& scenario, SimRoad* road, const int& crossId)
{
    SimCar* ret = 0;
    int position = road->GetRoad()->GetLength() + 1;
    for (int i = 1; i <= road->GetRoad()->GetLanes(); i++)
    {
        auto& list = road->GetCarsTo(i, crossId);
        if (list.size() > 0)
        {
            SimCar* car = &scenario.Cars()[(*list.begin())->GetId()];
            if (car->GetSimState(time) != SimCar::SCHEDULED &&
                !(car->GetSimState(time) == SimCar::WAITING && car->GetWaitingCar(time) == car))
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
SimCar* Simulator::CheckFirstPriorityCarOnRoad(const int& time, SimScenario& scenario, SimRoad* road, const int& crossId)
{
    SimCar* ret = 0;
    int position = road->GetRoad()->GetLength() + 1;
    for (int i = 1; i <= road->GetRoad()->GetLanes(); i++)
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
    int nextRoadId = car->GetNextRoadId();
    if (nextRoadId == -1) //reach goal
    {
        carlist.erase(carlist.begin());
        car->UpdateReachGoal(time);
        scenario.ReachGoal();
        Simulator::ReachedCarsN++;
        return true;
    }
    
    //try pass cross
    SimRoad* nextRoad = &scenario.Roads()[nextRoadId];
    bool isFromOrTo = nextRoad->IsFromOrTo(cross->GetId());
    auto direction = Simulator::GetDirection(road->GetRoad()->GetId(), nextRoadId);
    if (direction != Simulator::DIRECT) //check DIRECT priority
    {
        int peer = Simulator::GetRoadPeer(nextRoadId, Simulator::DIRECT, !isFromOrTo);
        if(peer >= 0)
        {
            SimCar* checkCar = Simulator::CheckFirstPriorityCarOnRoad(time, scenario, &scenario.Roads()[peer], cross->GetId());
            if (checkCar != 0 && (checkCar->GetNextRoadId() == nextRoadId || checkCar->GetCar()->GetToCrossId() == cross->GetId())) //direct or reach goal
            {
                car->UpdateWaiting(time, checkCar);
                return false;
            }
        }
    }
    if (direction == Simulator::RIGHT) //check LEFT priority
    {
        int peer = Simulator::GetRoadPeer(nextRoadId, Simulator::LEFT, !isFromOrTo);
        if(peer >= 0)
        {
            SimCar* checkCar = Simulator::CheckFirstPriorityCarOnRoad(time, scenario, &scenario.Roads()[peer], cross->GetId());
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
    for (int i = 1; i <= nextRoad->GetRoad()->GetLanes(); i++)
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
            //go on the new road
            inlist.push_back(car->GetCar());
            int newPosition = lastcar == 0 ? nextPosition : std::min(lastcar->GetCurrentPosition() - 1, nextPosition);
            car->UpdateOnRoad(time, nextRoad->GetRoad(), i, isFromOrTo, newPosition);
            updatedState = true;
            break;
        }
        //try next lane...
    }
    if (!updatedState) //this means no empty place for this car in next road
    {
        car->UpdatePosition(time, road->GetRoad()->GetLength()); //just move forward
        return true;
    }
    else
    {   //scheduled means passed the cross, so...remove it!
        if (car->GetSimState(time) == SimCar::SCHEDULED)
        {
            carlist.erase(carlist.begin());
            return true;
        }
    }
    return false;
}

Simulator::UpdateResult Simulator::UpdateSelf() const
{
    int time = m_time;
    SimScenario& scenario = m_scenario;

    UpdateResult result;
    result.Conflict = false;
    std::list<Cross*> crosses;
    for (auto ite = Scenario::Crosses().begin(); ite != Scenario::Crosses().end(); ite++)
        crosses.push_back(ite->second);
    Simulator::ScheduledCarsN = 0; //counter
    Simulator::ReachedCarsN = 0; //counter
    while(crosses.size() > 0)
    {
        Simulator::ConflictFlag = true; //for checking conflict
        for (auto crossIte = crosses.begin(); crossIte != crosses.end(); /* crossIte++ */)
        {
            int crossId = (*crossIte)->GetId();
            std::map<int, SimRoad*> roads; //roads in this cross
            for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
            {
                int id = (*crossIte)->GetRoadId((Cross::DirectionType)i);
                if (id >= 0)
                {
                    SimRoad* road = &scenario.Roads()[id];
                    if (road->GetRoad()->GetEndCrossId() == crossId ||
                        (road->GetRoad()->GetStartCrossId() == crossId && road->GetRoad()->GetIsTwoWay()))
                        roads[id] = road;
                }
            }
            while(roads.size() > 0)
            {
                bool crossConflict = true;
                for (auto roadIte = roads.begin(); roadIte != roads.end(); /* roadIte++ */)
                {
                    SimRoad* road = roadIte->second;
                    //try pass cross (only the first priority can pass the cross)
                    SimCar* firstPriority = 0;
                    while((firstPriority = PeekFirstPriorityCarOnRoad(time, scenario, road, crossId)) != 0)
                    {
                        //auto state = firstPriority->GetSimState(time);
                        //if (state == SimCar::WAITING && firstPriority->GetWaitingCar(time)->GetSimState(time) != SimCar::SCHEDULED)
                        //    break;
                        if (!PassCrossOrJustForward(time, scenario, firstPriority))
                            break;
                        crossConflict = false; //conflict only occurs on the first priority car
                    }
                    //try move forward other cars
                    for (int i = 1; i <= road->GetRoad()->GetLanes(); i++)
                    {
                        auto& cars = road->GetCarsTo(i, crossId);
                        if (cars.size() > 0) //any car on this lane
                        {
                            //update the first car
                            SimCar* firstCar = &scenario.Cars()[(*cars.begin())->GetId()];
                            bool firstComplete = firstCar->GetSimState(time) == SimCar::SCHEDULED;
                            if (!firstComplete && firstCar != firstPriority)
                            {
                                ASSERT(firstPriority != 0);
                                int maxLength = std::min(firstCar->GetCar()->GetMaxSpeed(), road->GetRoad()->GetLimit());
                                if(GetPositionInNextRoad(time, scenario, firstCar) > 0) //will pass the cross
                                    firstCar->UpdateWaiting(time, firstPriority);
                                else
                                    firstCar->UpdatePosition(time, firstCar->GetCurrentPosition() + maxLength);
                            }
                            //update cars behind the first car
                            
                            for (auto carIte = cars.begin() + 1; carIte != cars.end(); carIte++)
                            {
                                SimCar* simCar = &scenario.Cars()[(*carIte)->GetId()];
                                SimCar* frontCar = &scenario.Cars()[(*(carIte - 1))->GetId()];
                                if (simCar->GetSimState(time) == SimCar::SCHEDULED)
                                    break;
                                //if (simCar->GetSimState(time) == SimCar::WAITING && simCar->GetWaitingCar(time)->GetSimState(time) != SimCar::SCHEDULED)
                                //    continue;
                                int maxLength = std::min(simCar->GetCar()->GetMaxSpeed(), road->GetRoad()->GetLimit());
                                int frontLength = frontCar->GetCurrentPosition() - simCar->GetCurrentPosition();
                                ASSERT(frontLength > 0);
                                if (frontCar->GetSimState(time) != SimCar::SCHEDULED && maxLength >= frontLength) //need wait the front car
                                {
                                    simCar->UpdateWaiting(time, frontCar);
                                    continue;
                                }
                                maxLength = std::min(maxLength, frontLength - 1);
                                simCar->UpdatePosition(time, simCar->GetCurrentPosition() + maxLength);
                            }
                        }
                    }
                    if (firstPriority == 0)
                        roadIte = roads.erase(roadIte);
                    else
                        roadIte++;
                }
                if (crossConflict)
                    break;
            }
            //cross schedule complete or not
            crossIte = roads.size() == 0 ? crosses.erase(crossIte) : (crossIte + 1);
        }
        if(Simulator::ScheduledCarsN - Simulator::ReachedCarsN == scenario.GetOnRoadCarsN())
            break;
        if (Simulator::ConflictFlag && scenario.GetOnRoadCarsN() > 0)
        {
            PrintDeadLock(time, scenario);
            result.Conflict = true;
            break;
        }
    }
    
    if(!result.Conflict)
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
    for (auto ite = Scenario::Crosses().begin(); ite != Scenario::Crosses().end(); ite++)
    {
        int crossId = ite->second->GetId();
        auto findGarage = scenario.Garages().find(crossId);
        if (findGarage != scenario.Garages().end()) //find it!
        {
            std::map< int, std::vector<std::list<SimCar*>::iterator> > cangoCars;
            for (auto garageCarIte = findGarage->second.begin(); garageCarIte != findGarage->second.end(); garageCarIte++)
            {
                SimCar* car = *garageCarIte;
                if (car->GetRealTime() <= time) //can get out
                    cangoCars[car->GetRealTime()].push_back(garageCarIte);
            }
            for (auto cangoTimeIte = cangoCars.begin(); cangoTimeIte != cangoCars.end(); cangoTimeIte++)
            {
                const int& cangoTime = cangoTimeIte->first;
                for (auto cangoCarIte = cangoTimeIte->second.begin(); cangoCarIte != cangoTimeIte->second.end(); cangoCarIte++)
                {
                    SimCar* car = **cangoCarIte;
                    if (Simulator::SchedulerPtr != 0)
                        Simulator::SchedulerPtr->HandleGetoutGarage(time, scenario, car);
                    ASSERT(car->GetRealTime() == cangoTime || car->GetRealTime() > time);
                    if (car->GetRealTime() > time)
                        continue;

                    /* it's time to go! */
                    bool goout = false;
                    int roadId = car->GetNextRoadId();
                    ASSERT(roadId >= 0);
                    SimRoad* road = &scenario.Roads()[roadId];
                    int maxLength = std::min(car->GetCar()->GetMaxSpeed(), road->GetRoad()->GetLimit());
                    for (int i = 1; i <= road->GetRoad()->GetLanes(); i++)
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
                        car->UpdateOnRoad(time, road->GetRoad(), i, road->IsFromOrTo(crossId), maxLength);
                        cars.push_back(car->GetCar()); //cheater
                        car->SetRealTime(time);
                        goout = true;
                        break;
                    }
                    if (!goout)
                    {
                        LOG("car " << car->GetCar()->GetId() << " can not go on the road " << roadId << " @" << time);
                        car->SetRealTime(time + 1); //cheater
                    }
                    else //go out
                    {
                        findGarage->second.erase(*cangoCarIte);
                        getOutCounter++;
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

void Simulator::PrintCrossState(const int& time, SimScenario& scenario, Cross* cross) const
{
    int crossId = cross->GetId();
    for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
    {
        int id = cross->GetRoadId((Cross::DirectionType)i);
        if (id >= 0)
        {
            SimRoad* road = &scenario.Roads()[id];
            if (road->GetRoad()->GetEndCrossId() == crossId ||
                (road->GetRoad()->GetStartCrossId() == crossId && road->GetRoad()->GetIsTwoWay()))
            {
                LOG("Road [" << road->GetRoad()->GetId() << "] direction : " << i << " lanes " << road->GetRoad()->GetLanes() << " limit " << road->GetRoad()->GetLimit());
                for (int j = 1; j <= road->GetRoad()->GetLanes(); j++)
                {
                    LOG("\tLane " << j);
                    auto& cars = road->GetCarsTo(j, crossId);
                    for (auto carIte = cars.begin(); carIte != cars.end(); carIte++)
                    {
                        SimCar* car = &scenario.Cars()[(*carIte)->GetId()];
                        if (car->GetSimState(time) == SimCar::SCHEDULED || GetPositionInNextRoad(time, scenario, car) <= 0)
                            break;
                        LOG("car [" << car->GetCar()->GetId() << "]"
                            << " pos " << car->GetCurrentPosition()
                            << " speed " << car->GetCar()->GetMaxSpeed()
                            << " dir " << GetDirection(road->GetRoad()->GetId(), car->GetNextRoadId())
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
        for (auto crossIte = Scenario::Crosses().begin(); crossIte != Scenario::Crosses().end(); crossIte++)
        {
            int crossId = crossIte->second->GetId();
            for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
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
                            LOG("id " << firstPriority->GetCar()->GetId()
                                << " cross " << crossId
                                << " road " << road->GetRoad()->GetId()
                                << " lane " << i
                                << " position " << firstPriority->GetCurrentPosition()
                                << " next " << firstPriority->GetNextRoadId()
                                << " dir " << (firstPriority->GetNextRoadId() < 0 ? DIRECT : GetDirection(road->GetRoad()->GetId(), firstPriority->GetNextRoadId()))
                                << " waiting for " << firstPriority->GetWaitingCar(time)->GetCar()->GetId()
                                );
                        }
                    }
                }
            }
        }
    }
}
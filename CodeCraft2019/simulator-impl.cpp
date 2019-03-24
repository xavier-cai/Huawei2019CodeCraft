#include "simulator-impl.h"
#include "scenario.h"
#include <algorithm>

void UpdateCarsInRoad(const int& time, SimScenario& scenario, SimRoad* road)
{
    int lanes = road->GetRoad()->GetLanes();
    for (int i = 0; i < lanes * 2; i++)
    {
        bool opposite = i >= lanes;
        if (opposite && !road->GetRoad()->GetIsTwoWay())
            break;
        int lane = (i % lanes) + 1;
        auto& cars = opposite ? road->GetCarsOpposite(lane) : road->GetCars(lane);
        SimCar* frontCar = 0;
        for (auto carIte = cars.begin(); carIte != cars.end(); carIte++)
        {
            SimCar* car = &scenario.Cars()[(*carIte)->GetId()];
            SimCar::SimState state = car->GetSimState(time);
            if (state == SimCar::WAITING && car->GetWaitingCar(time) == car)
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
                        car->UpdatePosition(time, std::min(nexPosition, road->GetRoad()->GetLength()));
                }
                else
                {
                    ASSERT(frontCar != 0);
                    int frontPosition = frontCar->GetCurrentPosition();
                    if (frontCar->GetSimState(time) != SimCar::SCHEDULED && nexPosition >= frontPosition) //need wait
                        car->UpdateWaiting(time, frontCar);
                    else
                        car->UpdatePosition(time, std::min(nexPosition, frontPosition - 1));
                }
            }
            frontCar = car;
        }
    }
}

Simulator::UpdateResult SimulatorImpl::UpdateSelf(const int& time, SimScenario& scenario)
{
    Simulator::UpdateResult result;
    result.Conflict = false;
    Simulator::ScheduledCarsN = 0; //counter
    Simulator::ReachedCarsN = 0; //counter

    for (auto roadIte = scenario.Roads().begin(); roadIte != scenario.Roads().end(); roadIte++)
    {
        SimRoad* road = &roadIte->second;
        UpdateCarsInRoad(time, scenario, road);
    }
    std::list<Cross*> crosses;
    for (auto ite = Scenario::Crosses().begin(); ite != Scenario::Crosses().end(); ite++)
        crosses.push_back(ite->second);
    while(crosses.size() > 0)
    {
        Simulator::ConflictFlag = true; //for checking conflict
        for (auto crossIte = crosses.begin(); crossIte != crosses.end();)
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
                        //auto state = firstPriority->GetSimState(time);
                        //if (state == SimCar::WAITING && firstPriority->GetWaitingCar(time)->GetSimState(time) != SimCar::SCHEDULED)
                        //{
                        //    if (firstPriority->GetWaitingCar(time)->GetCurrentCross()->GetId() != crossId)
                        //        noEmptyPlace = true;
                        //    break;
                        //}
                        if (!Simulator::PassCrossOrJustForward(time, scenario, firstPriority))
                        {
                            ASSERT (firstPriority->GetSimState(time) == SimCar::WAITING && firstPriority->GetWaitingCar(time) != 0);
                            break;
                        }
                        UpdateCarsInRoad(time, scenario, road);
                        //crossConflict = false;
                    }
                    //if (firstPriority == 0)
                    //    roadIte = roads.erase(roadIte);
                    //else
                        roadIte++;
                }
                //if(crossConflict)
                    //break;
            }
            //if(roads.size() == 0)
            //    crossIte = crosses.erase(crossIte);
            //else
                crossIte++;
        }
        if (Simulator::ScheduledCarsN - Simulator::ReachedCarsN == scenario.GetOnRoadCarsN()) //complete
        {
            result.Conflict = false;
            break;
        }
        if (Simulator::ConflictFlag && scenario.GetOnRoadCarsN() > 0) //conflict
        {
            Simulator::InstanceAgent.PrintDeadLock(time, scenario);
            result.Conflict = true;
            break;
        }
    }
    if (!result.Conflict)
    {
        Simulator::InstanceAgent.GetOutFromGarage(time, scenario);
    }
    return result;
}
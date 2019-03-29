#include "scheduler-floyd.h"
#include "scenario.h"
#include "assert.h"
#include "log.h"

double SchedulerFloyd::lenthWeight(0.1);
double SchedulerFloyd::lanesWeight(0.2);
double SchedulerFloyd::carNumWeight(0.9);
double SchedulerFloyd::carLimit(2.1);

SchedulerFloyd::SchedulerFloyd()
{ }

void SchedulerFloyd::DoInitialize(SimScenario& scenario)
{
    int size = Scenario::Crosses().size();
    crosslength = m_memoryPool.NewArray<double*>(size);
    crosspath = m_memoryPool.NewArray<int*>(size);
    minpath = m_memoryPool.NewArray<std::list<int>*>(size);
    minspeed = 1.0;
    //crossspeed = m_memoryPool.NewArray<int>(size);
    for (int i = 0; i < size; i++)
    {
        //crossspeed[i] = 1;
        crosslength[i] = m_memoryPool.NewArray<double>(size);
        crosspath[i] = m_memoryPool.NewArray<int>(size);
        minpath[i] = m_memoryPool.NewArray< std::list<int> >(size);
    }

    m_deadLockSolver.Initialize(0, scenario);
    m_deadLockSolver.SetSelectedRoadCallback(Callback::Create(&SchedulerFloyd::SelectBestRoad, this));
}

void SchedulerFloyd::DoUpdate(int& time, SimScenario& scenario)
{
    int crossidfirst;
    int crossidsecond;
    for (auto itefirst = Scenario::Crosses().begin(); itefirst != Scenario::Crosses().end(); itefirst++)
    {
        crossidfirst = itefirst->first;
        for (auto itesecond = Scenario::Crosses().begin(); itesecond != Scenario::Crosses().end(); itesecond++)
        {
            crossidsecond = itesecond->first;
            crosslength[_c(crossidfirst)][_c(crossidsecond)] = 10000;
            crosspath[_c(crossidfirst)][_c(crossidsecond)] = crossidsecond;
        }
    }
    for (auto itefirst = Scenario::Crosses().begin(); itefirst != Scenario::Crosses().end(); itefirst++)
    {
        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
        {
            Road* road = itefirst->second->GetRoad((Cross::DirectionType)i);
            if (road != 0)
            {
                SimRoad* roadlink = &scenario.Roads()[road->GetId()];
                if (roadlink->GetRoad()->GetStartCrossId() == itefirst->first ||
                    (roadlink->GetRoad()->GetEndCrossId() == itefirst->first && roadlink->GetRoad()->GetIsTwoWay()))
                {
                    Cross* peerlink = roadlink->GetRoad()->GetPeerCross(itefirst->second);
                    int carAver = 0;
                    for (int j = 1; j <= roadlink->GetRoad()->GetLanes(); j++)
                    {
                        carAver += roadlink->GetCarsFrom(j, itefirst->first).size();
                    }
                    carAver = carAver/roadlink->GetRoad()->GetLanes();

                    int roadLength = itefirst->second->GetRoad((Cross::DirectionType)i)->GetLength();
                    /*
                    if (carAver >= roadLength-1)
                    {
                        carAver = carAver * 10;
                    }
                    */
                    int maxRoadspeed = itefirst->second->GetRoad((Cross::DirectionType)i)->GetLimit();
                    if (maxRoadspeed < minspeed)
                    {
                        minspeed = maxRoadspeed;
                    }
                    crosslength[_c(itefirst->first)][_c(peerlink->GetId())] = ((double)roadLength) * lenthWeight + (double)carAver * (double)carAver * (double)carAver / (double)roadLength;//carNumWeight;
                }
            }
        }
    }
    for (auto iteTransfer = Scenario::Crosses().begin(); iteTransfer != Scenario::Crosses().end(); iteTransfer++)
    {
        for (auto iteRow = Scenario::Crosses().begin(); iteRow != Scenario::Crosses().end(); iteRow++)
        {
            for (auto iteColumn = Scenario::Crosses().begin(); iteColumn != Scenario::Crosses().end(); iteColumn++)
            {
                auto& element = crosslength[_c(iteRow->first)][_c(iteColumn->first)];
                double lengthAfterTran = crosslength[_c(iteRow->first)][_c(iteTransfer->first)] + crosslength[_c(iteTransfer->first)][_c(iteColumn->first)];
                if (element > lengthAfterTran)
                {
                    element = lengthAfterTran;
                    crosspath[_c(iteRow->first)][_c(iteColumn->first)] = iteTransfer->first;
                }
            }
        }
    }
    for (auto iteStart = Scenario::Crosses().begin(); iteStart != Scenario::Crosses().end(); iteStart++)
    {
        for (auto iteEnd = Scenario::Crosses().begin(); iteEnd != Scenario::Crosses().end(); iteEnd++)
        {
            int startstep = iteStart->first;
            
            std::list<int> pathlist;
            std::list<int> crosslist;
            while (startstep != iteEnd->first)
            {
                int transstep = crosspath[_c(startstep)][_c(iteEnd->first)];
                while (crosspath[_c(startstep)][_c(transstep)] != transstep)
                {
                    transstep = crosspath[_c(startstep)][_c(transstep)];

                }
                startstep = transstep;
                crosslist.push_back(startstep);
            }
            //trans crosses to roads
            Cross* lastCross = iteStart->second;
            for (auto ite = crosslist.begin(); ite != crosslist.end(); ite++)
            {
                bool consistant = false;
                Cross* thisCross = Scenario::GetCross(*ite);
                for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
                {
                    Road* road = lastCross->GetRoad((Cross::DirectionType)i);
                    if (road != 0)
                    {
                        if ((road->GetStartCrossId() == lastCross->GetId() && road->GetEndCrossId() == thisCross->GetId())
                            || (road->GetEndCrossId() == lastCross->GetId() && road->GetStartCrossId() == thisCross->GetId() && road->GetIsTwoWay()))
                        {
                            consistant = true;
                            pathlist.push_back(road->GetId());
                            break;
                        }
                    }
                }
                ASSERT_MSG(consistant, "can not find the road bewteen " << lastCross->GetId() << " and " << thisCross->GetId());
                lastCross = thisCross;
            }
            minpath[_c(iteStart->first)][_c(iteEnd->first)] = pathlist;
        }
    }

    for(auto ite = scenario.Cars().begin(); ite != scenario.Cars().end(); ite++)
	{
        SimCar* car = &ite->second;
        if (car->GetCar()->GetFromCrossId() != car->GetCar()->GetToCrossId()
            && !car->GetIsReachedGoal()
            && !m_deadLockSolver.IsCarTraceLockedInBackup(car))
        {
            int from = car->GetCar()->GetFromCrossId();
            if (!car->GetIsInGarage() && car->GetCurrentRoad() != 0)
                from = car->GetCurrentCross()->GetId();
            int to = car->GetCar()->GetToCrossId();
            auto& carTrace = car->GetTrace();
            auto* newTrace = &minpath[_c(from)][_c(to)];

            if (car->GetIsInGarage())
                carTrace.Clear();
            else
            {
                if (!car->GetIsLockOnNextRoad())
                {
                    carTrace.Clear(car->GetCurrentTraceNode());
                    //drop back
                    if (carTrace.Size() > 0 && newTrace->size() > 0 && *newTrace->begin() == *--carTrace.Tail())
                    {
                        double minPath = -1;
                        int minPathId = -1;
                        int minCrossId = -1;
                        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
                        {
                            Road* road = Scenario::GetCross(from)->GetRoad((Cross::DirectionType)i);

                            if (road != 0 && road->GetId() != car->GetCurrentRoad()->GetId())
                            {
                                if (road->GetStartCrossId() == from ||
                                (road->GetEndCrossId() == from && road->GetIsTwoWay()))
                                {
                                    Cross* nearCross = road->GetPeerCross(Scenario::GetCross(from));
                                    double newlength = crosslength[_c(from)][_c(nearCross->GetId())] + crosslength[_c(nearCross->GetId())][_c(to)];
                                    if ((minPath < -0.5) ||(minPath > newlength))
                                    {
                                        minPath = newlength;
                                        minPathId = road->GetId();
                                        minCrossId = nearCross->GetId();
                                    }   
                                }
                            }
                        }
                        ASSERT(carTrace.Size() == 0 || (*(carTrace.Tail() - 1) != minPathId));
                        ASSERT(minPathId >= 0 && minCrossId >= 0);
                        carTrace.AddToTail(minPathId);
                        newTrace = &minpath[_c(minCrossId)][_c(to)];
                    }
                }
            }

            if (!car->GetIsLockOnNextRoad()  //will be updated
                && carTrace.Size() != 0 && newTrace != 0 && newTrace->size() > 0 //on the road
                && newTrace == &minpath[_c(from)][_c(to)]) //and no drop back
                ASSERT(*(carTrace.Tail() - 1) != *newTrace->begin()); //check next jump
            if (!car->GetIsLockOnNextRoad()) //can not update road if locked
            {
                for (auto traceIte = newTrace->begin(); traceIte != newTrace->end(); traceIte++)
                {
                    //ASSERT(carTrace.Size() == 0 || (*(carTrace.Tail() - 1) != *traceIte));
                    carTrace.AddToTail(*traceIte);
                }
            }

#ifdef ASSERT_ON
            //check path valid
            Cross* frontCross = car->GetCar()->GetFromCross();
            int frontRoad = -1;
            for (auto ite = carTrace.Head(); ite != carTrace.Tail(); ite++)
            {
                Road* road = Scenario::GetRoad(*ite);
                ASSERT(road != 0);
                ASSERT(road->GetId() >= 0);
                ASSERT(road->GetId() != frontRoad);
                ASSERT(road->CanStartFrom(frontCross->GetId()));
                frontCross = road->GetPeerCross(frontCross);
            }
            ASSERT(frontCross->GetId() == car->GetCar()->GetToCrossId());
#endif
        }
	}
}

void SchedulerFloyd::DoHandleGetoutGarage(const int& time, SimScenario& scenario, SimCar* car)
{
    if (m_deadLockSolver.IsGarageLockedInBackup(time))
        return;

    int roadId = car->GetNextRoadId();
    ASSERT(roadId >= 0);
    SimRoad* road = &scenario.Roads()[roadId];
    int crossId = car->GetCar()->GetFromCrossId();
    int carSum = 0;
    
    for(int i = 1; i <= road->GetRoad()->GetLanes(); i++)
    {
        carSum += road->GetCarsFrom(i, crossId).size();
    }
    double carAver = double(carSum)/double(road->GetRoad()->GetLanes());
    int minspeedingarage = -1;
    for (auto itergarage = scenario.Garages()[car->GetCar()->GetFromCrossId()].begin();
        itergarage != scenario.Garages()[car->GetCar()->GetFromCrossId()].end(); itergarage++)
    {
        if (minspeedingarage == -1 || minspeedingarage > (*itergarage)->GetCar()->GetMaxSpeed())
        {
            minspeedingarage = (*itergarage)->GetCar()->GetMaxSpeed();
            minspeed = minspeedingarage;
        }
    }
    if(carAver > carLimit || car->GetCar()->GetMaxSpeed() > minspeedingarage)//road->GetRoad()->GetLength()/5)
    {
        car->SetRealTime(time + 1);
    }
}

#include "random.h"
void SchedulerFloyd::DoHandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result)
{
    if (result.Conflict)
    {
        if (m_deadLockSolver.HandleDeadLock(time, scenario))
            result.Conflict = false; //retry
    }
}

std::pair<int, bool> SchedulerFloyd::SelectBestRoad(const std::list<int>& list, int from, SimCar* car)
{
    int to = car->GetCar()->GetToCrossId();
    double minPath = -1;
    int minPathId = -1;
    for (auto ite = list.begin(); ite != list.end(); ite++)
    {
        Road* road = Scenario::GetRoad(*ite);
        ASSERT(road->GetStartCrossId() == from ||
            (road->GetEndCrossId() == from && road->GetIsTwoWay()));
        Cross* nearCross = road->GetPeerCross(Scenario::GetCross(from));
        double newlength = crosslength[_c(from)][_c(nearCross->GetId())] + crosslength[_c(nearCross->GetId())][_c(to)];
        if ((minPath < -0.5) ||(minPath > newlength))
        {
            minPath = newlength;
            minPathId = road->GetId();
        }
    }
    return std::make_pair(minPathId, false);
}

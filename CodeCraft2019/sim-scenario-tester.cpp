#include "sim-scenario-tester.h"
#include "random.h"
#include "scenario.h"
#include "log.h"
#include "assert.h"
#include <set>
#include "simulator.h"

#define PEEK_CAR (&m_simCars[carIndex++])

double SimScenariotTester::GeneProb(0.7);
double SimScenariotTester::BlockProb(0.2);
double SimScenariotTester::WaitingProb(0.5);

SimScenariotTester::SimScenariotTester()
{
    m_simGarages.clear();

    int carIndex = 10000;
    Cross* cross = 0;
    int tryGetCross = 100;
    while (tryGetCross-- > 0)
    {
        for (auto ite = Scenario::Crosses().begin(); ite != Scenario::Crosses().end(); ite++)
        {
            bool valid = true;
            DirectionType_Foreach(dir, 
                if (ite->second->GetRoad(dir) == 0 || !ite->second->GetRoad(dir)->GetIsTwoWay())
                {
                    valid = false;
                    break;
                }
            );
            if (valid && Random::Uniform() < 0.2)
            {
                cross = ite->second;
                break;
            }
        }
    }
    ASSERT_MSG(cross != 0, "can not get a valid cross");
    std::map<int, SimRoad*> roads;
    std::vector<int> roadRng;
    DirectionType_Foreach(dir,
        SimRoad* road = &m_simRoads[cross->GetRoad(dir)->GetId()];
        roads[road->GetRoad()->GetId()] = road;
        roadRng.push_back(road->GetRoad()->GetId());
        road->GetRoad()->SetLength(Random::Uniform(8, 21));
        road->GetRoad()->SetLimit(Random::Uniform(2, 9));
        LOG("road " << road->GetRoad()->GetId()
            << " lanes " << road->GetRoad()->GetLanes()
            << " length " << road->GetRoad()->GetLength()
            << " limit " << road->GetRoad()->GetLimit());
    );
    
    for (auto ite = roads.begin(); ite != roads.end(); ite++)
    {
        SimRoad* road = ite->second;
        int roadId = ite->first;
        for (int i = 1; i < road->GetRoad()->GetLanes(); i++)
        {
            /* passing cars */
            auto& toCar = road->GetCarsTo(i, cross->GetId());
            for (int pos = 0; pos < 3; pos++)
            {
                if(Random::Uniform() < GeneProb)
                {
                    SimCar* car = PEEK_CAR;
                    car->GetCar()->SetFromCrossId(road->GetRoad()->GetPeerCross(cross)->GetId());
                    car->GetCar()->SetFromCross(road->GetRoad()->GetPeerCross(cross));
                    car->GetTrace().AddToTail(roadId);
                    int nextId = roadRng[Random::Uniform(0, roadRng.size())];
                    if (nextId != roadId)
                    {
                        car->GetTrace().AddToTail(nextId);
                    }
                    else
                    {
                        car->GetCar()->SetToCrossId(cross->GetId());
                        car->GetCar()->SetToCross(cross);
                    }
                    car->GetCar()->SetMaxSpeed(Random::Uniform(1, 9));
                    car->UpdateOnRoad(1, road->GetRoad(), i, !road->IsFromOrTo(cross->GetId()), road->GetRoad()->GetLength() - pos);
                    toCar.push_back(car->GetCar());
                    LOG("generate passing cross car [" << car->GetCar()->GetId() << "]"
                        << " next " << (nextId == roadId ? -1 : nextId)
                        << " dir " << (nextId == roadId ? Simulator::DIRECT : Simulator::GetDirection(roadId, nextId))
                        << " speed " << car->GetCar()->GetMaxSpeed()
                        );
                }
            }
            /* block cars */
            auto& fromCar = road->GetCarsFrom(i, cross->GetId());
            for (int pos = 2; pos >= 1; pos--)
            {
                if(Random::Uniform() < BlockProb)
                {
                    SimCar* car = PEEK_CAR;
                    car->GetCar()->SetFromCrossId(cross->GetId());
                    car->GetCar()->SetFromCross(cross);
                    car->GetCar()->SetMaxSpeed(0);
                    car->GetCar()->SetToCrossId(road->GetRoad()->GetPeerCross(cross)->GetId());
                    car->GetCar()->SetToCross(road->GetRoad()->GetPeerCross(cross));
                    car->GetTrace().AddToTail(roadId);
                    bool waiting = Random::Uniform() < WaitingProb;
                    car->UpdateOnRoad((waiting ? 1 : 2), road->GetRoad(), i, road->IsFromOrTo(cross->GetId()), pos);
                    if (waiting) car->UpdateWaiting(2, car);
                    fromCar.push_back(car->GetCar());
                    LOG("generate " << (waiting ? "waiting" : "scheduled") << " block cross car [" << car->GetCar()->GetId() << "]");
                }
            }
        }
    }
}

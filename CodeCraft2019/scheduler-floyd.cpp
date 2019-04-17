#include "scheduler-floyd.h"
#include "scenario.h"
#include "assert.h"
#include "log.h"
#include <algorithm>
#include "tactics.h"
#define Inf 1000000

/* Interfaces
    Trace& trace = Tactics::Instance.GetTraces()[car->GetCar()->GetId()];
*/

SchedulerFloyd::SchedulerFloyd()
    : m_updateInterval(1)
    , m_carsNumOnRoadLimit(-1)
{ 
    SetLengthWeight(0.1);
    SetCarNumWeight(0.9);
    SetLanesNumWeight(0.2);
    SetCarLimit(2.1);
    SetPresetVipTracePreloadWeight(1.0);
    SetLastPresetVipCarRealTime(0);
    SetLastPresetVipCarEstimateArriveTime(0);
    SetIsEnableVipWeight(false);
    SetIsOptimalForLastVipCar(true);
    SetIsLimitedByRoadSizeCount(true);
    SetIsFasterAtEndStep(true);
    SetIsLessCarAfterDeadLock(false);
    SetIsDropBackByDijkstra(false);
    SetIsVipCarDispatchFree(false);
}

void SchedulerFloyd::SetLengthWeight(double v)
{
    m_lengthWeight = v;
}

void SchedulerFloyd::SetCarNumWeight(double v)
{
    m_carNumWeight = v;
}

void SchedulerFloyd::SetLanesNumWeight(double v)
{
    m_lanesNumWeight = v;
}

void SchedulerFloyd::SetCarLimit(double v)
{
    m_carLimit = v;
    m_carLimitLooser = v * 2.0;
    m_carLimitTighter = v * 0.25;
}

void SchedulerFloyd::SetPresetVipTracePreloadWeight(double v)
{
    m_presetVipTracePreloadWeight = v;
}

void SchedulerFloyd::SetLastPresetVipCarRealTime(int v)
{
    m_lastPresetVipCarRealTime = v;
}

void SchedulerFloyd::SetLastPresetVipCarEstimateArriveTime(int v)
{
    m_lastPresetVipCarEstimateArriveTime = v;
}

void SchedulerFloyd::SetIsEnableVipWeight(bool v)
{
    m_isEnableVipWeight = v;
}

void SchedulerFloyd::SetIsOptimalForLastVipCar(bool v)
{
    m_isOptimalForLastVipCar = v;
}

void SchedulerFloyd::SetIsLimitedByRoadSizeCount(bool v)
{
    m_isLimitedByRoadSizeCount = v;
}

void SchedulerFloyd::SetIsFasterAtEndStep(bool v)
{
    m_isFasterAtEndStep = v;
}

void SchedulerFloyd::SetIsLessCarAfterDeadLock(bool v)
{
    m_isLessCarAfterDeadLock = v;
}

void SchedulerFloyd::SetIsDropBackByDijkstra(bool v)
{
    m_isDropBackByDijkstra = v;
}

void SchedulerFloyd::SetIsVipCarDispatchFree(bool v)
{
    m_isVipCarDispatchFree = v;
}

bool CompareVipCars(SimCar* a, SimCar* b)
{
    return a->CalculateTime(true) > b->CalculateTime(true);
}

void SchedulerFloyd::HandlePresetCars(SimScenario& scenario)
{
    std::vector<SimCar*> presetVipCars;
    presetVipCars.reserve(Scenario::GetVipCarsN());
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car!= 0 && car->GetCar()->GetIsPreset() && car->GetCar()->GetIsVip())
        { 
            presetVipCars.push_back(car);
        }
    }
    std::sort(presetVipCars.begin(), presetVipCars.end(), CompareVipCars);
    int canChangesN = Scenario::GetPresetCarsN() * 0.1;
    for (uint i = 0; i < canChangesN; ++i)
    {
        presetVipCars[i]->SetIsForceOutput(true);
    }
}

void SchedulerFloyd::CalculateWeight(SimScenario& scenario)
{
    uint roadCount = scenario.Roads().size();
    int lengthTotal = 0;
    for (uint i = 0; i < roadCount; ++i)
    {
        lengthTotal += Scenario::Roads()[i]->GetLength();
    }
    m_roadCapacityAverage = lengthTotal / roadCount;
    SetCarLimit(m_roadCapacityAverage / 10.0);
    m_carsNumOnRoadLimit = roadCount * m_roadCapacityAverage;
    m_lastPresetVipCarRealTime = -1;
    m_lastPresetVipCarEstimateArriveTime = -1;
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car != 0 && car->GetCar()->GetIsVip() && (car->GetCar()->GetIsPreset() && !car->GetIsForceOutput()))
        {
            int realTime = car->GetRealTime();
            int arriveTime = realTime + car->CalculateTime(true);
            if (m_lastPresetVipCarRealTime < 0 || realTime > m_lastPresetVipCarRealTime)
                m_lastPresetVipCarRealTime = realTime;
            if (m_lastPresetVipCarEstimateArriveTime < 0 || arriveTime > m_lastPresetVipCarEstimateArriveTime)
                m_lastPresetVipCarEstimateArriveTime = arriveTime;
        }
    }
}

void SchedulerFloyd::RefreshNotArrivedPresetCars(SimScenario& scenario)
{
    m_notArrivedPresetCars.clear();
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car != 0 && car->GetCar()->GetIsVip() && car->GetCar()->GetIsPreset() && !car->GetIsForceOutput() && !car->GetIsReachedGoal())
            m_notArrivedPresetCars.insert(car->GetCar()->GetId());
    }
}

void SchedulerFloyd::DoInitialize(SimScenario& scenario)
{
    //HandlePresetCars(scenario);

    uint crossCount = Scenario::Crosses().size();
    int roadCount = Scenario::Roads().size();

    CalculateWeight(scenario);

    if (m_isFasterAtEndStep)
    {
        RefreshNotArrivedPresetCars(scenario);
    }

    LOG("Car Limit = " << m_carLimit);
    weightCrossToCross.resize(crossCount);
    connectionCrossToCross.resize(crossCount);
    minPathCrossToCross.resize(crossCount);
    appointOnRoadCounter.resize(roadCount, std::make_pair(0, 0));
    garageMinSpeed.resize(crossCount, -1);

    for (uint i = 0; i < crossCount; i++)
    {
        weightCrossToCross[i].resize(crossCount);
        connectionCrossToCross[i].resize(crossCount);
        minPathCrossToCross[i].resize(crossCount);
    }

    m_deadLockSolver.Initialize(0, scenario);
    m_deadLockSolver.SetSelectedRoadCallback(Callback::Create(&SchedulerFloyd::SelectBestRoad, this));
    SimCar::SetUpdateGoOnNewRoad(Callback::Create(&SchedulerFloyd::HandleGoOnNewRoad, this));
}

void SchedulerFloyd::HandleGoOnNewRoad(const SimCar* car, Road* oldRoad)
{
    bool reachGoal = car->GetIsReachedGoal();
    if (reachGoal)
    {
        if (car->GetCar()->GetIsVip())
        {
            if (car->GetCar()->GetIsPreset() && !car->GetIsForceOutput())
            {
                LOG("PreVip" << " :" << car->GetRealTime() << " :" << time);
                m_notArrivedPresetCars.erase(car->GetCar()->GetId());
            }
            else
            {
                LOG(car->GetCar()->GetId() << " :" << time);
            }
        }
    }
    return;
    
    if (oldRoad != 0)
    {
        Cross* oldCross = car->GetCar()->GetToCross();
        if (!reachGoal)
            oldCross = car->GetCurrentDirection() ? car->GetCurrentRoad()->GetStartCross() : car->GetCurrentRoad()->GetEndCross();
        auto& pair = appointOnRoadCounter[oldRoad->GetId()];
        (oldRoad->IsFromOrTo(oldCross->GetId()) ? pair.second : pair.first) -= 1;
    }
    if (!reachGoal)
    {
        int newRoadId = car->GetCurrentRoad()->GetId();
        auto& pair = appointOnRoadCounter[newRoadId];
        (car->GetCurrentDirection() ? pair.first : pair.second) += 1;
    }
}
void SchedulerFloyd::DoHandleBecomeFirstPriority(const int& time, SimScenario& scenario, SimCar* car)
{
    return;
}
void SchedulerFloyd::DoUpdate(int& time, SimScenario& scenario)
{
    //appointOnRoadCounter单时间片更新

    if (!m_deadLockSolver.NeedUpdate(time))
    {
        return;
    }
    if (time % m_updateInterval != 0)
    {
        return;
    }

    uint crossSize = Scenario::Crosses().size();
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        int minSpeedInGarage = -1;
        auto& garage = scenario.Garages()[iCross];
        for (uint iCar = 0; iCar < garage.size(); ++iCar)
        {
            if (garage[iCar] != 0 && garage[iCar]->GetIsInGarage() && !garage[iCar]->GetCar()->GetIsPreset())
            {
                if (minSpeedInGarage > garage[iCar]->GetCar()->GetMaxSpeed() || minSpeedInGarage < 0)
                {
                    minSpeedInGarage = garage[iCar]->GetCar()->GetMaxSpeed();
                    garageMinSpeed[iCross] = minSpeedInGarage;
                }
            }
        }
        for (uint jCross = 0; jCross < crossSize; ++jCross)
        {
            weightCrossToCross[iCross][jCross] = Inf;
            connectionCrossToCross[iCross][jCross] = jCross;
        }
    }

    if (m_isEnableVipWeight)
    {
        if (m_notArrivedPresetCars.size() > 0)
        {
            bool noPreVipCar = true;
            for (uint iCar = 0; iCar < scenario.Cars().size(); ++iCar)
            {
                SimCar* car = scenario.Cars()[iCar];
                if (car == 0) continue;
                if (car->GetIsReachedGoal()) continue;

                if (time >= m_lastPresetVipCarRealTime - 60 && (car->GetCar()->GetIsPreset() && !car->GetIsForceOutput()) && car->GetCar()->GetIsVip())
                {
                    noPreVipCar = false;
                    auto& carTrace = car->GetTrace();
                    Cross* lastCross = car->GetCurrentCross();
                    for (uint iTrace = car->GetCurrentTraceIndex(); iTrace < carTrace.Size(); ++iTrace)
                    {
                        Road* currentRoad = Scenario::Roads()[carTrace[iTrace]];
                        Cross* nextCross = currentRoad->GetPeerCross(lastCross);
                        auto& updateweightCrossToCross = weightCrossToCross[lastCross->GetId()][nextCross->GetId()];
                        ASSERT(nextCross->GetId() == currentRoad->GetPeerCross(lastCross)->GetId());
                        if (updateweightCrossToCross == Inf)
                        {
                            updateweightCrossToCross = m_presetVipTracePreloadWeight;
                        }
                        else
                        {
                            updateweightCrossToCross += m_presetVipTracePreloadWeight;
                        }
                        lastCross = nextCross;
                    }
                }
            }
        }
    }

    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        Cross* cross = Scenario::Crosses()[iCross];
        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
        {
            Road* road = cross->GetRoad((Cross::DirectionType)i);
            if (road != 0)
            {
                SimRoad* roadLink = scenario.Roads()[road->GetId()];
                if (roadLink->GetRoad()->CanStartFrom(cross->GetId()))
                {
                    Cross* peerlink = roadLink->GetRoad()->GetPeerCross(cross);
                    auto& updateweightCrossToCross = weightCrossToCross[iCross][peerlink->GetId()];
                    double carAver = 0;
                    if (updateweightCrossToCross != Inf)
                    {
                        carAver += weightCrossToCross[iCross][peerlink->GetId()];
                    }
                    //ASSERT(carAver <= 5000);
                    int roadLines = roadLink->GetRoad()->GetLanes();
                    
                    for (int j = 1; j <= roadLines; j++)
                    {
                        carAver += (double)roadLink->GetCarsFrom(j, iCross).size();
                    }
                    
                    carAver = carAver/ (double)roadLink->GetRoad()->GetLanes();
                    int roadLength = cross->GetRoad((Cross::DirectionType)i)->GetLength();
                    weightCrossToCross[iCross][peerlink->GetId()] = (double)roadLength  * m_lengthWeight + carAver * carAver * carAver / (double)roadLength;//m_carNumWeight;
                }
            }
        }
    }

    for (uint iTransfer = 0; iTransfer < crossSize; ++iTransfer)
    {
        for (uint iRow = 0; iRow < crossSize; ++iRow)
        {
            for (uint iColumn = 0; iColumn < crossSize; ++iColumn)
            {
                double lengthAfterTran = weightCrossToCross[iRow][iTransfer] + weightCrossToCross[iTransfer][iColumn];
                if (weightCrossToCross[iRow][iColumn] > lengthAfterTran)
                {
                    weightCrossToCross[iRow][iColumn] = lengthAfterTran;
                    connectionCrossToCross[iRow][iColumn] = iTransfer;
                }
            }
        }
    }

    for (uint iStart = 0; iStart < crossSize; ++iStart)
    {
        for (uint iEnd = 0; iEnd < crossSize; ++iEnd)
        {
            int startStep = iStart;
            static std::vector<int> crossList;
            crossList.clear();
            while (startStep != iEnd)
            {
                int transstep = connectionCrossToCross[startStep][iEnd];
                while (connectionCrossToCross[startStep][transstep] != transstep)
                {
                    transstep = connectionCrossToCross[startStep][transstep];
                }
                startStep = transstep;
                crossList.push_back(startStep);
            }

            //trans crosses to roads
            auto& pathList = minPathCrossToCross[iStart][iEnd];
            pathList.clear();
            Cross* lastCross = Scenario::Crosses()[iStart];
            for (uint i = 0; i < crossList.size(); ++i)
            {
                bool consistant = false;
                Cross* thisCross = Scenario::Crosses()[crossList[i]];
                for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
                {
                    Road* road = lastCross->GetRoad((Cross::DirectionType)i);
                    if (road != 0)
                    {
                        if (road->CanStartFrom(lastCross->GetId()) && road->CanReachTo(thisCross->GetId()))
                        {
                            consistant = true;
                            pathList.push_back(road->GetId());
                            break;
                        }
                    }
                }
                ASSERT_MSG(consistant, "can not find the road bewteen " << lastCross->GetId() << " and " << thisCross->GetId());
                lastCross = thisCross;
            }
        }
    }

    for(uint i = 0; i < scenario.Cars().size(); ++i)
	{
        SimCar* car = scenario.Cars()[i];
        if (car == 0) continue;
        auto& carTrace = car->GetTrace();
        if (car->GetCar()->GetFromCrossId() != car->GetCar()->GetToCrossId()
            && !car->GetIsReachedGoal()
            && (!car->GetCar()->GetIsPreset() || car->GetIsForceOutput())
            && !m_deadLockSolver.IsCarTraceLockedInBackup(car))
        {
            int from = car->GetCar()->GetFromCrossId();
            if (!car->GetIsInGarage() && car->GetCurrentRoad() != 0)
                from = car->GetCurrentCross()->GetId();
            int to = car->GetCar()->GetToCrossId();
            const auto* newTrace = &minPathCrossToCross[from][to];

            if (car->GetIsInGarage())
                carTrace.Clear();
            else
            {
                if (!car->GetIsLockOnNextRoad())
                {
                    if (m_isDropBackByDijkstra || !(carTrace.Size() > 0 && newTrace->size() > 0 && *newTrace->begin() == car->GetCurrentRoad()->GetId()))
                    {
                        carTrace.Clear(car->GetCurrentTraceIndex());
                    }
                    //drop back
                    if (m_isDropBackByDijkstra)
                    {
                        UpdateCarTraceByDijkstra(time, scenario, car);
                    }
                }
            }

            if (!car->GetIsLockOnNextRoad()  //will be updated
                && carTrace.Size() != 0 && newTrace != 0 && newTrace->size() > 0 //on the road
                && newTrace == &minPathCrossToCross[from][to]) //and no drop back
                ASSERT(*(carTrace.Tail() - 1) != *newTrace->begin()); //check next jump
            if (!car->GetIsLockOnNextRoad() && (m_isDropBackByDijkstra || !(carTrace.Size() > 0 && newTrace->size() > 0 && *newTrace->begin() == car->GetCurrentRoad()->GetId()))) //can not update road if locked
            {
                for (auto traceIte = newTrace->begin(); traceIte != newTrace->end(); traceIte++)
                {
                    //ASSERT(carTrace.Size() == 0 || (*(carTrace.Tail() - 1) != *traceIte));
                    carTrace.AddToTail(*traceIte);
                }
            }
        }
#ifdef ASSERT_ON
        //check path valid
        ASSERT(carTrace.Size() > 0);
        Cross* frontCross = car->GetCar()->GetFromCross();
        int frontRoad = -1;
        for (auto ite = carTrace.Head(); ite != carTrace.Tail(); ite++)
        {
            Road* road = Scenario::Roads()[*ite];
            ASSERT(road != 0);
            ASSERT(road->GetId() >= 0);
            ASSERT(road->GetId() != frontRoad);
            ASSERT(road->CanStartFrom(frontCross->GetId()));
            frontCross = road->GetPeerCross(frontCross);
        }
        //ASSERT(frontCross->GetId() == car->GetCar()->GetToCrossId());
#endif
    }
}


void SchedulerFloyd::DoHandleGetoutGarage(const int& time, SimScenario& scenario, SimCar* car)
{
    if (m_deadLockSolver.IsGarageLockedInBackup(time))
        return;

    int currentLimit = m_carLimit;
    int carOnRoad = scenario.GetOnRoadCarsN();
    if (m_isLimitedByRoadSizeCount)
    {
        ASSERT(m_carsNumOnRoadLimit > 0);
        currentLimit = carOnRoad >= m_carsNumOnRoadLimit ? m_carLimitTighter : m_carLimit;
    }

    if (m_isFasterAtEndStep)
    {
        ASSERT(m_carsNumOnRoadLimit > 0);
        if (m_notArrivedPresetCars.size() == 0 && m_lastPresetVipCarRealTime != -1 && carOnRoad <= m_carsNumOnRoadLimit)
        {
            currentLimit = m_carLimitLooser * 4.0;
        }
    }

    if (m_isLessCarAfterDeadLock)
    {
        if (time <= m_deadLockSolver.GetDeadLockTime())
        {
            currentLimit = m_carLimitTighter;
        }
    }

    if (m_isOptimalForLastVipCar)
    {
        if (m_lastPresetVipCarRealTime != -1 && (time >= m_lastPresetVipCarRealTime - 100 && time <= m_lastPresetVipCarEstimateArriveTime - 100))
        {
            if (!car->GetCar()->GetIsVip())
            {
                
                if (carOnRoad >= m_carsNumOnRoadLimit / 2.0)
                {
                    currentLimit = m_carLimitTighter;
                }
                
                //car->SetRealTime(time + 1);
                //return;
            }
            else
            {
                currentLimit = m_carLimitLooser;
            }
        }
    }  

    int roadId = car->GetNextRoadId();
    ASSERT(roadId >= 0);
    SimRoad* road = scenario.Roads()[roadId];
    int crossId = car->GetCar()->GetFromCrossId();
    int carSum = 0;
    for (int i = 1; i <= road->GetRoad()->GetLanes(); i++)
    {
        carSum += road->GetCarsFrom(i, crossId).size();
    }
    double carAver = double(carSum) / double(road->GetRoad()->GetLanes());
    if (!m_isVipCarDispatchFree)
    {
        if ((carAver > currentLimit)
            || ((!car->GetCar()->GetIsVip() && car->GetCar()->GetMaxSpeed() > garageMinSpeed[crossId])))
        {
            car->SetRealTime(time + 1);
        }
    }
}

#include "random.h"
void SchedulerFloyd::DoHandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result)
{
    if (result.Conflict)
    {
        if (m_deadLockSolver.HandleDeadLock(time, scenario))
        {
            result.Conflict = false; //retry
            RefreshNotArrivedPresetCars(scenario);
        }
    }
    else
    {
        if (time > 0 && time % 200 == 0)
        {
            m_deadLockSolver.Backup(time, scenario);
        }
    }
}

std::pair<int, bool> SchedulerFloyd::SelectBestRoad(SimScenario& scenario, const std::vector<int>& list, SimCar* car)
{
    auto ret = UpdateCarTraceByDijkstra(0, scenario, list, car);
    ASSERT(ret);
    return std::make_pair(1, true);
}

bool SchedulerFloyd::UpdateCarTraceByDijkstra(const int& time, const SimScenario& scenario, SimCar* car) const
{
    ASSERT(!car->GetIsInGarage());
    ASSERT(!car->GetIsReachedGoal());
    std::vector<int> validFirstHop;
    Cross* cross = car->GetCurrentCross();
    DirectionType_Foreach(dir,
        Road* road = cross->GetRoad(dir);
        if (road != 0 && road != car->GetCurrentRoad())
            validFirstHop.push_back(road->GetId());
    );
    return UpdateCarTraceByDijkstra(time, scenario, validFirstHop, car);
}

bool SchedulerFloyd::UpdateCarTraceByDijkstra(const int& time, const SimScenario& scenario, const std::vector<int>& validFirstHop, SimCar* car) const
{
    ASSERT(!car->GetIsInGarage());
    ASSERT(!car->GetIsReachedGoal());
    ASSERT(validFirstHop.size() > 0);

    /* Dijkstra weight */
    static uint crossSize = 0;
    static std::vector< std::vector<double> > lengthMap;
    static std::vector<int> lengthList;
    static std::vector<bool> visitedList;
    static std::vector<int> pathLastCrossId;
    
    if (Scenario::Crosses().size() != crossSize)
    {
        crossSize = Scenario::Crosses().size();
        lengthMap.resize(crossSize);
        lengthList.resize(crossSize);
        visitedList.resize(crossSize);
        pathLastCrossId.resize(crossSize);
        for (uint i = 0; i < crossSize; ++i)
        {
            lengthMap[i].resize(crossSize);
        }
    }

    int from = car->GetCurrentCross()->GetId();
    int to = car->GetCar()->GetToCrossId();

    //initialize
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        for (uint jCross = 0; jCross < crossSize; ++jCross)
        {
            lengthMap[iCross][jCross] = Inf;
        }
    }

    //calculate
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        
        if (iCross == from)
        {
            Cross* cross = Scenario::Crosses()[from];
            for (uint i = 0; i < validFirstHop.size(); ++i)
            {
                Road* road = Scenario::Roads()[validFirstHop[i]];
                SimRoad* roadLink = scenario.Roads()[road->GetId()];
                ASSERT(road->GetStartCrossId() == from ||
                    (road->GetEndCrossId() == from && road->GetIsTwoWay()));
                Cross* nearCross = road->GetPeerCross(cross);
                int carAverager = 0;
                for (int j = 1; j <= roadLink->GetRoad()->GetLanes(); j++)
                {
                    carAverager += roadLink->GetCarsFrom(j, from).size();
                }
                carAverager = carAverager / roadLink->GetRoad()->GetLanes();
                int roadLength = road->GetLength();
                lengthMap[from][nearCross->GetId()] = ((double)roadLength) * m_lengthWeight + (double)carAverager * (double)carAverager * (double)carAverager / (double)roadLength;//m_carNumWeight;
            }
        }
        else
        {
            for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
            {
                Cross* cross = Scenario::Crosses()[iCross];
                Road* road = cross->GetRoad((Cross::DirectionType)i);
                if (road != 0)
                {
                    SimRoad* roadLink = scenario.Roads()[road->GetId()];
                    if (roadLink->GetRoad()->GetStartCrossId() == cross->GetId() ||
                        (roadLink->GetRoad()->GetEndCrossId() == cross->GetId() && roadLink->GetRoad()->GetIsTwoWay()))
                    {
                        Cross* peerlink = roadLink->GetRoad()->GetPeerCross(cross);
                        int carAver = 0;
                        int roadLanes = roadLink->GetRoad()->GetLanes();
                        for (int j = 1; j <= roadLink->GetRoad()->GetLanes(); j++)
                        {
                            carAver += roadLink->GetCarsFrom(j, cross->GetId()).size();
                        }
                        carAver = carAver / roadLink->GetRoad()->GetLanes();
                        int roadLength = cross->GetRoad((Cross::DirectionType)i)->GetLength();
                        lengthMap[cross->GetId()][peerlink->GetId()] = ((double)roadLength)* m_lengthWeight + (double)carAver * (double)carAver * (double)carAver / (double)roadLength;//m_carNumWeight;
                    }
                }
            }
        }
    }

    lengthMap[from][car->GetCurrentRoad()->GetPeerCross(car->GetCurrentCross())->GetId()] = Inf;
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        if (from != iCross && lengthMap[from][iCross] > 0)
        {
            lengthList[iCross] = lengthMap[from][iCross];
            pathLastCrossId[iCross] = from;
        }
        else
        {
            lengthList[iCross] = Inf;
            pathLastCrossId[iCross] = -1;
        }
        visitedList[iCross] = false;
        pathLastCrossId[from] = from;
        lengthList[from] = 0;
    }

    visitedList[from] = true;
    for (uint iExtend = 0; iExtend < crossSize - 1; ++iExtend)
    {
        int min = Inf;
        int visited = -1;
        for (uint iMin = 0; iMin < crossSize; ++iMin)
        {
            if (!visitedList[iMin] && lengthList[iMin] < min)
            {
                min = lengthList[iMin];
                visited = iMin;
            }
        }

        ASSERT(visited != -1);
        visitedList[visited] = true;
        for (uint iUpdate = 0; iUpdate < crossSize; ++iUpdate)
        {
            if (visitedList[iUpdate] == false && lengthMap[visited][iUpdate] > 0 && (min + lengthMap[visited][iUpdate]) < lengthList[iUpdate])
            {
                

                
                lengthList[iUpdate] = min + lengthMap[visited][iUpdate];
                pathLastCrossId[iUpdate] = visited;
            }
        }
    }

    static std::vector<int> crossListDiji;
    static std::vector<int> pathListDiji;
    crossListDiji.clear();
    pathListDiji.clear();

    int endCrossId = to;
    while (endCrossId != from)
    {
        crossListDiji.push_back(endCrossId);
        endCrossId = pathLastCrossId[endCrossId];
    }
    //crossListDiji.push_front(endCrossId);

    //check path valid
    Cross* lastCross = car->GetCurrentCross();
    for (int i = crossListDiji.size() - 1; i >= 0; --i)
    {
        bool consistant = false;
        Cross* thisCross = Scenario::Crosses()[crossListDiji[i]];
        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
        {
            Road* road = lastCross->GetRoad((Cross::DirectionType)i);
            if (road != 0)
            {
                if ((road->GetStartCrossId() == lastCross->GetId() && road->GetEndCrossId() == thisCross->GetId())
                    || (road->GetEndCrossId() == lastCross->GetId() && road->GetStartCrossId() == thisCross->GetId() && road->GetIsTwoWay()))
                {
                    consistant = true;
                    pathListDiji.push_back(road->GetId());
                    break;
                }
            }
        }
        ASSERT_MSG(consistant, "can not find the road bewteen " << lastCross->GetId() << " and " << thisCross->GetId());
        lastCross = thisCross;
    }

    auto& carTrace = car->GetTrace();
    carTrace.Clear(car->GetCurrentTraceIndex());
    for (auto traceIte = pathListDiji.begin(); traceIte != pathListDiji.end(); traceIte++)
    {
        //ASSERT(carTrace.Size() == 0 || (*(carTrace.Tail() - 1) != *traceIte));
        carTrace.AddToTail(*traceIte);
    }
    return true;
}

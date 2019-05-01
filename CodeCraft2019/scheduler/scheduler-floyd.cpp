#include "scheduler-floyd.h"
#include "scenario.h"
#include "assert.h"
#include "log.h"
#include <algorithm>
#include "tactics.h"
#include "simulator.h"

/* Interfaces
    Trace& trace = Tactics::Instance.GetTraces()[car->GetCar()->GetId()];
*/

SchedulerFloyd::SchedulerFloyd()
    : m_updateInterval(2)
    , m_lastVipCarRealTime(0)
    , m_carsNumOnRoadLimit(-1)
{ 
    SetLengthWeight(0.1);
    SetCarNumWeight(0.9);
    SetLanesNumWeight(0.2);
    SetCarLimit(2.1);
    SetPresetVipTracePreloadWeight(1.0);
    SetLastPresetVipCarEstimateArriveTime(0);
    SetLooserCarsNumOnRoadLimit(0);
    SetIsEnableVipWeight(false);
    SetIsOptimalForLastVipCar(true);
    SetIsLimitedByRoadSizeCount(true);
    SetIsFasterAtEndStep(true);
    SetIsLessCarAfterDeadLock(false);
    SetIsDropBackByDijkstra(false);
    SetIsVipCarDispatchFree(false);
    //wsq
    
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
    m_carLimitTighter = v * 0.5;
}

void SchedulerFloyd::SetPresetVipTracePreloadWeight(double v)
{
    m_presetVipTracePreloadWeight = v;
}

void SchedulerFloyd::SetVipCarOptimalStartTime(int v)
{
    m_vipCarOptimalStartTime = v;
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

void SchedulerFloyd::SetLooserCarsNumOnRoadLimit(int v)
{
    m_looserCarsNumOnRoadLimit = v;
}

bool IsProtected(const SimCar* car)
{
    return car->GetCar()->GetIsVip();
}

bool CompareVipCarsFloyd(SimCar* a, SimCar* b)
{
    int ta = a->CalculateSpendTime(true);
    int tb = b->CalculateSpendTime(true);
    return ta != tb ? ta > tb : a->GetCar()->GetOriginId() < b->GetCar()->GetOriginId();
}

void SchedulerFloyd::HandlePresetCars(SimScenario& scenario, const int& canChangesN)
{
    std::vector<SimCar*> presetVipCars;
    presetVipCars.reserve(Scenario::GetVipCarsN());
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car!= 0 && car->GetCar()->GetIsPreset())// && car->GetCar()->GetIsVip())
        { 
            presetVipCars.push_back(car);
        }
    }
    std::sort(presetVipCars.begin(), presetVipCars.end(), CompareVipCarsFloyd);
    //wsq test
    for (int i = 0; i < canChangesN; ++i)
    {
        presetVipCars[i]->SetCanChangePath(true);
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
    m_lastVipCarRealTime = -1;
    m_lastPresetVipCarEstimateArriveTime = -1;
    int vipCarNumInPreset = 0;
    int vipStartTime = 0;
    int vipPresetArriveSpendTime = 0;
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car != 0)
        {
            if (car->GetCar()->GetIsVip())
            {
                int realTime = car->GetRealTime();
                if (m_lastVipCarRealTime < 0 || realTime > m_lastVipCarRealTime)
                    m_lastVipCarRealTime = realTime;
                if (car->GetCar()->GetIsPreset())
                {
                    vipCarNumInPreset++;
                    int arriveTime = realTime + car->CalculateSpendTime(true);
                    if (m_lastPresetVipCarEstimateArriveTime < 0 || arriveTime > m_lastPresetVipCarEstimateArriveTime)
                        m_lastPresetVipCarEstimateArriveTime = arriveTime;
                    vipPresetArriveSpendTime += car->CalculateSpendTime(true);
                }
                vipStartTime += car->GetRealTime();
            }
        }
    }
    if (Scenario::GetVipCarsN() > 0)
    {
        vipStartTime /= Scenario::GetVipCarsN();
        vipPresetArriveSpendTime /= vipCarNumInPreset;
        int vipProtectedTimeSpan = Scenario::GetVipCarsN() * vipPresetArriveSpendTime / 25 / (m_lastVipCarRealTime - vipStartTime);
        vipProtectedTimeSpan = std::min(100, std::max(40, vipProtectedTimeSpan));
        m_vipCarTraceProtectedStartTime = std::max(0, m_lastVipCarRealTime - vipProtectedTimeSpan);
    }

    int vipCarOptimalTime = Scenario::GetVipCarsN() / m_lastPresetVipCarEstimateArriveTime;
    m_vipCarOptimalStartTime = m_lastPresetVipCarEstimateArriveTime - std::max(vipCarOptimalTime * 10, std::max(80, vipCarNumInPreset / 10));
    LOG(" VipCarNum = " << Scenario::GetVipCarsN() <<
        " lastPresetVipCarEstimateArriveTime = " << m_lastPresetVipCarEstimateArriveTime <<
        " vipCarOptimalTime = " << vipCarOptimalTime <<
        " vipCarNumInPreset = " << vipCarNumInPreset <<
        " m_vipCarOptimalStartTime = " << m_vipCarOptimalStartTime);
}

void SchedulerFloyd::RefreshNotArrivedPresetCars(SimScenario& scenario)
{
    m_notArrivedProtectedCars.clear();
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car != 0 && !car->GetIsReachedGoal() && IsProtected(car))
            m_notArrivedProtectedCars.insert(car->GetCar()->GetId());
    }
}

void SchedulerFloyd::HandleSimCarScheduled(const SimCar* car)
{
    //wsq
    static int maxWaitTime = 0;
    
    int waitTime = car->GetLastUpdateTime() - car->GetLockOnNextRoadTime();
    if (waitTime > maxWaitTime)
    {
        if (car->GetIsLockOnNextRoad())
        {
            maxWaitTime = waitTime;
        }
    }
}


void SchedulerFloyd::DoInitialize(SimScenario& scenario)
{
    int canChangesN = Scenario::GetPresetCarsN() * 0.1;
    int forDeadLockSolver = canChangesN * 0.5;
    HandlePresetCars(scenario, canChangesN - forDeadLockSolver);
    m_deadLockSolver.SetCanChangedPresetN(forDeadLockSolver);

    uint crossCount = Scenario::Crosses().size();
    int roadCount = Scenario::Roads().size();

    CalculateWeight(scenario);

    if (m_isFasterAtEndStep)
    {
        RefreshNotArrivedPresetCars(scenario);
    }

    LOG("Car Limit = " << m_carLimit);
    m_weightCrossToCross.resize(crossCount);
    m_connectionCrossToCross.resize(crossCount);
    m_minPathCrossToCross.resize(crossCount);
    m_appointOnRoadCounter.resize(roadCount, std::make_pair(0, 0));
    m_garageMinSpeed.resize(crossCount, std::make_pair(-1, -1));
    m_timeWeightForRoad.resize(roadCount);
    m_garageTraceSizeLimit.resize(crossCount, std::make_pair(-1, -1));
    m_garagePlanCarNum.resize(crossCount, 0);
    for (uint i = 0; i < crossCount; i++)
    {
        m_weightCrossToCross[i].resize(crossCount);
        m_connectionCrossToCross[i].resize(crossCount);
        m_minPathCrossToCross[i].resize(crossCount);
    }
    for (uint i = 0; i < roadCount; i++)
    {
        m_timeWeightForRoad[i].resize(roadCount,std::make_pair(Inf, Inf));
    }
    m_deadLockSolver.Initialize(0, scenario);
    m_deadLockSolver.SetSelectedRoadCallback(Callback::Create(&SchedulerFloyd::SelectBestRoad, this));
    SimCar::SetUpdateGoOnNewRoadNotifier(Callback::Create(&SchedulerFloyd::HandleGoOnNewRoad, this));
    SimCar::SetUpdateCarScheduledNotifier(Callback::Create(&SchedulerFloyd::HandleSimCarScheduled, this));
    m_garageDispatchCounter.Initialize(scenario);
}

void SchedulerFloyd::HandleGoOnNewRoad(const SimCar* car, Road* oldRoad)
{
    bool reachGoal = car->GetIsReachedGoal();
    if (reachGoal)
    {
        if (IsProtected(car))
        {
            m_notArrivedProtectedCars.erase(car->GetCar()->GetId());
        }
    }
    return;
    
    if (oldRoad != 0)
    {
        Cross* oldCross = car->GetCar()->GetToCross();
        if (!reachGoal)
            oldCross = car->GetCurrentDirection() ? car->GetCurrentRoad()->GetStartCross() : car->GetCurrentRoad()->GetEndCross();
        auto& pair = m_appointOnRoadCounter[oldRoad->GetId()];
        (oldRoad->IsFromOrTo(oldCross->GetId()) ? pair.second : pair.first) -= 1;
    }
    if (!reachGoal)
    {
        int newRoadId = car->GetCurrentRoad()->GetId();
        auto& pair = m_appointOnRoadCounter[newRoadId];
        (car->GetCurrentDirection() ? pair.first : pair.second) += 1;
    }
}
void SchedulerFloyd::DoHandleBecomeFirstPriority(const int& time, SimScenario& scenario, SimCar* car)
{
    return;
    if (car->GetCurrentCross()->GetId() != car->GetCar()->GetToCrossId() && !car->GetIsLockOnNextRoad())
    {
        int nextRoadId = car->GetNextRoadId();
        SimRoad* roadLink = scenario.Roads()[nextRoadId];
        int roadLines = roadLink->GetRoad()->GetLanes();
        double carAver = 0;
        for (int j = 1; j <= roadLines; j++)
        {
            carAver += (double)roadLink->GetCarsFrom(j, car->GetCurrentCross()->GetId()).size();
        }
        carAver = carAver / (double)roadLink->GetRoad()->GetLanes();
        if (carAver >= (double)roadLink->GetRoad()->GetLength() - 3.0)
        {
            std::vector<int> validFirstHop;
            Cross* cross = car->GetCurrentCross();
            for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
            {
                Road* road = cross->GetRoad((Cross::DirectionType)i);
                if (road != 0 && road != car->GetCurrentRoad() && road != roadLink->GetRoad() && road->CanStartFrom(cross->GetId()))
                {
                    SimRoad* roadLinkNew = scenario.Roads()[road->GetId()];
                    int roadLines = roadLinkNew->GetRoad()->GetLanes();
                    double carAverCal = 0;
                    for (int j = 1; j <= roadLines; j++)
                    {
                        carAverCal += (double)roadLinkNew->GetCarsFrom(j, car->GetCurrentCross()->GetId()).size();
                    }
                    carAverCal = carAverCal / (double)roadLinkNew->GetRoad()->GetLanes();
                    if (carAverCal < (double)roadLinkNew->GetRoad()->GetLength() - 3.0)
                    {
                        validFirstHop.push_back(road->GetId());
                    }
                }
                
            }
            if (validFirstHop.size() > 0)
                UpdateCarTraceByDijkstra(time, scenario, validFirstHop, car);
        }
    }
    
}

void SchedulerFloyd::DoUpdate(int& time, SimScenario& scenario)
{
    //m_appointOnRoadCounter
    if (!m_deadLockSolver.NeedUpdate(time))
    {
        m_garageDispatchCounter.Update(time, scenario);
        return;
    }

    uint crossSize = Scenario::Crosses().size();
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        //int carNum = 0;
        if(m_notArrivedProtectedCars.size() == 0)
            m_garagePlanCarNum[iCross] = m_garagePlanCarNum[iCross] == 0 ? 1 : m_garagePlanCarNum[iCross] + 1;
        int minSpeedInGarage = -1;
        int minVipSpeedInGarage = -1;
        auto& garage = scenario.Garages()[iCross];
        for (uint iCar = 0; iCar < garage.size(); ++iCar)
        {
            if (garage[iCar] != 0 && garage[iCar]->GetIsInGarage() && !garage[iCar]->GetCar()->GetIsPreset())
            {
                //carNum++;
                if (garage[iCar]->GetRealTime() > time)
                    continue;
                if (minSpeedInGarage > garage[iCar]->GetCar()->GetMaxSpeed() || minSpeedInGarage < 0)
                {
                    minSpeedInGarage = garage[iCar]->GetCar()->GetMaxSpeed();
                    m_garageMinSpeed[iCross].first = minSpeedInGarage;
                }
                if (garage[iCar]->GetCar()->GetIsVip())
                {
                    if (minVipSpeedInGarage > garage[iCar]->GetCar()->GetMaxSpeed() || minVipSpeedInGarage < 0)
                    {
                        minVipSpeedInGarage = garage[iCar]->GetCar()->GetMaxSpeed();
                        m_garageMinSpeed[iCross].second = minVipSpeedInGarage;
                    }
                }
            }
        }
        for (uint jCross = 0; jCross < crossSize; ++jCross)
        {
            m_weightCrossToCross[iCross][jCross] = Inf;
            m_connectionCrossToCross[iCross][jCross] = jCross;
        }
    }
    if (time % m_updateInterval != 0)
    {
        m_garageDispatchCounter.Update(time, scenario);
        return;
    }
    if (m_isEnableVipWeight)
    {
        if (m_notArrivedProtectedCars.size() > 0)
        {
            bool noPreVipCar = true;
            for (uint iCar = 0; iCar < scenario.Cars().size(); ++iCar)
            {
                SimCar* car = scenario.Cars()[iCar];
                if (car == 0) continue;
                if (car->GetIsReachedGoal()) continue;
                //time >= m_lastPresetVipCarRealTime - 50 && 
                if (car->GetCar()->GetIsVip() && ((car->GetCar()->GetIsPreset() && !car->GetCanChangePath() && time >= car->GetRealTime() - 50)))// || (time >= m_lastPresetVipCarEstimateArriveTime && !car->GetCar()->GetIsPreset() && !car->GetIsInGarage())))
                {
                    noPreVipCar = false;
                    auto& carTrace = car->GetTrace();
                    Cross* lastCross = car->GetCurrentCross();
                    for (uint iTrace = car->GetCurrentTraceIndex(); iTrace < carTrace.Size(); ++iTrace)
                    {
                        Road* currentRoad = Scenario::Roads()[carTrace[iTrace]];
                        Cross* nextCross = currentRoad->GetPeerCross(lastCross);
                        auto& updatem_weightCrossToCross = m_weightCrossToCross[lastCross->GetId()][nextCross->GetId()];
                        ASSERT(nextCross->GetId() == currentRoad->GetPeerCross(lastCross)->GetId());
                        if (updatem_weightCrossToCross == Inf)
                        {
                            updatem_weightCrossToCross = m_presetVipTracePreloadWeight;
                        }
                        else
                        {
                            updatem_weightCrossToCross += m_presetVipTracePreloadWeight;
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
        double roadNumConnectWithCross = 0;
        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
        {
            Road* road = cross->GetRoad((Cross::DirectionType)i);
            if (road != 0)
            {
                SimRoad* roadLink = scenario.Roads()[road->GetId()];
                if (roadLink->GetRoad()->CanStartFrom(cross->GetId()))
                {
                    roadNumConnectWithCross += 1.0;
                }
            }
        }
        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
        {
            Road* road = cross->GetRoad((Cross::DirectionType)i);
            if (road != 0)
            {
                SimRoad* roadLink = scenario.Roads()[road->GetId()];
                if (roadLink->GetRoad()->CanStartFrom(cross->GetId()))
                {
                    Cross* peerlink = roadLink->GetRoad()->GetPeerCross(cross);
                    auto& updatem_weightCrossToCross = m_weightCrossToCross[iCross][peerlink->GetId()];
                    //wsq
                    double carAver = 0;         
                    if (updatem_weightCrossToCross != Inf)
                    {
                        carAver += m_weightCrossToCross[iCross][peerlink->GetId()];
                    }
                    int roadLines = roadLink->GetRoad()->GetLanes();                   
                    for (int j = 1; j <= roadLines; j++)
                    {
                        carAver += (double)roadLink->GetCarsFrom(j, iCross).size();
                    }
                    carAver = carAver/ (double)roadLink->GetRoad()->GetLanes();
                    //wsq
                    
                    if (roadLines == 1)
                    {
                        carAver = carAver + 6.0;
                    }
                    if (roadLines == 2)
                    {
                        carAver = carAver + 2.0;
                    }   
                    carAver += (4.0 - roadNumConnectWithCross) * 4.0;
                    int roadLength = cross->GetRoad((Cross::DirectionType)i)->GetLength();
                    m_weightCrossToCross[iCross][peerlink->GetId()] = (double)roadLength * m_lengthWeight + carAver * carAver * carAver / (double)roadLength;
                    ASSERT(m_weightCrossToCross[iCross][peerlink->GetId()] != Inf);
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
                double lengthAfterTran = m_weightCrossToCross[iRow][iTransfer] + m_weightCrossToCross[iTransfer][iColumn];
                if (m_weightCrossToCross[iRow][iColumn] > lengthAfterTran)
                {
                    m_weightCrossToCross[iRow][iColumn] = lengthAfterTran;
                    ASSERT(m_weightCrossToCross[iRow][iColumn] != Inf);
                    m_connectionCrossToCross[iRow][iColumn] = iTransfer;
                }
            }
        }
    }
    
    for (uint iRow = 0; iRow < crossSize; ++iRow)
    {
        for (uint iColumn = 0; iColumn < crossSize; ++iColumn)
        {
            ASSERT(m_weightCrossToCross[iRow][iColumn] != Inf);
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
                int transstep = m_connectionCrossToCross[startStep][iEnd];
                while (m_connectionCrossToCross[startStep][transstep] != transstep)
                {
                    transstep = m_connectionCrossToCross[startStep][transstep];
                }
                startStep = transstep;
                ASSERT(m_weightCrossToCross[startStep][iEnd] != Inf);
                crossList.push_back(startStep);
            }

            //trans crosses to roads
            auto& pathList = m_minPathCrossToCross[iStart][iEnd];
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
            && (!car->GetCar()->GetIsPreset() || car->GetCanChangePath())
            && !m_deadLockSolver.IsCarTraceLockedInBackup(car))
        {
            int from = car->GetCar()->GetFromCrossId();
            if (!car->GetIsInGarage() && car->GetCurrentRoad() != 0)
                from = car->GetCurrentCross()->GetId();
            int to = car->GetCar()->GetToCrossId();
            const auto* newTrace = &m_minPathCrossToCross[from][to];

            if (!car->GetIsLockOnNextRoad())
            {
                if (car->GetIsInGarage())
                    carTrace.Clear();
                else
                {
                    if (!(carTrace.Size() > 0 && newTrace->size() > 0 && *newTrace->begin() == car->GetCurrentRoad()->GetId()))
                    {
                        carTrace.Clear(car->GetCurrentTraceIndex());
                    }
                    //drop back
                    if (m_isDropBackByDijkstra && (carTrace.Size() > 0 && newTrace->size() > 0 && *newTrace->begin() == car->GetCurrentRoad()->GetId()))
                    {
                        UpdateCarTraceByDijkstra(time, scenario, car);
                    }
                }
            }

            if (!car->GetIsLockOnNextRoad()  //will be updated
                && carTrace.Size() != 0 && newTrace != 0 && newTrace->size() > 0 //on the road
                && newTrace == &m_minPathCrossToCross[from][to]) //and no drop back
                ASSERT(*(carTrace.Tail() - 1) != *newTrace->begin()); //check next jump
            if (!car->GetIsLockOnNextRoad() && (!(carTrace.Size() > 0 && newTrace->size() > 0 && *newTrace->begin() == car->GetCurrentRoad()->GetId()))) //can not update road if locked
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
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        int maxCarTraceSizeInGarage = -1;
        int maxVipCarTraceSizeInGarage = -1;
        auto& garage = scenario.Garages()[iCross];
        for (uint iCar = 0; iCar < garage.size(); ++iCar)
        {
            if (garage[iCar] != 0 && garage[iCar]->GetIsInGarage() && !garage[iCar]->GetCar()->GetIsPreset())
            {
                if (garage[iCar]->GetRealTime() > time )
                    continue;
                if (garage[iCar]->GetCar()->GetMaxSpeed() <= m_garageMinSpeed[iCross].first)
                {
                    if (maxCarTraceSizeInGarage < garage[iCar]->GetTrace().Size() || maxCarTraceSizeInGarage < 0)
                    {
                        maxCarTraceSizeInGarage = garage[iCar]->GetTrace().Size();
                        m_garageTraceSizeLimit[iCross].first = maxCarTraceSizeInGarage;
                    }
                }
                if (garage[iCar]->GetCar()->GetIsVip() && garage[iCar]->GetCar()->GetMaxSpeed() <= m_garageMinSpeed[iCross].second)
                {
                    if (maxVipCarTraceSizeInGarage < garage[iCar]->GetTrace().Size() || maxVipCarTraceSizeInGarage < 0)
                    {
                        maxCarTraceSizeInGarage = garage[iCar]->GetTrace().Size();
                        m_garageTraceSizeLimit[iCross].second = maxCarTraceSizeInGarage;
                    }
                }
            }
        }
    }
    m_garageDispatchCounter.Update(time, scenario);
}

void SchedulerFloyd::DoHandleBeforeGarageDispatch(const int& time, SimScenario& scenario)
{
    m_garageDispatchCounter.HandleBeforeGarageDispatch(time, scenario);
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
        currentLimit = carOnRoad >= (m_carsNumOnRoadLimit + m_looserCarsNumOnRoadLimit) ? 0 : m_carLimitLooser * 15.0;
        /*
        if (car->GetCar()->GetIsVip())
        {
            currentLimit = m_carLimitLooser * 20.0;
        }
        */
    }
    if (m_isFasterAtEndStep)
    {
        ASSERT(m_carsNumOnRoadLimit > 0);
        if (m_notArrivedProtectedCars.size() == 0 && m_lastVipCarRealTime != -1 && carOnRoad <= (m_carsNumOnRoadLimit + m_looserCarsNumOnRoadLimit))
        {
            currentLimit = m_carLimitLooser * 15.0;
        }
        if (m_notArrivedProtectedCars.size() == 0 && m_lastVipCarRealTime != -1 && scenario.GetCarInGarageN() <= (m_carsNumOnRoadLimit + m_looserCarsNumOnRoadLimit))
        {
            currentLimit = m_carLimitLooser * 15.0;
        }
    }

    if (m_isLessCarAfterDeadLock)
    {
        if (time <= m_deadLockSolver.GetDeadLockTime())
        {
            currentLimit = m_carLimit * 1.5;
        }
    }

    if (m_isOptimalForLastVipCar)
    {
        //m_vipCarTraceProtectedStartTime
        if (m_lastVipCarRealTime != -1 && (time >= 50 && m_notArrivedProtectedCars.size() != 0))
        {             
            if (!car->GetCar()->GetIsVip())
            {   
                if (!car->GetCar()->GetIsPreset())
                {
                    car->SetRealTime(time + 1);
                    return;
                }             
            }     
            else
            {
                if(carOnRoad <= (m_carsNumOnRoadLimit + m_looserCarsNumOnRoadLimit))
                {
                    currentLimit = m_carLimitLooser * 15.0;
                }
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
            || ((!car->GetCar()->GetIsVip() && car->GetCar()->GetMaxSpeed() > m_garageMinSpeed[crossId].first))
            || ((car->GetCar()->GetIsVip() && car->GetCar()->GetMaxSpeed() > m_garageMinSpeed[crossId].second))
            || ((!car->GetCar()->GetIsVip() && (double)car->GetTrace().Size() < (double)m_garageTraceSizeLimit[crossId].first * 0.90))
            || ((car->GetCar()->GetIsVip() && (double)car->GetTrace().Size() < (double)m_garageTraceSizeLimit[crossId].second * 0.90))
            )
        {    
            if (!car->GetCar()->GetIsPreset())
            {
                car->SetRealTime(time + 1);
                return;
            }
        }
    }
    /*
    if (!car->GetCar()->GetIsPreset())
    {

        if (carOnRoad >= m_carsNumOnRoadLimit + 2000|| !m_garageDispatchCounter.CanDispatch(time, scenario, car))
        {
            car->SetRealTime(time + 1);
            return;
        }
    }
    if (Simulator::Instance.CanCarGetOutFromGarage(time, scenario, car).first > 0)
        m_garageDispatchCounter.NotifyDispatch(car->GetCar()->GetFromCrossId());
    return;
    */
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
        if (time > 0 && time % 100 == 0)
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
        if (road != 0 && road != car->GetCurrentRoad() && road->CanStartFrom(cross->GetId()))
            validFirstHop.push_back(road->GetId());
    );
    return UpdateCarTraceByDijkstra(time, scenario, validFirstHop, car);
}

bool SchedulerFloyd::UpdateCarTraceByDijkstra(const int& time, const SimScenario& scenario, const std::vector<int>& validFirstHop, SimCar* car) const
{
    //ASSERT(!car->GetIsInGarage());
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
    int from = car->GetCar()->GetFromCrossId();
    if (!car->GetIsInGarage())
        from = car->GetCurrentCross()->GetId();       
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
                        lengthMap[cross->GetId()][peerlink->GetId()] = ((double)roadLength) * m_lengthWeight + (double)carAver * (double)carAver * (double)carAver / (double)roadLength;//m_carNumWeight;
                    }
                }
            }
        }
    }
    if (!car->GetIsInGarage())
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
    Cross* lastCross = car->GetCar()->GetFromCross(); 
    if (!car->GetIsInGarage())
        lastCross = car->GetCurrentCross();
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
    if (car->GetIsInGarage())
    {
        carTrace.Clear();
    }
    else
    {
        carTrace.Clear(car->GetCurrentTraceIndex());
    }
    for (auto traceIte = pathListDiji.begin(); traceIte != pathListDiji.end(); traceIte++)
    {
        //ASSERT(carTrace.Size() == 0 || (*(carTrace.Tail() - 1) != *traceIte));
        carTrace.AddToTail(*traceIte);
    }
    return true;
}


void SchedulerFloyd::UpdateTimeWeight(SimCar* car)
{
    auto& carTrace = car->GetTrace();
    Cross* nextCross = car->GetCurrentCross();
    Road* currentRoad = car->GetCurrentRoad();
    Cross* lastCross = currentRoad->GetPeerCross(nextCross);
    int traceIndex = 0;
    auto& updatem_timeWeightForRoad = m_timeWeightForRoad[currentRoad->GetId()][traceIndex];
    if (currentRoad->IsFromOrTo(lastCross->GetId()))
    {
        updatem_timeWeightForRoad.first = updatem_timeWeightForRoad.first == Inf ? 1 : updatem_timeWeightForRoad.first + 1;
    }
    else
    {
        updatem_timeWeightForRoad.second = updatem_timeWeightForRoad.second == Inf ? 1 : updatem_timeWeightForRoad.second + 1;
    }
    if (nextCross->GetId() == car->GetCar()->GetToCrossId())
        return;
    for (uint iTrace = car->GetCurrentTraceIndex(); iTrace < carTrace.Size(); ++iTrace)
    {
        traceIndex++;
        lastCross = nextCross;
        currentRoad = Scenario::Roads()[carTrace[iTrace]];
        Cross* nextCross = currentRoad->GetPeerCross(lastCross);
        auto& updatem_timeWeightForRoad = m_timeWeightForRoad[currentRoad->GetId()][traceIndex];
        ASSERT(nextCross->GetId() == currentRoad->GetPeerCross(lastCross)->GetId());
        if (currentRoad->IsFromOrTo(lastCross->GetId()))
        {
            updatem_timeWeightForRoad.first = updatem_timeWeightForRoad.first == Inf ? 1 : updatem_timeWeightForRoad.first + 1;
        }
        else
        {
            updatem_timeWeightForRoad.second = updatem_timeWeightForRoad.second == Inf ? 1 : updatem_timeWeightForRoad.second + 1;
        }
    }
}

bool SchedulerFloyd::UpdateCarTraceByDijkstraWithTimeWeight(const int& time, const SimScenario& scenario, const std::vector<int>& validFirstHop, SimCar* car) const
{
    //ASSERT(!car->GetIsInGarage());
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
    int from = car->GetCar()->GetFromCrossId();
    if (!car->GetIsInGarage())
        from = car->GetCurrentCross()->GetId();
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
                        lengthMap[cross->GetId()][peerlink->GetId()] = ((double)roadLength) * m_lengthWeight / 2.0 + (double)carAver * (double)carAver * (double)carAver / (double)roadLength;//m_carNumWeight;
                    }
                }
            }
        }
    }
    if (!car->GetIsInGarage())
        lengthMap[from][car->GetCurrentRoad()->GetPeerCross(car->GetCurrentCross())->GetId()] = Inf;
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        if (from != iCross && lengthMap[from][iCross] > 0)
        {
            Road* roadForUpdate = 0;
            Cross* initCross = Scenario::Crosses()[from];
            DirectionType_Foreach(dir,
                Road* road = initCross->GetRoad(dir);
            if (road != 0 && road->CanStartFrom(initCross->GetId()) && road->GetPeerCross(initCross)->GetId() == iCross)
                roadForUpdate = road;
            );
            double roadFromOrTo = m_timeWeightForRoad[roadForUpdate->GetId()][0].second;
            if (roadForUpdate->IsFromOrTo(from))
                roadFromOrTo = m_timeWeightForRoad[roadForUpdate->GetId()][0].first;
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
        int traceIndex = visited;
        int traceSize = 0;
        while (traceIndex != from)
        {
            traceSize ++;
            traceIndex = pathLastCrossId[traceIndex];
        }
        for (uint iUpdate = 0; iUpdate < crossSize; ++iUpdate)
        {
            Cross* visitedCross = Scenario::Crosses()[visited];
            Road* roadForUpdate = 0;
            if (visitedList[iUpdate] == false && lengthMap[visited][iUpdate] > 0)
            {
                DirectionType_Foreach(dir,
                    Road* road = visitedCross->GetRoad(dir);
                if (road != 0 && road->CanStartFrom(visitedCross->GetId()) && road->GetPeerCross(visitedCross)->GetId() == iUpdate)
                    roadForUpdate = road;
                );
                ASSERT(roadForUpdate != 0);
                double roadFromOrTo = m_timeWeightForRoad[roadForUpdate->GetId()][traceSize + 1].second;
                if (roadForUpdate->IsFromOrTo(visited))
                    roadFromOrTo = m_timeWeightForRoad[roadForUpdate->GetId()][traceSize + 1].first;
                double sumWeightForUpdate = min + lengthMap[visited][iUpdate] + roadFromOrTo;
                if (sumWeightForUpdate < lengthList[iUpdate])
                {
                    lengthList[iUpdate] = sumWeightForUpdate;
                }
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
    Cross* lastCross = car->GetCar()->GetFromCross();
    if (!car->GetIsInGarage())
        lastCross = car->GetCurrentCross();
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
    if (car->GetIsInGarage())
    {
        carTrace.Clear();
    }
    else
    {
        carTrace.Clear(car->GetCurrentTraceIndex());
    }
    for (auto traceIte = pathListDiji.begin(); traceIte != pathListDiji.end(); traceIte++)
    {
        //ASSERT(carTrace.Size() == 0 || (*(carTrace.Tail() - 1) != *traceIte));
        carTrace.AddToTail(*traceIte);
    }
    return true;
}

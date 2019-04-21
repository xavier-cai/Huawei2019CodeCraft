#include "scheduler-dijkstra.h"
#include "scenario.h"
#include "assert.h"
#include "log.h"
#include <algorithm>
#include "tactics.h"

/* Interfaces
Trace& trace = Tactics::Instance.GetTraces()[car->GetCar()->GetId()];
*/

SchedulerDijkstra::SchedulerDijkstra()
    : m_updateInterval(2)
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
    //wsq
}

void SchedulerDijkstra::SetLengthWeight(double v)
{
    m_lengthWeight = v;
}

void SchedulerDijkstra::SetCarNumWeight(double v)
{
    m_carNumWeight = v;
}

void SchedulerDijkstra::SetLanesNumWeight(double v)
{
    m_lanesNumWeight = v;
}

void SchedulerDijkstra::SetCarLimit(double v)
{
    m_carLimit = v;
    m_carLimitLooser = v * 2.0;
    m_carLimitTighter = v * 0.25;
}

void SchedulerDijkstra::SetPresetVipTracePreloadWeight(double v)
{
    m_presetVipTracePreloadWeight = v;
}

void SchedulerDijkstra::SetLastPresetVipCarRealTime(int v)
{
    m_lastPresetVipCarRealTime = v;
}

void SchedulerDijkstra::SetLastPresetVipCarEstimateArriveTime(int v)
{
    m_lastPresetVipCarEstimateArriveTime = v;
}

void SchedulerDijkstra::SetIsEnableVipWeight(bool v)
{
    m_isEnableVipWeight = v;
}

void SchedulerDijkstra::SetIsOptimalForLastVipCar(bool v)
{
    m_isOptimalForLastVipCar = v;
}

void SchedulerDijkstra::SetIsLimitedByRoadSizeCount(bool v)
{
    m_isLimitedByRoadSizeCount = v;
}

void SchedulerDijkstra::SetIsFasterAtEndStep(bool v)
{
    m_isFasterAtEndStep = v;
}

void SchedulerDijkstra::SetIsLessCarAfterDeadLock(bool v)
{
    m_isLessCarAfterDeadLock = v;
}

void SchedulerDijkstra::SetIsDropBackByDijkstra(bool v)
{
    m_isDropBackByDijkstra = v;
}

void SchedulerDijkstra::SetIsVipCarDispatchFree(bool v)
{
    m_isVipCarDispatchFree = v;
}

bool CompareVipCarsDijkstra(SimCar* a, SimCar* b)
{
    return a->CalculateArriveTime(true) > b->CalculateArriveTime(true);
}

void SchedulerDijkstra::HandlePresetCars(SimScenario& scenario)
{
    std::vector<SimCar*> presetVipCars;
    presetVipCars.reserve(Scenario::GetVipCarsN());
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car != 0 && car->GetCar()->GetIsPreset())// && car->GetCar()->GetIsVip())
        {
            presetVipCars.push_back(car);
        }
    }
    std::sort(presetVipCars.begin(), presetVipCars.end(), CompareVipCarsDijkstra);
    //wsq test
    int canChangesN = Scenario::GetPresetCarsN() * 1.0;
    for (int i = 0; i < canChangesN; ++i)
    {
        presetVipCars[i]->SetIsForceOutput(true);
    }
}

void SchedulerDijkstra::CalculateWeight(SimScenario& scenario)
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
            int arriveTime = realTime + car->CalculateArriveTime(true);
            if (m_lastPresetVipCarRealTime < 0 || realTime > m_lastPresetVipCarRealTime)
                m_lastPresetVipCarRealTime = realTime;
            if (m_lastPresetVipCarEstimateArriveTime < 0 || arriveTime > m_lastPresetVipCarEstimateArriveTime)
                m_lastPresetVipCarEstimateArriveTime = arriveTime;
        }
    }
}

void SchedulerDijkstra::RefreshNotArrivedPresetCars(SimScenario& scenario)
{
    m_notArrivedPresetCars.clear();
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car != 0 && car->GetCar()->GetIsVip() && car->GetCar()->GetIsPreset() && !car->GetIsForceOutput() && !car->GetIsReachedGoal())
            m_notArrivedPresetCars.insert(car->GetCar()->GetId());
    }
}
const SimCar* needPrintCarDijkstra = 0;
void SchedulerDijkstra::HandleSimCarScheduled(const SimCar* car)
{
    //wsq
    static int maxWaitTime = 0;

    int waitTime = car->GetLastUpdateTime() - car->GetLockOnNextRoadTime();
    if (waitTime > maxWaitTime)
    {
        if (car->GetIsLockOnNextRoad())
        {
            maxWaitTime = waitTime;
            needPrintCarDijkstra = car;
        }
    }
}


void SchedulerDijkstra::DoInitialize(SimScenario& scenario)
{
    HandlePresetCars(scenario);

    uint crossCount = Scenario::Crosses().size();
    int roadCount = Scenario::Roads().size();

    CalculateWeight(scenario);

    if (m_isFasterAtEndStep)
    {
        RefreshNotArrivedPresetCars(scenario);
    }

    LOG("Car Limit = " << m_carLimit);
    weightCrossToCross.resize(crossCount);
    lengthMapInDoUpdate.resize(crossCount);
    connectionCrossToCross.resize(crossCount);
    minPathCrossToCross.resize(crossCount);
    appointOnRoadCounter.resize(roadCount, std::make_pair(0, 0));
    garageMinSpeed.resize(crossCount, -1);
    garagePlanCarNum.resize(crossCount, 0);
    minPathCrossToCrossDijkstra.resize(crossCount);

    for (uint i = 0; i < crossCount; i++)
    {
        weightCrossToCross[i].resize(crossCount);
        lengthMapInDoUpdate[i].resize(crossCount);
        connectionCrossToCross[i].resize(crossCount);
        minPathCrossToCross[i].resize(crossCount);
        minPathCrossToCrossDijkstra[i].resize(crossCount);
    }

    m_deadLockSolver.Initialize(0, scenario);
    m_deadLockSolver.SetSelectedRoadCallback(Callback::Create(&SchedulerDijkstra::SelectBestRoad, this));
    SimCar::SetUpdateGoOnNewRoadNotifier(Callback::Create(&SchedulerDijkstra::HandleGoOnNewRoad, this));
    SimCar::SetUpdateCarScheduledNotifier(Callback::Create(&SchedulerDijkstra::HandleSimCarScheduled,this));

    uint crossSize = Scenario::Crosses().size();
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        for (uint jCross = 0; jCross < crossSize; ++jCross)
        {
            weightCrossToCross[iCross][jCross] = Inf;
            connectionCrossToCross[iCross][jCross] = jCross;
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
                    int roadLength = cross->GetRoad((Cross::DirectionType)i)->GetLength();
                    weightCrossToCross[iCross][peerlink->GetId()] = (double)roadLength;
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
            std::vector<int> pathList; ;
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
            minPathCrossToCross[iStart][iEnd] = pathList.size();

        }
    }
}

void SchedulerDijkstra::HandleGoOnNewRoad(const SimCar* car, Road* oldRoad)
{
    bool reachGoal = car->GetIsReachedGoal();
    if (reachGoal)
    {
        if (car->GetCar()->GetIsVip())
        {
            if (car->GetCar()->GetIsPreset() && !car->GetIsForceOutput())
            {
                //LOG("PreVip" << " :" << car->GetRealTime() << " :" << time);
                m_notArrivedPresetCars.erase(car->GetCar()->GetId());
            }
            else
            {
                // LOG(car->GetCar()->GetId() << " :" << time);
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
void SchedulerDijkstra::DoHandleBecomeFirstPriority(const int& time, SimScenario& scenario, SimCar* car)
{
    return;
}
void SchedulerDijkstra::DoUpdate(int& time, SimScenario& scenario)
{
    //appointOnRoadCounter单时间片更新

    if (!m_deadLockSolver.NeedUpdate(time))
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
            if (m_notArrivedPresetCars.size() == 0)
                garagePlanCarNum[iCross] = garagePlanCarNum[iCross] == 0 ? 1 : garagePlanCarNum[iCross] + 1;
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
            lengthMapInDoUpdate[iCross][jCross] = Inf;
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
                    int roadLaneNum = road->GetLanes();
                    int roadLength = cross->GetRoad((Cross::DirectionType)i)->GetLength();
                    lengthMapInDoUpdate[iCross][peerlink->GetId()] = (double)roadLength;
                }
            }
        }
    }
    for (uint iCar = 0; iCar < scenario.Cars().size(); ++iCar)
    {
        SimCar* car = scenario.Cars()[iCar];
        if (car == 0) continue;
        if (car->GetIsReachedGoal() || car->GetIsInGarage()) continue;
        auto& carTrace = car->GetTrace();
        Road* currentRoad = car->GetCurrentRoad();
        Cross* nextCross = car->GetCurrentCross(); 
        Cross* lastCross = currentRoad->GetPeerCross(nextCross);
        ASSERT(nextCross->GetId() == currentRoad->GetPeerCross(lastCross)->GetId());
        ASSERT(lengthMapInDoUpdate[lastCross->GetId()][nextCross->GetId()] != Inf);
        if(lengthMapInDoUpdate[lastCross->GetId()][nextCross->GetId()] - (double)currentRoad->GetLength() < (double)currentRoad->GetLength()/2.0 * (double)currentRoad->GetLanes())
            lengthMapInDoUpdate[lastCross->GetId()][nextCross->GetId()] += m_presetVipTracePreloadWeight;
        for (uint iTrace = car->GetCurrentTraceIndex(); iTrace < carTrace.Size(); ++iTrace)
        {
            lastCross = nextCross;
            currentRoad = Scenario::Roads()[carTrace[iTrace]];
            nextCross = currentRoad->GetPeerCross(lastCross);
            auto& updateweightCrossToCross = lengthMapInDoUpdate[lastCross->GetId()][nextCross->GetId()];
            ASSERT(nextCross->GetId() == currentRoad->GetPeerCross(lastCross)->GetId());
            ASSERT(updateweightCrossToCross != Inf);
            if (lengthMapInDoUpdate[lastCross->GetId()][nextCross->GetId()] - (double)currentRoad->GetLength() < (double)currentRoad->GetLength() / 2.0 * (double)currentRoad->GetLanes())
            {
                updateweightCrossToCross += m_presetVipTracePreloadWeight;
            }
            else
            {
                updateweightCrossToCross = 10000;
            }
        }
    }
}


void SchedulerDijkstra::DoHandleGetoutGarage(const int& time, SimScenario& scenario, SimCar* car)
{
    if (m_deadLockSolver.IsGarageLockedInBackup(time))
        return;
    if (scenario.GetOnRoadCarsN() >= 9000)
    {
        car->SetRealTime(time + 1);
        return;
    }
    Cross* startCross = car->GetCar()->GetFromCross();
    if (garagePlanCarNum[startCross->GetId()] == 0 || car->GetCar()->GetMaxSpeed() > garageMinSpeed[startCross->GetId()])
    {
        car->SetRealTime(time + 1);
        return;
    }
    else
    {
        std::vector<int> validFirstHop;
        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
        {
            Road* road = startCross->GetRoad((Cross::DirectionType)i);
            if (road != 0)
            {
                SimRoad* roadLink = scenario.Roads()[road->GetId()];
                if (roadLink->GetRoad()->CanStartFrom(startCross->GetId()))
                {
                    bool canGetOnRoad = false;
                    for (int j = 1; j <= road->GetLanes(); ++j)
                    {
                        auto carInLane = roadLink->GetCarsFrom(j, startCross->GetId());
                        if (carInLane.size() == 0 || scenario.Cars()[carInLane.back()->GetId()]->GetCurrentPosition() != 1)
                            canGetOnRoad = true;
                    }
                    if (canGetOnRoad)
                    {
                        validFirstHop.push_back(road->GetId());
                    }
                }
            }
        }
        if (validFirstHop.size() != 0)
        {
            UpdateCarTraceByDijkstraForDoUpdate(time, scenario, validFirstHop, car);
            ASSERT(car->GetNextRoadId() >= 0);
            /*
            if ((double)car->GetTrace().Size() / (double)minPathCrossToCross[startCross->GetId()][car->GetCar()->GetToCrossId()] > 1.5)
            {
                car->SetRealTime(time + 1);
                return;
            }
            
            else
            */
            //{
            garagePlanCarNum[startCross->GetId()] -= 1;
            auto& carTrace = car->GetTrace();
            Cross* lastCross = startCross;
            for (uint iTrace = car->GetCurrentTraceIndex(); iTrace < carTrace.Size(); ++iTrace)
            {
                Road* currentRoad = Scenario::Roads()[carTrace[iTrace]];
                Cross* nextCross = currentRoad->GetPeerCross(lastCross);
                auto& updateweightCrossToCross = lengthMapInDoUpdate[lastCross->GetId()][nextCross->GetId()];
                ASSERT(nextCross->GetId() == currentRoad->GetPeerCross(lastCross)->GetId());
                ASSERT(updateweightCrossToCross != Inf);
                updateweightCrossToCross += m_presetVipTracePreloadWeight;
                lastCross = nextCross;
            }
            //}
        }
        else
        {
            car->SetRealTime(time + 1);
            return;
        }
    }
}

#include "random.h"
void SchedulerDijkstra::DoHandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result)
{
    if (needPrintCarDijkstra != 0)
    {
        LOG("waitTime =  " << needPrintCarDijkstra->GetLastUpdateTime() - needPrintCarDijkstra->GetLockOnNextRoadTime() << " car in road : " << needPrintCarDijkstra->GetCurrentRoad()->GetId() << " Cross = " << needPrintCarDijkstra->GetCurrentCross()->GetId() << " next road = " << needPrintCarDijkstra->GetNextRoadId());
        Cross* busyCross = needPrintCarDijkstra->GetCurrentCross();
        Simulator::Instance.PrintCrossState(time, scenario, busyCross);
        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
        {
            Road* road = busyCross->GetRoad((Cross::DirectionType)i);
            if (road != 0)
            {
                SimRoad* roadLink = scenario.Roads()[road->GetId()];
                if (roadLink->GetRoad()->CanStartFrom(busyCross->GetId()))
                {
                    Cross* peerlink = roadLink->GetRoad()->GetPeerCross(busyCross);
                    //wsq
                    int carAver = 0;
                    int roadLines = roadLink->GetRoad()->GetLanes();
                    for (int j = 1; j <= roadLines; j++)
                    {
                        carAver += roadLink->GetCarsFrom(j, busyCross->GetId()).size();
                    }
                    LOG("roadId =  " << road->GetId() << " Sum =  " << carAver << " Lanes = " << roadLines);
                }
            }
        }

        needPrintCarDijkstra = 0;
    }

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

std::pair<int, bool> SchedulerDijkstra::SelectBestRoad(SimScenario& scenario, const std::vector<int>& list, SimCar* car)
{
    auto ret = UpdateCarTraceByDijkstraForSelectBestRoad(0, scenario, list, car);
    ASSERT(ret);
    return std::make_pair(1, true);
}

void SchedulerDijkstra::UpdateCarTraceByDijkstraForDoUpdate(const int& time, const SimScenario& scenario, const std::vector<int>& validFirstHop, SimCar* car)
{
    ASSERT(validFirstHop.size() > 0);

    /* Dijkstra weight */
    static uint crossSize = 0;
    static std::vector<int> lengthList;
    static std::vector<bool> visitedList;
    static std::vector<int> pathLastCrossId;
    if (Scenario::Crosses().size() != crossSize)
    {
        crossSize = Scenario::Crosses().size();
        lengthList.resize(crossSize);
        visitedList.resize(crossSize);
        pathLastCrossId.resize(crossSize);
    }
    int from = car->GetCar()->GetFromCrossId();
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        if (from != iCross && lengthMapInDoUpdate[from][iCross] > 0)
        {
            lengthList[iCross] = lengthMapInDoUpdate[from][iCross];
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
            if (visitedList[iUpdate] == false && lengthMapInDoUpdate[visited][iUpdate] > 0 && (min + lengthMapInDoUpdate[visited][iUpdate]) < lengthList[iUpdate])
            {



                lengthList[iUpdate] = min + lengthMapInDoUpdate[visited][iUpdate];
                pathLastCrossId[iUpdate] = visited;
            }
        }
    }

    static std::vector<int> crossListDiji;
    crossListDiji.clear();
    int endCrossId = car->GetCar()->GetToCrossId();
    auto& pathList = minPathCrossToCrossDijkstra[from][endCrossId];
    pathList.clear();
    while (endCrossId != from)
    {
        crossListDiji.push_back(endCrossId);
        endCrossId = pathLastCrossId[endCrossId];
    }
    //crossListDiji.push_front(endCrossId);

    //check path valid
    ASSERT(crossListDiji.size() > 0);
    Cross* lastCross = Scenario::Crosses()[from];
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
                    pathList.push_back(road->GetId());
                    break;
                }
            }
        }
        ASSERT_MSG(consistant, "can not find the road bewteen " << lastCross->GetId() << " and " << thisCross->GetId());
        lastCross = thisCross;
    }
    ASSERT(minPathCrossToCrossDijkstra[from][car->GetCar()->GetToCrossId()].size() > 0);
    auto& carTrace = car->GetTrace();
    carTrace.Clear();
    for (auto traceIte = minPathCrossToCrossDijkstra[from][car->GetCar()->GetToCrossId()].begin(); traceIte != minPathCrossToCrossDijkstra[from][car->GetCar()->GetToCrossId()].end(); traceIte++)
    {
        //ASSERT(carTrace.Size() == 0 || (*(carTrace.Tail() - 1) != *traceIte));
        carTrace.AddToTail(*traceIte);
    }
}

bool SchedulerDijkstra::UpdateCarTraceByDijkstraForSelectBestRoad(const int& time, const SimScenario& scenario, const std::vector<int>& validFirstHop, SimCar* car) const
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
                lengthMap[from][nearCross->GetId()] = ((double)roadLength) * m_lengthWeight / (double)roadLength + (double)carAverager * (double)carAverager * (double)carAverager / (double)roadLength;//m_carNumWeight;
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
                        lengthMap[cross->GetId()][peerlink->GetId()] = ((double)roadLength) / (double)roadLanes * m_lengthWeight + (double)carAver * (double)carAver * (double)carAver / (double)roadLength;//m_carNumWeight;
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
    carTrace.Clear(car->GetCurrentTraceIndex());
    for (auto traceIte = pathListDiji.begin(); traceIte != pathListDiji.end(); traceIte++)
    {
        //ASSERT(carTrace.Size() == 0 || (*(carTrace.Tail() - 1) != *traceIte));
        carTrace.AddToTail(*traceIte);
    }
    return true;
}

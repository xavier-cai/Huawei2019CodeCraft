#include "scheduler-time-weight.h"
#include "scenario.h"
#include "assert.h"
#include "log.h"
#include <algorithm>
#include "tactics.h"
#include <math.h>

//std::vector< std::vector< std::vector<double> > > SchedulerTimeWeight::m_confidence;
std::vector< std::vector< std::vector<double> > > SchedulerTimeWeight::m_collectionWeight;
int SchedulerTimeWeight::m_maxValidRange;

void SchedulerTimeWeight::InitilizeConfidence()
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    m_maxValidRange = Scenario::Roads().size();

    int maxN = m_maxValidRange;
    double p = 0.97;
    double bound = 0.5e-5; //10w cars -> 0.5 weight

    std::vector< std::vector<double> > binomial;
    std::vector< std::vector< std::vector<double> > > confidence;
    binomial.resize(maxN);
    confidence.resize(maxN);
    m_collectionWeight.resize(maxN);
    for (int i = 0; i < maxN; ++i)
    {
        binomial[i].resize(maxN, 1);
        confidence[i].resize(maxN);
        m_collectionWeight[i].resize(maxN);
        for (int j = 0; j < maxN; ++j)
        {
            confidence[i][j].resize(maxN, 0);
            m_collectionWeight[i][j].resize(maxN, 0);
        }
    }

    binomial[0][0] = 1.0;
    for (int i = 1; i < maxN; ++i)
        binomial[i][0] = (1.0 - p) * binomial[i - 1][0];
    for (int j = 1; j < maxN; ++j)
        binomial[0][j] = 0.0;
    for (int i = 1; i < maxN; ++i)
        for (int j = 1; j < maxN; ++j)
            binomial[i][j] = (1.0 - p) * binomial[i - 1][j] + p * binomial[i - 1][j - 1];

    for (int t = 0; t < maxN; ++t)
    {
        for (int b = 0; b < maxN; ++b)
        {
            confidence[t][b][b] = binomial[t][b];
            for (int a = b - 1; a >= 0; --a)
                confidence[t][a][b] = binomial[t][a] + confidence[t][a + 1][b];
        }
    }

    //simplify confidence[deltaT][a][b] to collection weight[a][b][past]
    for (int a = 0; a < maxN; ++a)
    {
        for (int b = a; b < maxN; ++b)
        {
            int tmpb = a;
            int tmpt = a;
            bool decreasing = false;
            double lastW = confidence[tmpt][a][tmpb];
            while (tmpt < maxN)
            {
                double thisW = confidence[tmpt][a][tmpb];
                if (lastW > thisW) decreasing = true;
                if (decreasing && thisW < bound) break;

                //transform to w'
                for (int past = 0; past <= tmpt; ++past)
                    if (binomial[tmpt][tmpt - past] < bound) break;
                    else m_collectionWeight[a][b][tmpt - past] += binomial[tmpt][tmpt - past] * thisW;

                ++tmpt;
                if (tmpb < b) ++tmpb;
            }
        }
    }
}

struct CompareCarForDispatchStruct
{
    bool operator () (SimCar* a, SimCar* b)
    {
        const Car* ac = a->GetCar();
        const Car* bc = b->GetCar();
        if (ac->GetIsVip() != bc->GetIsVip())
            return ac->GetIsVip();
        if (ac->GetMaxSpeed() != bc->GetMaxSpeed())
            return ac->GetMaxSpeed() < bc->GetMaxSpeed();
        if (a->GetRealTime() != b->GetRealTime())
            return a->GetRealTime() < b->GetRealTime();
        return ac->GetId() < bc->GetId();
    }

} CompareCarForDispatch;

SchedulerTimeWeight::SchedulerTimeWeight()
    : m_updateInterval(1), m_carWeightStartTime(-1)
{ }

void SchedulerTimeWeight::InitializeBestTraceByFloyd()
{
    int crossSize = Scenario::Crosses().size();

    std::vector< std::vector<int> > flodyWeight;
    std::vector< std::vector<int> > flodyConnection;
    flodyWeight.resize(crossSize);
    flodyConnection.resize(crossSize);
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        flodyWeight[iCross].resize(crossSize, Inf);
        flodyConnection[iCross].resize(crossSize);
        for (uint jCross = 0; jCross < crossSize; ++jCross)
            flodyConnection[iCross][jCross] = jCross;
    }
    
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        Cross* cross = Scenario::Crosses()[iCross];
        for (int i = (int)Cross::NORTH; i <= (int)Cross::WEST; i++)
        {
            Road* road = cross->GetRoad((Cross::DirectionType)i);
            if (road != 0)
            {
                if (road->CanStartFrom(cross->GetId()))
                {
                    Cross* peer = road->GetPeerCross(cross);
                    flodyWeight[iCross][peer->GetId()] = road->GetLength();
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
                double lengthAfterTran = flodyWeight[iRow][iTransfer] + flodyWeight[iTransfer][iColumn];
                if (flodyWeight[iRow][iColumn] > lengthAfterTran)
                {
                    flodyWeight[iRow][iColumn] = lengthAfterTran;
                    ASSERT(flodyWeight[iRow][iColumn] != Inf);
                    flodyConnection[iRow][iColumn] = iTransfer;
                }
            }
        }
    }

    for (uint iRow = 0; iRow < crossSize; ++iRow)
    {
        for (uint iColumn = 0; iColumn < crossSize; ++iColumn)
        {
            ASSERT(flodyWeight[iRow][iColumn] != Inf);
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
                int transstep = flodyConnection[startStep][iEnd];
                while (flodyConnection[startStep][transstep] != transstep)
                {
                    transstep = flodyConnection[startStep][transstep];
                }
                startStep = transstep;
                ASSERT(flodyWeight[startStep][iEnd] != Inf);
                crossList.push_back(startStep);
            }

            //trans crosses to roads
            auto& pathList = m_bestTrace[iStart][iEnd];
            pathList.second = flodyWeight[iStart][iEnd];
            pathList.first.clear();
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
                            pathList.first.push_back(road->GetId());
                            break;
                        }
                    }
                }
                ASSERT_MSG(consistant, "can not find the road bewteen " << lastCross->GetId() << " and " << thisCross->GetId());
                lastCross = thisCross;
            }
        }
    }
}

void SchedulerTimeWeight::InitializeCarTraceByBeastTrace(SimScenario& scenario)
{
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car->GetCar()->GetIsPreset()) continue;
        auto& best = m_bestTrace[car->GetCar()->GetFromCrossId()][car->GetCar()->GetToCrossId()].first;
        for (auto ite = best.begin(); ite != best.end(); ite++)
            car->GetTrace().AddToTail(*ite);
    }
}

void SchedulerTimeWeight::InitializeCarTraceByDijkstra(SimScenario& scenario)
{
    uint crossCount = Scenario::Crosses().size();
    std::vector< std::vector<double> > dijkWeight;
    std::vector<int> dijkLengthList;
    std::vector<bool> dijkVisitedList;
    std::vector<int> dijkPathLastCrossId;
    dijkWeight.resize(crossCount);
    dijkLengthList.resize(crossCount);
    dijkVisitedList.resize(crossCount);
    dijkPathLastCrossId.resize(crossCount);
    for (uint i = 0; i < crossCount; ++i)
        dijkWeight[i].resize(crossCount, Inf);
    for (uint i = 0; i < Scenario::Roads().size(); ++i)
    {
        const Road* road = Scenario::Roads()[i];
        dijkWeight[road->GetStartCrossId()][road->GetEndCrossId()] = road->GetLength();
        if (road->GetIsTwoWay())
            dijkWeight[road->GetEndCrossId()][road->GetStartCrossId()] = road->GetLength();
    }

    std::vector< std::vector<SimCar*> > cars;
    cars.resize(Scenario::Crosses().size());
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car == 0) continue;
        if ((car->GetIsInGarage() && car->GetCar()->GetIsPreset())
            || (!car->GetIsInGarage() && !car->GetIsReachedGoal()))
        {
            Cross* cross = car->GetCurrentCross();
            for (uint i = car->GetCurrentTraceIndex(); i < car->GetTrace().Size(); ++i)
            {
                const Road* road = Scenario::Roads()[car->GetTrace()[i]];
                Cross* peer = road->GetPeerCross(cross);
                dijkWeight[cross->GetId()][peer->GetId()] += car->GetCar()->GetIsPreset() ? 2.0 : 1.0;
                cross = peer;
            }
        }
        if (car->GetIsInGarage() && !car->GetCar()->GetIsPreset())
        {
            cars[car->GetCar()->GetFromCrossId()].push_back(car);
        }
    }

    for (uint i = 0; i < cars.size(); ++i)
        std::sort(cars[i].begin(), cars[i].end(), CompareCarForDispatch);

    int notEndCount = Scenario::Crosses().size();
    for (uint i = 0; i < Scenario::Crosses().size(); ++i)
        if (cars[i].size() == 0)
            --notEndCount;
    std::vector<int> indexInGarage;
    indexInGarage.resize(Scenario::Crosses().size(), 0);
    while (notEndCount > 0)
    for (uint i = 0; i < Scenario::Crosses().size(); ++i)
    {
        if (cars[i].size() == indexInGarage[i]) continue;
        SimCar* car = cars[i][indexInGarage[i]];

        int from = car->GetCar()->GetFromCross()->GetId();
        int to = car->GetCar()->GetToCross()->GetId();
        //calculate
        for (uint iCross = 0; iCross < crossCount; ++iCross)
        {
            if (from != iCross)
            {
                dijkLengthList[iCross] = dijkWeight[from][iCross];
                dijkPathLastCrossId[iCross] = from;
            }
            else
            {
                dijkLengthList[iCross] = Inf;
                dijkPathLastCrossId[iCross] = -1;
            }
            dijkVisitedList[iCross] = false;
        }
        dijkLengthList[from] = 0;
        dijkPathLastCrossId[from] = from;
        dijkVisitedList[from] = true;
        for (uint iExtend = 0; iExtend < crossCount - 1; ++iExtend)
        {
            int min = Inf;
            int visited = -1;
            for (uint iMin = 0; iMin < crossCount; ++iMin)
            {
                if (!dijkVisitedList[iMin] && dijkLengthList[iMin] < min)
                {
                    min = dijkLengthList[iMin];
                    visited = iMin;
                }
            }

            ASSERT(visited != -1);
            dijkVisitedList[visited] = true;
            for (uint iUpdate = 0; iUpdate < crossCount; ++iUpdate)
            {
                if (!dijkVisitedList[iUpdate])
                {
                    double thisLenght = min + dijkWeight[visited][iUpdate];
                    if (thisLenght< dijkLengthList[iUpdate])
                    {
                        dijkLengthList[iUpdate] = thisLenght;
                        dijkPathLastCrossId[iUpdate] = visited;
                    }
                }
            }
        }

        std::vector<int> crossListDiji;

        int endCrossId = to;
        while (endCrossId != from)
        {
            crossListDiji.push_back(endCrossId);
            endCrossId = dijkPathLastCrossId[endCrossId];
        }
        //crossListDiji.push_front(endCrossId);

        //check path valid
        car->GetTrace().Clear();
        Cross* lastCross = car->GetCar()->GetFromCross();
        if (!car->GetIsInGarage())
            lastCross = car->GetCurrentCross();
        for (int crossIndex = crossListDiji.size() - 1; crossIndex >= 0; --crossIndex)
        {
            bool consistant = false;
            Cross* thisCross = Scenario::Crosses()[crossListDiji[crossIndex]];
            for (int dirIndex = (int)Cross::NORTH; dirIndex <= (int)Cross::WEST; dirIndex++)
            {
                Road* road = lastCross->GetRoad((Cross::DirectionType)dirIndex);
                if (road != 0)
                {
                    if ((road->GetStartCrossId() == lastCross->GetId() && road->GetEndCrossId() == thisCross->GetId())
                        || (road->GetEndCrossId() == lastCross->GetId() && road->GetStartCrossId() == thisCross->GetId() && road->GetIsTwoWay()))
                    {
                        consistant = true;
                        car->GetTrace().AddToTail(road->GetId());
                        dijkWeight[lastCross->GetId()][thisCross->GetId()] += 1.2 / sqrt(road->GetLanes());
                        break;
                    }
                }
            }
            ASSERT_MSG(consistant, "can not find the road bewteen " << lastCross->GetId() << " and " << thisCross->GetId());
            lastCross = thisCross;
        }

        if (cars[i].size() == ++indexInGarage[i]) --notEndCount; //no cars here
    }
}

void SchedulerTimeWeight::DoInitialize(SimScenario& scenario)
{
    InitilizeConfidence();

    m_deadLockSolver.Initialize(0, scenario);
    m_deadLockSolver.SetSelectedRoadCallback(Callback::Create(&SchedulerTimeWeight::SelectBestRoad, this));

    int roadCount = Scenario::Roads().size();
    int crossCount = Scenario::Crosses().size();
    m_carWeight.resize(roadCount);
    m_roadWeight.resize(roadCount);
    m_roadCapacity.resize(roadCount);
    m_expectedWeight.resize(Scenario::Cars().size());
    m_bestTrace.resize(crossCount);
    m_carList.resize(crossCount);
    int maxLanes = 0;
    for (uint i = 0; i < roadCount; i++)
    {
        m_carWeight[i].resize(roadCount, std::make_pair(Inf, Inf));
        m_roadWeight[i] = Scenario::Roads()[i]->GetLength();
        m_roadCapacity[i] = Scenario::Roads()[i]->GetLanes() * Scenario::Roads()[i]->GetLength();
        m_expectedWeight[i].resize(roadCount, -1);
        maxLanes = std::max(Scenario::Roads()[i]->GetLength(), maxLanes);
    }
    m_threshold.resize(maxLanes + 1, std::make_pair(0.0, 0.0));
    for (int i = 1; i < maxLanes; ++i)
    {
        m_threshold[i].first = 0.2 / sqrt(i);
        m_threshold[i].second = 0.8 / sqrt(i);
    }

    for (uint i = 0; i < crossCount; ++i)
        m_bestTrace[i].resize(crossCount);

    //InitializeBestTraceByFloyd();
    //InitializeCarTraceByBeastTrace(scenario);
    //InitializeCarTraceByDijkstra(scenario);
    return ;
    

    struct Statistic
    {
        int Road;
        bool Dir;
        int Count;
    };

    struct CompareStatisticStruct
    {
        bool operator () (Statistic& a, Statistic& b)
        {
            return a.Count > b.Count;
        }
    } CompareStatistic;

    std::vector<Statistic> statistic;
    statistic.resize(roadCount * 2);
    for (int i = 0; i < roadCount * 2; ++i)
    {
        statistic[i].Road = i % roadCount;
        statistic[i].Dir = i < roadCount;
        statistic[i].Count = 0;
    }

    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        Cross* cross = car->GetCar()->GetFromCross();
        for (auto ite = car->GetTrace().Head(); ite != car->GetTrace().Tail(); ite++)
        {
            Road* road = Scenario::Roads()[*ite];
            ++(statistic[(*ite) + (road->IsFromOrTo(cross->GetId()) ? 0 : roadCount)].Count);
            cross = road->GetPeerCross(cross);
        }
    }

    std::sort(statistic.begin(), statistic.end(), CompareStatistic);
    for (uint i = 0; i < statistic.size(); ++i)
        LOG (Scenario::Roads()[statistic[i].Road]->GetOriginId() << " " << statistic[i].Dir << " " << statistic[i].Count);
}

std::pair<int, bool> SchedulerTimeWeight::SelectBestRoad(SimScenario& scenario, const std::vector<int>& list, SimCar* car)
{
    std::vector<int> baned;
    Cross* cross = car->GetCurrentCross();
    DirectionType_Foreach(dir,
        Road* road = cross->GetRoad(dir);
        if (road != 0 && road->CanStartFrom(cross->GetId()))
        {
            bool find = false;
            for (auto ite = list.begin(); ite != list.end(); ++ite)
                if (*ite == road->GetId())
                    find = true;
            if (!find)
                baned.push_back(road->GetId());
        }
    );
    auto ret = UpdateCarTraceByDijkstraWithTimeWeight(m_deadLockSolver.GetDeadLockTime(), scenario, car, baned);
    ASSERT(ret);
    return std::make_pair(1, true);
}

void SchedulerTimeWeight::DoUpdate(int& time, SimScenario& scenario)
{
    if (!m_deadLockSolver.NeedUpdate(time))
        return;

    static double updateTime = 1;
    //if (time == 0 || --updateTime == 0)
    if (time % 50 == 0)
    {
        InitializeCarTraceByDijkstra(scenario);
        updateTime = (double)(scenario.GetCarInGarageN() + scenario.GetOnRoadCarsN()) / (double)scenario.Cars().size() * 100.0;
    }
    

    if (time % m_updateInterval == 0)
    {
        UpdateTimeWeight(time, scenario);
    }

    if (m_deadLockSolver.IsGarageLockedInBackup(time))
        return;

    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (!car->GetCar()->GetIsPreset() && car->GetIsInGarage() && car->GetRealTime() <= time)
            m_carList[car->GetCar()->GetFromCrossId()].push_back(car);
    }

    for (uint i = 0; i < m_carList.size(); ++i)
        std::sort(m_carList[i].begin(), m_carList[i].end(), CompareCarForDispatch);

    int notEndCount = m_carList.size();
    std::vector<bool> endFlag;
    std::vector<int> dispatchIndex;
    std::vector<int> tryCounter;
    endFlag.resize(notEndCount, false);
    dispatchIndex.resize(notEndCount, 0);
    tryCounter.resize(notEndCount, 0);
    while (notEndCount > 0)
    {
        for (uint i = 0; i < m_carList.size(); ++i) //each garage
        {
            if (endFlag[i]) continue;
            if (dispatchIndex[i] == m_carList[i].size()) //no more car
            {
                m_carList[i].clear();
                --notEndCount;
                endFlag[i] = true;
                break;
            }
            while (dispatchIndex[i] < m_carList[i].size()) //untill find a car which can be dispatch
            {
                SimCar* car = m_carList[i][dispatchIndex[i]];
                if (IsAppropriateToDispatch(time, car))
                {
                    UpdateTimeWeightForEachCar(time, car);
                    tryCounter[i] = 0;
                    ++dispatchIndex[i]; //...
                    break;
                }
                else
                {
                    car->SetRealTime(time + 1);
                    if (false && ++tryCounter[i] > 4) //give up in this garage
                    {
                        //give up left cars
                        for (int iCar = dispatchIndex[i] + 1; iCar < m_carList[i].size(); ++iCar)
                            m_carList[i][iCar]->SetRealTime(time + 1);
                        dispatchIndex[i] = m_carList[i].size() - 1; //end in next loop
                        ++dispatchIndex[i]; //...
                        break;
                    }
                }
                ++dispatchIndex[i];
            }
        }
    }
}

void SchedulerTimeWeight::DoHandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result)
{
    if (result.Conflict)
    {
        if (m_deadLockSolver.HandleDeadLock(time, scenario))
        {
            result.Conflict = false; //retry
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

void SchedulerTimeWeight::DoHandleBecomeFirstPriority(const int& time, SimScenario& scenario, SimCar* car)
{
    if (m_deadLockSolver.IsCarTraceLockedInBackup(car))
        return;
    ASSERT(!car->GetIsLockOnNextRoad());
    SimRoad* nextRoad = scenario.Roads()[car->GetNextRoadId()];
    if (nextRoad->GetCarN() > m_roadCapacity[car->GetNextRoadId()] * 0.7)
    {
        //need reset the trace
        UpdateTimeWeightForEachCar(time, car, true); //decrease
        std::vector<int> baned;
        baned.push_back(car->GetCurrentRoad()->GetId());
        UpdateCarTraceByDijkstraWithTimeWeight(time, scenario, car, baned);
        UpdateTimeWeightForEachCar(time, car, false); //increase
    }
}

bool SchedulerTimeWeight::IsAppropriateToDispatch(const int& time, SimCar* car) const
{
    ASSERT(car->GetIsInGarage());
    static const double thresold = 0.6;
    int lengthCount = 0;
    double weightCount = 0;
    int pos = 0;
    Cross* cross = car->GetCar()->GetFromCross();
    for (uint iTrace = 0; iTrace < car->GetTrace().Size(); ++iTrace)
    {
        Road* road = Scenario::Roads()[car->GetTrace()[iTrace]];
        Road* nextRoad = (iTrace == car->GetTrace().Size() - 1) ? 0 : Scenario::Roads()[car->GetTrace()[iTrace + 1]];
        auto next = car->CalculateLeaveTime(road, nextRoad, pos);
        pos = next.second;
        lengthCount += road->GetLength();
        int index = next.first + time - m_carWeightStartTime;
        if (index < m_maxValidRange)
        {
            double weight = road->IsFromOrTo(cross->GetId()) ? m_carWeight[index][road->GetId()].first : m_carWeight[index][road->GetId()].second;
            double factor = weight / m_roadCapacity[road->GetId()];
            //if (factor > m_roadCapacity[road->GetId()] * 0.7)
            //    return false;
            //if (factor > m_threshold[road->GetLanes()].second) //too crowed
            //    return false;
            weightCount += weight * (factor > m_threshold[road->GetLanes()].first ? 1.0 : 0.5);
            if (weightCount / lengthCount > thresold) //too crowed
                return false;
        }
        cross = road->GetPeerCross(cross);
    }
    return true;
}

void SchedulerTimeWeight::UpdateTimeWeight(const int& time, SimScenario& scenario)
{
    m_carWeightStartTime = time;
    for (uint iTime = 0; iTime < m_maxValidRange; ++iTime)
    {
        for (uint iRoad = 0; iRoad < Scenario::Roads().size(); ++iRoad)
        {
            m_carWeight[iTime][iRoad].first = 0;
            m_carWeight[iTime][iRoad].second = 0;
        }
    }
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (!car->GetIsReachedGoal() && ((car->GetCar()->GetIsPreset() && !car->GetIsForceOutput()) || !car->GetIsInGarage()))
            UpdateTimeWeightForEachCar(time, car);
    }
}

void SchedulerTimeWeight::UpdateTimeWeightByRoadAndTime(const int& time, const int& roadId, const bool& dir, int startTime, int leaveTime, const bool& isDecrease)
{
    ASSERT(leaveTime >= startTime && startTime >= time);
    if (startTime - m_carWeightStartTime >= m_maxValidRange)
        return;

    int deltaStart = startTime - time;
    int deltaEnd = std::min(leaveTime - time, m_maxValidRange - 1);
    int indexDelta = time - m_carWeightStartTime;

    const auto& collectionWeight = m_collectionWeight[deltaStart][deltaEnd];
    for (int t = deltaStart - 1; t >= 0; --t) //before
    {
        const double& wba = collectionWeight[t];
        if (wba <= 0) break;
        (dir ? m_carWeight[t + indexDelta][roadId].first : m_carWeight[t + indexDelta][roadId].second) += isDecrease ? -wba : wba;
    }
    for (int t = deltaStart; t <= deltaEnd; ++t)
    {
        (dir ? m_carWeight[t + indexDelta][roadId].first : m_carWeight[t + indexDelta][roadId].second) += isDecrease ? -collectionWeight[t] : collectionWeight[t];
    }
    for (int t = deltaEnd + 1; t + indexDelta < m_maxValidRange; ++t) //after
    {
        const double& wba = collectionWeight[t];
        if (wba <= 0) break;
        (dir ? m_carWeight[t + indexDelta][roadId].first : m_carWeight[t + indexDelta][roadId].second) += isDecrease ? -wba : wba;
    }
}

void SchedulerTimeWeight::UpdateTimeWeightForEachCar(const int& originTime, SimCar* car, const bool& isDecrease)
{
    if (car->GetIsReachedGoal() || car->GetRealTime() - m_carWeightStartTime >= m_maxValidRange)
        return;

    int time = isDecrease ? m_carWeightStartTime : originTime;

    auto& carTrace = car->GetTrace();
    
    int pos = 0;
    int eventTime = std::max(time, car->GetRealTime());
    Cross* cross = car->GetCurrentCross();

    uint iTrace = car->GetCurrentTraceIndex();
    if (!car->GetIsInGarage())
    {
        eventTime = car->GetSimState(originTime) == SimCar::SCHEDULED ? time + 1 : time;
        pos = car->GetCurrentPosition();
        --iTrace;
        cross = car->GetCurrentRoad()->GetPeerCross(cross);
    }

    int lastEventTime = eventTime;
    for ( ; iTrace < carTrace.Size(); ++iTrace)
    {
        Road* road = Scenario::Roads()[carTrace[iTrace]];
        Road* nextRoad = (iTrace == carTrace.Size() - 1) ? 0 : Scenario::Roads()[carTrace[iTrace + 1]];

        auto next = car->CalculateLeaveTime(road, nextRoad, pos);
        eventTime += next.first;
        pos = next.second;

        UpdateTimeWeightByRoadAndTime(time, road->GetId(), road->IsFromOrTo(cross->GetId()), lastEventTime, eventTime - 1, isDecrease);
        lastEventTime = eventTime;
        cross = road->GetPeerCross(cross);
    }
}

void SchedulerTimeWeight::UpdateCurrentWeightByScenario(const int& time, SimScenario& scenario)
{
    LOG("gosh, the predict must be send to hell! @" << time);
    ASSERT(time >= m_carWeightStartTime);
    int weightTimeIndex = (time - m_carWeightStartTime) % m_maxValidRange;
    for(uint iRoad = 0; iRoad < scenario.Roads().size(); ++iRoad)
    {
        const SimRoad* road = scenario.Roads()[iRoad];
        auto& weightPair = m_carWeight[weightTimeIndex][road->GetRoad()->GetId()];
        for (int dir = 0; (dir == 0) || (dir == 1 && road->GetRoad()->GetIsTwoWay()); ++dir)
        {
            auto& weight = dir == 0 ? weightPair.first : weightPair.second;
            weight = 0;
            for (uint iLane = 0; iLane < road->GetRoad()->GetLanes(); ++iLane)
                weight += road->GetCars(iLane, dir == 1).size();
        }
    }
}

inline double WbaToLengthWeight(double wba, const int& capacity)
{
    double factor = wba / capacity;
    if (factor < 0.2) wba = 0;
    else if (factor > 0.7) wba *= 4;
    return wba;
}

bool SchedulerTimeWeight::UpdateCarTraceByDijkstraWithTimeWeight(const int& time, SimScenario& scenario, SimCar* car, const std::vector<int>& banedFirstHop) const
{
    ASSERT(!car->GetIsReachedGoal());
    ASSERT(banedFirstHop.size() < 4);

    int firstHopTime = time;
    if (car->GetIsInGarage())
        firstHopTime = std::max(time, car->GetRealTime());
    else if (car->GetSimState(time) == SimCar::SCHEDULED)
        ++firstHopTime;
    ASSERT(firstHopTime >= m_carWeightStartTime);
    firstHopTime -= m_carWeightStartTime;

    struct DijkHop
    {
        DijkHop()
            : Road(0), Time(-1), Position(-1), Weight(Inf)
        { }
        Road* Road;
        int Time;
        int Position;
        int Weight;
    };

    struct DijWeight
    {
        DijWeight()
            : Length(Inf), Road(-1)
        { }
        int Length;
        int Road;
        bool Dir;
    };

    /* Dijkstra weight */
    static uint crossSize = 0;
    static std::vector< std::vector<DijWeight> > dijkWeight;
    static std::vector<DijkHop> dijkHopList;
    static std::vector<bool> dijkVisitedList;
    static std::vector<int> dijkPathLastCrossId;

    if (Scenario::Crosses().size() != crossSize)
    {
        crossSize = Scenario::Crosses().size();
        dijkWeight.resize(crossSize);
        dijkHopList.resize(crossSize);
        dijkVisitedList.resize(crossSize);
        dijkPathLastCrossId.resize(crossSize);
        for (uint iCross = 0; iCross < crossSize; ++iCross)
        {
            dijkWeight[iCross].resize(crossSize);
            for (uint jCross = 0; jCross < crossSize; ++ jCross)
            {
                dijkWeight[iCross][jCross].Length = Inf;
                dijkWeight[iCross][jCross].Road = -1;
            }
        }

        for (uint i = 0; i < Scenario::Roads().size(); ++i)
        {
            Road* road = Scenario::Roads()[i];
            dijkWeight[road->GetStartCrossId()][road->GetEndCrossId()].Length = road->GetLength();
            dijkWeight[road->GetStartCrossId()][road->GetEndCrossId()].Road = i;
            dijkWeight[road->GetStartCrossId()][road->GetEndCrossId()].Dir = true;
            if (road->GetIsTwoWay())
            {
                dijkWeight[road->GetEndCrossId()][road->GetStartCrossId()].Length = road->GetLength();
                dijkWeight[road->GetEndCrossId()][road->GetStartCrossId()].Road = i;
                dijkWeight[road->GetEndCrossId()][road->GetStartCrossId()].Dir = false;
            }
        }
    }

    struct WeightLengthBackup
    {
        int FromCross;
        int EndCross;
        int BackupLength;
    };

    std::vector<WeightLengthBackup> backupLength;
    //if (!car->GetIsInGarage())
    {
        backupLength.reserve(banedFirstHop.size());
        Cross* currentCross = car->GetCurrentCross();
        for (uint i = 0; i < banedFirstHop.size(); ++i)
        {
            Road* banedRoad = Scenario::Roads()[banedFirstHop[i]];
            if (banedRoad->CanStartFrom(currentCross->GetId()))
            {
                Cross* peer = banedRoad->GetPeerCross(currentCross);
                WeightLengthBackup backup;
                backup.FromCross = currentCross->GetId();
                backup.EndCross = peer->GetId();
                backup.BackupLength = dijkWeight[backup.FromCross][backup.EndCross].Length;
                backupLength.push_back(backup);
                dijkWeight[backup.FromCross][backup.EndCross].Length = Inf;
            }
        }
    }

    Cross* fromCross = car->GetCar()->GetFromCross();
    if (!car->GetIsInGarage())
        fromCross = car->GetCurrentCross();
    int from = fromCross->GetId();
    int to = car->GetCar()->GetToCrossId();

    //calculate
    for (uint iCross = 0; iCross < crossSize; ++iCross)
    {
        dijkHopList[iCross].Weight = Inf;
        dijkHopList[iCross].Time = Inf;
        dijkPathLastCrossId[iCross] = -1;
        dijkVisitedList[iCross] = false;
    }
    DirectionType_Foreach(dir,
        Road* road = fromCross->GetRoad(dir);
        if (road == 0) continue;
        if (!road->CanStartFrom(fromCross->GetId())) continue;
        Cross* peer = road->GetPeerCross(fromCross);
        if (!Is_Inf(dijkWeight[fromCross->GetId()][peer->GetId()].Length))
        {
            if (car->GetIsInGarage())
            {
                dijkHopList[peer->GetId()].Time = firstHopTime;
                double wba = WbaToLengthWeight(
                    road->IsFromOrTo(fromCross->GetId()) ? m_carWeight[firstHopTime][road->GetId()].first : m_carWeight[firstHopTime][road->GetId()].second
                    , m_roadCapacity[road->GetId()]);
                dijkHopList[peer->GetId()].Weight = dijkWeight[fromCross->GetId()][peer->GetId()].Length + wba;
                dijkHopList[peer->GetId()].Position = std::min(car->GetCar()->GetMaxSpeed(), road->GetLimit());
            }
            else
            {
                auto next = car->CalculateLeaveTime(car->GetCurrentRoad(), road, car->GetCurrentPosition());
                int hopTime = firstHopTime + next.first - 1;
                dijkHopList[peer->GetId()].Time = hopTime;
                dijkHopList[peer->GetId()].Position = next.second;
                double wba = 0;
                if (hopTime < m_maxValidRange)
                    wba = WbaToLengthWeight(
                        road->IsFromOrTo(fromCross->GetId()) ? m_carWeight[hopTime][road->GetId()].first : m_carWeight[hopTime][road->GetId()].second
                        , m_roadCapacity[road->GetId()]);
                dijkHopList[peer->GetId()].Weight = dijkWeight[fromCross->GetId()][peer->GetId()].Length + wba;
            }
            dijkHopList[peer->GetId()].Road = road;
            dijkPathLastCrossId[peer->GetId()] = from;
        }
    );
    dijkPathLastCrossId[from] = from;
    dijkHopList[from].Time = firstHopTime - 1;
    dijkHopList[from].Position = car->GetIsInGarage() ? 0 : car->GetCurrentPosition();
    dijkHopList[from].Weight = 0;
    dijkVisitedList[from] = true;

    for (uint iExtend = 0; iExtend < crossSize - 1; ++iExtend)
    {
        int min = Inf;
        int visited = -1;
        for (uint iMin = 0; iMin < crossSize; ++iMin)
        {
            if (!dijkVisitedList[iMin] && dijkHopList[iMin].Weight < min)
            {
                min = dijkHopList[iMin].Weight;
                visited = iMin;
            }
        }

        ASSERT(visited != -1);
        dijkVisitedList[visited] = true;
        int traceIndex = visited;
        int traceSize = 0;
        while (traceIndex != from)
        {
            traceSize++;
            ASSERT(traceIndex >= 0 && traceIndex < dijkPathLastCrossId.size());
            traceIndex = dijkPathLastCrossId[traceIndex];
        }
        Cross* visitedCross = Scenario::Crosses()[visited];
        const auto& hop = dijkHopList[visited];
        DirectionType_Foreach(dir, 
            Road* road = visitedCross->GetRoad(dir);
            if (road == 0) continue;
            if (!road->CanStartFrom(visitedCross->GetId())) continue;
            Cross* peerUpdate = road->GetPeerCross(visitedCross);
            const auto& weight = dijkWeight[visited][peerUpdate->GetId()];
            auto next = car->CalculateLeaveTime(hop.Road, road, hop.Position);
            int reachTime = hop.Time + next.first;
            double wba = 0;
            if (reachTime < m_maxValidRange)
                wba = WbaToLengthWeight(
                    road->IsFromOrTo(visitedCross->GetId()) ? m_carWeight[reachTime][road->GetId()].first : m_carWeight[reachTime][road->GetId()].second
                    , m_roadCapacity[road->GetId()]);
            double sumWeight = min + dijkWeight[visited][peerUpdate->GetId()].Length + wba;
            if (sumWeight < dijkHopList[peerUpdate->GetId()].Weight)
            {
                dijkHopList[peerUpdate->GetId()].Weight = sumWeight;
                dijkHopList[peerUpdate->GetId()].Position = next.second;
                dijkHopList[peerUpdate->GetId()].Road = road;
                dijkHopList[peerUpdate->GetId()].Time = reachTime;
                dijkPathLastCrossId[peerUpdate->GetId()] = visited;
            }
        );
    }

    car->GetTrace().Clear(car->GetCurrentTraceIndex());
    std::vector<int> newTrace;
    int endCrossId = to;
    while (endCrossId != from)
    {
        ASSERT(dijkHopList[endCrossId].Road != 0);
        ASSERT(dijkHopList[endCrossId].Road->CanReachTo(endCrossId));
        newTrace.push_back(dijkHopList[endCrossId].Road->GetId());
        endCrossId = dijkPathLastCrossId[endCrossId];
    }
    for (int i = newTrace.size() - 1; i >= 0; --i)
        car->GetTrace().AddToTail(newTrace[i]);

    //recorver
    for (uint i = 0; i < backupLength.size(); ++i)
    {
        const auto& backup = backupLength[i];
        dijkWeight[backup.FromCross][backup.EndCross].Length = backup.BackupLength;
    }

#ifdef ASSERT_ON
    //check path valid
    ASSERT(car->GetTrace().Size() > 0);
    Cross* frontCross = car->GetCar()->GetFromCross();
    int frontRoad = -1;
    for (auto ite = car->GetTrace().Head(); ite != car->GetTrace().Tail(); ite++)
    {
        Road* road = Scenario::Roads()[*ite];
        ASSERT(road != 0);
        ASSERT(road->GetId() >= 0);
        ASSERT(road->GetId() != frontRoad);
        ASSERT(road->CanStartFrom(frontCross->GetId()));
        frontCross = road->GetPeerCross(frontCross);
    }
    ASSERT(frontCross->GetId() == car->GetCar()->GetToCrossId());
#endif

    return true;
}

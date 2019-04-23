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

    m_maxValidRange = 200;

    int maxN = m_maxValidRange;
    double p = 0.99;
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

SchedulerTimeWeight::SchedulerTimeWeight()
    : m_updateInterval(1), m_carWeightStartTime(-1)
{ }

void SchedulerTimeWeight::InitilizeBestTraceByFloyd()
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

void SchedulerTimeWeight::DoInitialize(SimScenario& scenario)
{
    InitilizeConfidence();

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
        m_threshold[i].first = 0.1 / sqrt(i);
        m_threshold[i].second = 0.3 / sqrt(i);
    }

    for (uint i = 0; i < crossCount; ++i)
        m_bestTrace[i].resize(crossCount);

    InitilizeBestTraceByFloyd();

    //set trace for cars
    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car->GetCar()->GetIsPreset()) continue;
        auto& best = m_bestTrace[car->GetCar()->GetFromCrossId()][car->GetCar()->GetToCrossId()].first;
        for (auto ite = best.begin(); ite != best.end(); ite++)
            car->GetTrace().AddToTail(*ite);
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

void SchedulerTimeWeight::DoUpdate(int& time, SimScenario& scenario)
{
    if (time % m_updateInterval == 0)
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
    }

    for (uint i = 0; i < scenario.Cars().size(); ++i)
    {
        SimCar* car = scenario.Cars()[i];
        if (car->GetCar()->GetIsPreset() || !car->GetIsInGarage())
            UpdateTimeWeightForEachCar(time, car);
        else if (car->GetRealTime() <= time)
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
            SimCar* car = m_carList[i][dispatchIndex[i]];
            if (IsAppropriateToDispatch(time, car))
            {
                UpdateTimeWeightForEachCar(time, car);
                tryCounter[i] = 0;
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
                }
            }
            ++dispatchIndex[i];
        }
    }
}

bool SchedulerTimeWeight::IsAppropriateToDispatch(const int& time, SimCar* car) const
{
    ASSERT(car->GetIsInGarage());
    static const double thresold = 0.2;
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
            if (factor > m_threshold[road->GetLanes()].second) //too crowed
                return false;
            weightCount += weight * (factor > m_threshold[road->GetLanes()].first ? 1.0 : 0.5);
            if (weightCount / lengthCount > thresold) //too crowed
                return false;
        }
        cross = road->GetPeerCross(cross);
    }
    return true;
}

void SchedulerTimeWeight::UpdateTimeWeightByRoadAndTime(const int& time, const int& roadId, const bool& dir, int startTime, int leaveTime)
{
    ASSERT(leaveTime >= startTime && startTime >= time);
    if (startTime - m_carWeightStartTime >= m_maxValidRange)
        return;

    int deltaStart = startTime - time;
    int deltaEnd = std::min(leaveTime - time, m_maxValidRange - 1);
    int indexDelta = time - m_carWeightStartTime;

    const auto& collectionWeight = m_collectionWeight[deltaStart][deltaEnd];
    for (int t = deltaEnd; t >= 0; --t)
    {
        const double& wba = collectionWeight[t];
        if (wba <= 0) break;
        (dir ? m_carWeight[t + indexDelta][roadId].first : m_carWeight[t + indexDelta][roadId].second) += wba;
    }
}

void SchedulerTimeWeight::UpdateTimeWeightForEachCar(const int& time, SimCar* car)
{
    if (car->GetIsReachedGoal() || car->GetRealTime() - m_carWeightStartTime >= m_maxValidRange)
        return;

    auto& carTrace = car->GetTrace();
    
    int pos = 0;
    int eventTime = std::max(time, car->GetRealTime());
    Cross* cross = car->GetCurrentCross();

    uint iTrace = car->GetCurrentTraceIndex();
    if (!car->GetIsInGarage())
    {
        eventTime = car->GetSimState(time) == SimCar::SCHEDULED ? time + 1 : time;
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

        UpdateTimeWeightByRoadAndTime(time, road->GetId(), road->IsFromOrTo(cross->GetId()), lastEventTime, eventTime - 1);
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

bool SchedulerTimeWeight::UpdateCarTraceByDijkstraWithTimeWeight(const int& time, SimScenario& scenario, const std::vector<int>& validFirstHop, SimCar* car) const
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
                ASSERT(road->CanStartFrom(from));
                lengthMap[from][road->GetPeerCross(cross)->GetId()] = road->GetLength();
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
                    if (road->CanStartFrom(cross->GetId()))
                    {
                        lengthMap[cross->GetId()][road->GetPeerCross(cross)->GetId()] = road->GetLength();
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
            double roadFromOrTo = m_carWeight[roadForUpdate->GetId()][0].second;
            if (roadForUpdate->IsFromOrTo(from))
                roadFromOrTo = m_carWeight[roadForUpdate->GetId()][0].first;
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
            traceSize++;
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
                double roadFromOrTo = m_carWeight[roadForUpdate->GetId()][traceSize + 1].second;
                if (roadForUpdate->IsFromOrTo(visited))
                    roadFromOrTo = m_carWeight[roadForUpdate->GetId()][traceSize + 1].first;
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

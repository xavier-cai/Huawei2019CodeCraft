#include "map-generator.h"
#include "random.h"
#include "assert.h"
#include <fstream>
#include "log.h"
#include "config.h"
#include "sim-car.h"
#include "scheduler-floyd.h"

MapGenerator::MapGenerator()
    : m_width(13), m_height(13), m_carsN(90000), m_singleWayProb(0.03), m_noWayProb(0.01), m_vipProb(0.1), m_presetProb(0.15), m_presetMaxRealTime(800)
{ }

void MapGenerator::SetWidth(int width)
{
    ASSERT(width > 0);
    m_width = width;
}

void MapGenerator::SetHeight(int height)
{
    ASSERT(height > 0);
    m_height = height;
}

void MapGenerator::SetCarsN(int n)
{
    ASSERT(n > 0);
    m_carsN = n;
}

void MapGenerator::SetSingleWayProbability(double prob)
{
    ASSERT(prob >= 0 && prob <= 1);
    m_singleWayProb = prob;
}

void MapGenerator::SetNoWayProbability(double prob)
{
    ASSERT(prob >= 0 && prob <= 1);
    m_noWayProb = prob;
}

void MapGenerator::SetVipProbability(double prob)
{
    ASSERT(prob >= 0 && prob <= 1);
    m_vipProb = prob;
}

void MapGenerator::SetPresetProbability(double prob)
{
    ASSERT(prob >= 0 && prob <= 1);
    m_presetProb = prob;
}

void MapGenerator::SetPresetMaxRealTime(int time)
{
    ASSERT(time > 0);
    m_presetMaxRealTime = time;
}

int GetLength()
{
    double miu = 30;
    double sigma = 12;
    auto rng = Random::Normal(miu, sigma);
    int category = 0;
    if (rng < 20)
        category = 20;
    else if (rng < 26.5)
        category = 24;
    else if (rng < 33.5)
        category = 30;
    else if (rng < 40)
        category = 36;
    else
        category = 40;
    ASSERT(category != 0);
    return category;
}

int GetSpeed()
{
    return (int)Random::Uniform(0, 7) * 2 + 4;
}

int GetPlanTime()
{
    return Random::Uniform(0, 48) + 1;
}

int GetLimit()
{
    const static int selections[] = {8, 10, 12, 15};
    return selections[(int)Random::Uniform(0, 4)];
}

int GetLanes()
{
    return Random::Uniform(0, 4) + 1;
}

void MapGenerator::GenerateMap()
{
    int width = m_width;
    int height = m_height;
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int id =  i * width + j + 1;
            int geneHorizon = j < width - 1 ? 2 : 0;
            int geneVertical = i < height - 1 ? 2 : 0;
            int idHorizon = (i * width + j) * 2 + 5001;
            int idVertical = (i * width + j) * 2 + 5002;
            {
                if ((i != 0 || j != 0) && geneHorizon > 0 && geneVertical > 0)
                {
                    double rng = Random::Uniform();
                    double prob = m_noWayProb + m_singleWayProb;
                    ASSERT(prob < 0.5);
                    if (rng < prob * 2)
                    {
                        if (rng < m_noWayProb)
                            geneVertical = 0;
                        else if (rng < prob)
                            geneVertical = 1;
                        else if (rng < prob + m_noWayProb)
                            geneHorizon = 0;
                        else 
                            geneHorizon = 1;
                    }
                }
                if (geneHorizon > 0)
                {
                    int tmpSstart = id;
                    int tmpEnd = id + 1;
                    if (geneHorizon == 1 && Random::Uniform() < 0.5)
                    {
                        tmpSstart = id + 1;
                        tmpEnd = id;
                    }
                    m_roads.insert(std::make_pair(idHorizon, Road(idHorizon, GetLength(), GetLimit(), GetLanes(), tmpSstart, tmpEnd, geneHorizon == 2)));
                }
                if (geneVertical > 0)
                {
                    int tmpSstart = id;
                    int tmpEnd = id + width;
                    if (geneVertical == 1 && Random::Uniform() < 0.5)
                    {
                        tmpSstart = id + width;
                        tmpEnd = id;
                    }
                    m_roads.insert(std::make_pair(idVertical, Road(idVertical, GetLength(), GetLimit(), GetLanes(), tmpSstart, tmpEnd, geneVertical == 2)));
                }
            }//create road
            Road* north = i > 0 ? m_crosses[(i - 1) * width + j + 1].GetSouthRoad() : 0;
            Road* west = j > 0 ? m_crosses[id - 1].GetEasthRoad() : 0;
            Road* east = geneHorizon > 0 ? &m_roads[idHorizon] : 0;
            Road* south = geneVertical > 0 ? &m_roads[idVertical] : 0;
            Cross& cross = m_crosses.insert(std::make_pair(id, Cross(id
                , north != 0 ? north->GetId() : -1
                , east != 0 ? east->GetId() : -1
                , south != 0 ? south->GetId() : -1
                , west != 0 ? west->GetId() : -1
                ))).first->second;
            if (north != 0) cross.SetNorthRoad(north);
            if (west != 0) cross.SetWestRoad(west);
            if (east != 0) cross.SetEasthRoad(east);
            if (south != 0) cross.SetSouthRoad(south);
        }
    }
    for (auto ite = m_roads.begin(); ite != m_roads.end(); ++ite)
    {
        ite->second.SetStartCross(&m_crosses[ite->second.GetStartCrossId()]);
        ite->second.SetEndCross(&m_crosses[ite->second.GetEndCrossId()]);
    }
}

bool MapGenerator::CheckPathValid() const
{
    int arriveCount = 1;
    int backCount = 1;
    std::map<int, bool> arriveMap;
    std::map<int, bool> backMap;
    for (auto ite = m_crosses.begin(); ite != m_crosses.end(); ++ite)
    {
        arriveMap[ite->first] = ite == m_crosses.begin();
        backMap[ite->first] = ite == m_crosses.begin();
    }
    while (true)
    {
        int oriArriveCount = arriveCount;
        int oriBackCount = backCount;
        for (auto ite = m_crosses.begin(); ite != m_crosses.end(); ++ite)
        {
            bool canArrive = arriveMap[ite->first];
            bool canBack = backMap[ite->first];
            DirectionType_Foreach(dir,
                Road* road = ite->second.GetRoad(dir);
                if (road != 0)
                {
                    Cross* peer = road->GetPeerCross(&(ite->second));
                    if (canArrive && road->CanStartFrom(ite->first) && !arriveMap[peer->GetId()])
                    {
                        arriveMap[peer->GetId()] = true;
                        ++arriveCount;
                    }
                    if (canBack && road->CanReachTo(ite->first) && !backMap[peer->GetId()])
                    {
                        backMap[peer->GetId()] = true;
                        ++backCount;
                    }
                }
            );
        }
        if (oriArriveCount == arriveCount && oriBackCount == backCount)
            return false;
        if (arriveCount == m_crosses.size() && backCount == m_crosses.size())
            break;
    }
    return true;
}

void MapGenerator::SaveToFile() const
{
    LOG("saving cross information to file");
    {
        std::ofstream ofs(Config::PathCross);
        ASSERT(ofs.is_open());
        for (auto ite = m_crosses.begin(); ite != m_crosses.end(); ++ite)
        {
            const Cross& cross = ite->second;
            ofs << '(' << cross.GetId()
                << ", " << cross.GetNorthRoadId()
                << ", " << cross.GetEasthRoadId()
                << ", " << cross.GetSouthhRoadId()
                << ", " << cross.GetWestRoadId()
                << ")\n";
        }
        ofs.flush();
        ofs.close();
    }
    LOG("saving road information to file");
    {
        std::ofstream ofs(Config::PathRoad);
        ASSERT(ofs.is_open());
        for (auto ite = m_roads.begin(); ite != m_roads.end(); ++ite)
        {
            const Road& road = ite->second;
            ofs << '(' << road.GetId()
                << ", " << road.GetLength()
                << ", " << road.GetLimit()
                << ", " << road.GetLanes()
                << ", " << road.GetStartCrossId()
                << ", " << road.GetEndCrossId()
                << ", " << (road.GetIsTwoWay() ? '1' : '0')
                << ")\n";
        }
        ofs.flush();
        ofs.close();
    }
    LOG("saving car information to file");
    {
        std::ofstream ofs(Config::PathCar);
        ASSERT(ofs.is_open());
        for (auto ite = m_cars.begin(); ite != m_cars.end(); ++ite)
        {
            const Car& car = ite->second;
            ofs << '(' << car.GetId()
                << ", " << car.GetFromCrossId()
                << ", " << car.GetToCrossId()
                << ", " << car.GetMaxSpeed()
                << ", " << car.GetPlanTime()
                << ", " << car.GetIsVip()
                << ", " << (m_preset.find(car.GetId()) != m_preset.end() ? '1' : '0')
                << ")\n";
        }
        ofs.flush();
        ofs.close();
    }
    LOG("saving preset information to file");
    {
        std::ofstream ofs(Config::PathPreset);
        ASSERT(ofs.is_open());
        for (auto ite = m_preset.begin(); ite != m_preset.end(); ++ite)
        {
            const SimCar* car = ite->second;
            ASSERT(car->GetCar()->GetPlanTime() <= m_presetMaxRealTime);
            ofs << '(' << ite->first << ", " << (int)Random::Uniform(car->GetCar()->GetPlanTime(), m_presetMaxRealTime + 1);
            ASSERT(car->GetTrace().Head() != car->GetTrace().Tail());
            for (auto traceIte = car->GetTrace().Head(); traceIte != car->GetTrace().Tail(); ++traceIte)
            {
                ofs << ", " << *traceIte;
            }
            ofs << ")\n";
        }
        ofs.flush();
        ofs.close();
    }

}

void MapGenerator::Generate(int argc, char *argv[])
{
    Config::Initialize(argc, argv);
    int tryCount = 10;
    LOG("try generate valid map");
    while (--tryCount > 0)
    {
        m_crosses.clear();
        m_roads.clear();
        GenerateMap();
        if (CheckPathValid())
            break;
    }
    ASSERT_MSG(tryCount > 0, "plz change the random seed and regenerate");
    LOG("generating cars");
    int maxCrossId = m_width * m_height;
    for (int i = 0; i < m_carsN; ++i)
    {
        int start = Random::Uniform(0, maxCrossId) + 1;
        int dealta = Random::Uniform(1, maxCrossId);
        int end = (start + dealta) % maxCrossId + 1;
        int id = i + 10001;
        m_cars.insert(std::make_pair(id, Car(id, start, end, GetSpeed(), GetPlanTime(), Random::Uniform() < m_vipProb, false)));
    }
    SaveToFile();
    LOG("finding path");
    Scenario::Initialize();
    SimScenario scenario;
    SchedulerFloyd scheduler;
    scheduler.Initialize(scenario);
    int time = 0;
    scheduler.Update(time, scenario);
    LOG("configuring preset car");
    for (auto ite = scenario.Cars().begin(); ite != scenario.Cars().end(); ++ite)
    {
        if (Random::Uniform() < m_presetProb)
            m_preset[ite->first] = &(ite->second);
    }
    SaveToFile();
    LOG("generation complete");
}
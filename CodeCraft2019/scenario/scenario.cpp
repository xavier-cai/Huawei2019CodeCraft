#include "scenario.h"
#include "file-reader.h"
#include "assert.h"
#include "config.h"
#include "log.h"
#include "tactics.h"

Scenario Scenario::Instance;

Scenario::Scenario()
{
    m_cars.reserve(100000);
    m_crosses.reserve(300);
    m_roads.reserve(400);
}

template <typename _T>
void ClearVector(std::vector<_T*> &vector)
{
    for (uint i = 0; i < vector.size(); ++i)
        delete vector[i];
    vector.clear();
}

Scenario::~Scenario()
{
    ClearVector(m_cars);
    ClearVector(m_crosses);
    ClearVector(m_roads);
}

bool HandleIntStream(std::istream& is, int argc, int* argv)
{
    char c;
    is >> c;
    if (c == '#')
        return false;
    if (c != '(')
        ASSERT(false);
    for (int i = 0; i < argc; ++i)
    {
        argv[i] = -1;
        is >> argv[i] >> c;
        ASSERT_MSG(c == (i == argc - 1 ? ')' : ','), "char=" << c << " position=" << i);
    }
    return true;
}

bool Scenario::HandleCar(std::istream& is)
{
    int argc = 7;
    int* contents = new int[argc];
    if (HandleIntStream(is, argc, contents))
    {
        int id = contents[0];
        ASSERT(m_carsIndexMap.find(id) == m_carsIndexMap.end());
        m_carsIndexMap[id] = m_cars.size();
        m_cars.push_back(new Car(id, m_cars.size(), contents[1], contents[2], contents[3], contents[4], contents[5] == 1, contents[6] == 1));
    }
    delete[] contents;
    return true;
}

bool Scenario::HandleCross(std::istream& is)
{
    int argc = 5;
    int* contents = new int[argc];
    if (HandleIntStream(is, argc, contents))
    {
        int id = contents[0];
        ASSERT(m_crossesIndexMap.find(id) == m_crossesIndexMap.end());
        m_crossesIndexMap[id] = m_crosses.size();
        m_crosses.push_back(new Cross(id, m_crosses.size(), contents[1], contents[2], contents[3], contents[4]));
    }
    delete[] contents;
    return true;
}

bool Scenario::HandleRoad(std::istream& is)
{
    int argc = 7;
    int* contents = new int[argc];
    if (HandleIntStream(is, argc, contents))
    {
        int id = contents[0];
        ASSERT(m_roadsIndexMap.find(id) == m_roadsIndexMap.end());
        m_roadsIndexMap[id] = m_roads.size();
        ASSERT(contents[6] == 1 || contents[6] == 0);
        m_roads.push_back(new Road(id, m_roads.size(), contents[1], contents[2], contents[3], contents[4], contents[5], contents[6] == 1));
    }
    delete[] contents;
    return true;
}

bool Scenario::HandleAnswer(std::istream& is)
{
    char c;
    is >> c;
    if (c == '#')
        return true;
    if (c != '(')
        ASSERT(false);
    int argv[2];
    for (int i = 0; i < 2; ++i)
    {
        argv[i] = -1;
        is >> argv[i] >> c;
        ASSERT_MSG(c == ',', "char=" << c << " position=" << i);
    }
    ASSERT(argv[0] >= 0 && argv[1] >= 0);
    int id = MapCarOriginToIndex(argv[0]);
    Tactics::Instance.GetRealTimes()[id] = argv[1];
    int path;
    while (true)
    {
        path = -1;
        is >> path >> c;
        ASSERT(c == ',' || c == ')');
        ASSERT(path >= 0);
        auto find = m_roadsIndexMap.find(path);
        ASSERT(find != m_roadsIndexMap.end());
        int pathId = find->second;
        Tactics::Instance.GetTraces()[id].AddToTail(pathId);
        if (c == ')')
            break;
    }
    return true;
}
    
void Scenario::DoInitialize()
{
    ClearVector(m_cars);
    ClearVector(m_crosses);
    ClearVector(m_roads);
    m_garageSize.clear();
    m_garageInnerIndex.clear();
    m_carsIndexMap.clear();
    m_crossesIndexMap.clear();
    m_roadsIndexMap.clear();
    m_vipCarsN = 0;
    m_presetCarsN = 0;
    FileReader reader;
    bool result;
    LOG("read information of cars from " << Config::PathCar);
    result = reader.Read(Config::PathCar.c_str(), Callback::Create(&Scenario::HandleCar, this));
    ASSERT(result);
    LOG("read information of crosses from " << Config::PathCross);
    result = reader.Read(Config::PathCross.c_str(), Callback::Create(&Scenario::HandleCross, this));
    ASSERT(result);
    LOG("read information of roads from " << Config::PathRoad);
    result = reader.Read(Config::PathRoad.c_str(), Callback::Create(&Scenario::HandleRoad, this));
    ASSERT(result);

    m_garageSize.resize(m_crosses.size(), 0);
    m_garageInnerIndex.resize(m_cars.size(), -1);

    for (uint i = 0; i < m_cars.size(); ++i)
    {
        Car* car = m_cars[i];
        ASSERT((int)i == car->GetId());
        //check origin input
        ASSERT(m_crossesIndexMap.find(car->GetFromCrossId()) != m_crossesIndexMap.end());
        ASSERT(m_crossesIndexMap.find(car->GetToCrossId()) != m_crossesIndexMap.end());
        //replace information to index
        car->SetFromCrossId(m_crossesIndexMap[car->GetFromCrossId()]);
        car->SetToCrossId(m_crossesIndexMap[car->GetToCrossId()]);
        //set ptr
        car->SetFromCross(m_crosses[car->GetFromCrossId()]);
        car->SetToCross(m_crosses[car->GetToCrossId()]);
        if (car->GetIsVip())
            ++m_vipCarsN;
        if (car->GetIsPreset())
            ++m_presetCarsN;
        m_garageInnerIndex[i] = m_garageSize[car->GetFromCrossId()]++;
    }
    for (uint i = 0; i < m_crosses.size(); ++i)
    {
        Cross* cross = m_crosses[i];
        ASSERT((int)i == cross->GetId());
        if (cross->GetNorthRoadId() != -1)
        {
            ASSERT(m_roadsIndexMap.find(cross->GetNorthRoadId()) != m_roadsIndexMap.end());
            cross->SetNorthRoadId(m_roadsIndexMap[cross->GetNorthRoadId()]);
            cross->SetNorthRoad(m_roads[cross->GetNorthRoadId()]);
        }
        if (cross->GetEasthRoadId() != -1)
        {
            ASSERT(m_roadsIndexMap.find(cross->GetEasthRoadId()) != m_roadsIndexMap.end());
            cross->SetEasthRoadId(m_roadsIndexMap[cross->GetEasthRoadId()]);
            cross->SetEasthRoad(m_roads[cross->GetEasthRoadId()]);
        }
        if (cross->GetSouthRoadId() != -1)
        {
            ASSERT(m_roadsIndexMap.find(cross->GetSouthRoadId()) != m_roadsIndexMap.end());
            cross->SetSouthRoadId(m_roadsIndexMap[cross->GetSouthRoadId()]);
            cross->SetSouthRoad(m_roads[cross->GetSouthRoadId()]);
        }
        if (cross->GetWestRoadId() != -1)
        {
            ASSERT(m_roadsIndexMap.find(cross->GetWestRoadId()) != m_roadsIndexMap.end());
            cross->SetWestRoadId(m_roadsIndexMap[cross->GetWestRoadId()]);
            cross->SetWestRoad(m_roads[cross->GetWestRoadId()]);
        }
    }
    for (uint i = 0; i < m_roads.size(); ++i)
    {
        Road* road = m_roads[i];
        ASSERT((int)i == road->GetId());
        ASSERT(m_crossesIndexMap.find(road->GetStartCrossId()) != m_crossesIndexMap.end());
        ASSERT(m_crossesIndexMap.find(road->GetEndCrossId()) != m_crossesIndexMap.end());
        road->SetStartCrossId(m_crossesIndexMap[road->GetStartCrossId()]);
        road->SetEndCrossId(m_crossesIndexMap[road->GetEndCrossId()]);
        road->SetStartCross(m_crosses[road->GetStartCrossId()]);
        road->SetEndCross(m_crosses[road->GetEndCrossId()]);
    }
}

void Scenario::DoMoreInitialize()
{
    LOG("read information of preset from " << Config::PathPreset);
    bool result = FileReader().Read(Config::PathPreset.c_str(), Callback::Create(&Scenario::HandleAnswer, this));
    ASSERT(result);
}

void Scenario::Initialize()
{
    Instance.DoInitialize();
    Reset();
}

void Scenario::Reset()
{
    Tactics::Instance.Initialize();
    Instance.DoMoreInitialize();
}

const int& Scenario::GetGarageSize(const int& id)
{
    ASSERT((int)Instance.m_garageSize.size() > id);
    return Instance.m_garageSize[id];
}

const int& Scenario::GetGarageInnerIndex(const int& carId)
{
    ASSERT((int)Instance.m_garageInnerIndex.size() > carId);
    return Instance.m_garageInnerIndex[carId];
}

const int& Scenario::MapCarOriginToIndex(const int& origin)
{
    auto find = Instance.m_carsIndexMap.find(origin);
    ASSERT(find != Instance.m_carsIndexMap.end());
    return find->second;
}

const int& Scenario::MapCrossOriginToIndex(const int& origin)
{
    auto find = Instance.m_crossesIndexMap.find(origin);
    ASSERT(find != Instance.m_crossesIndexMap.end());
    return find->second;
}

const int& Scenario::MapRoadOriginToIndex(const int& origin)
{
    auto find = Instance.m_roadsIndexMap.find(origin);
    ASSERT(find != Instance.m_roadsIndexMap.end());
    return find->second;
}

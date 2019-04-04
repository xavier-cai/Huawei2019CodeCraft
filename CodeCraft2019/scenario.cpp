#include "scenario.h"
#include "file-reader.h"
#include "assert.h"
#include "config.h"
#include "log.h"
#include "tactics.h"

Scenario Scenario::Instance;

Scenario::Scenario()
{ }

template <typename _TK, typename _TV>
void ClearMapValue(std::map<_TK, _TV*> &map)
{
    for (auto ite = map.begin(); ite != map.end(); ++ite)
        delete ite->second;
    map.clear();
}

Scenario::~Scenario()
{
    m_memoryPool.Release();
    ClearMapValue(m_cars);
    ClearMapValue(m_crosses);
    ClearMapValue(m_roads);
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
        ASSERT(id >= 0 && m_cars.find(id) == m_cars.end());
        m_cars[id] = new Car(id, contents[1], contents[2], contents[3], contents[4], contents[5] == 1, contents[6] == 1);
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
        ASSERT(id >= 0 && m_crosses.find(id) == m_crosses.end());
        m_crosses[id] = new Cross(id, contents[1], contents[2], contents[3], contents[4]);
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
        ASSERT(id >= 0 && m_roads.find(id) == m_roads.end());
        ASSERT(contents[6] == 1 || contents[6] == 0);
        m_roads[id] = new Road(id, contents[1], contents[2], contents[3], contents[4], contents[5], contents[6] == 1);
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
    auto find = m_cars.find(argv[0]);
    ASSERT(find != m_cars.end());
    Tactics::Instance.GetRealTimes()[argv[0]] = argv[1];
    int path;
    while (true)
    {
        path = -1;
        is >> path >> c;
        ASSERT(c == ',' || c == ')');
        ASSERT(path >= 0);
        Tactics::Instance.GetTraces()[argv[0]].AddToTail(path);
        if (c == ')')
            break;
    }
    return true;
}
    
void Scenario::DoInitialize()
{
    ClearMapValue(m_cars);
    ClearMapValue(m_crosses);
    ClearMapValue(m_roads);
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
    LOG("read information of preset from " << Config::PathPreset);
    result = reader.Read(Config::PathPreset.c_str(), Callback::Create(&Scenario::HandleAnswer, this));
    ASSERT(result);

    m_carIndexer = IndexerEnhanced<int>();
    m_crossIndexer = IndexerEnhanced<int>();
    m_roadIndexer = IndexerEnhanced<int>();
    m_memoryPool.Release();
    m_carArray = m_memoryPool.Manage(new MapArray<int, Car>(Scenario::CalculateIndexArraySize(Scenario::Cars()), false));
    m_carArray->ReplaceIndexer(IndexerMirror<int>(m_carIndexer));
    m_crossArray = m_memoryPool.Manage(new MapArray<int, Cross>(Scenario::CalculateIndexArraySize(Scenario::Crosses()), false));
    m_crossArray->ReplaceIndexer(IndexerMirror<int>(m_crossIndexer));
    m_roadArray = m_memoryPool.Manage(new MapArray<int, Road>(Scenario::CalculateIndexArraySize(Scenario::Roads()), false));
    m_roadArray->ReplaceIndexer(IndexerMirror<int>(m_roadIndexer));
    
    int index;
    index = 0;
    DirectionType_Foreach(dir,
        m_directionIndexer.Input(dir, index++);
    );
    index = 0;
    for (auto ite = m_cars.begin(); ite != m_cars.end(); ++ite, ++index)
    {
        Car* car = ite->second;
        ASSERT(m_crosses.find(car->GetFromCrossId()) != m_crosses.end());
        ASSERT(m_crosses.find(car->GetToCrossId()) != m_crosses.end());
        car->SetFromCross(m_crosses[car->GetFromCrossId()]);
        car->SetToCross(m_crosses[car->GetToCrossId()]);
        //indexer
        if (index != 0)
            for ( ; ite->first != index + m_carIndexer.GetDiffer(); ++index)
                m_carArray->ReplaceDataByIndex(index, 0);
        m_carIndexer.Input(ite->first, index);
        m_carArray->ReplaceDataByIndex(index, car);
    }
    index = 0;
    for (auto ite = m_crosses.begin(); ite != m_crosses.end(); ++ite, ++index)
    {
        Cross* cross = ite->second;
        if (cross->GetNorthRoadId() != -1)
        {
            ASSERT(m_roads.find(cross->GetNorthRoadId()) != m_roads.end());
            cross->SetNorthRoad(m_roads[cross->GetNorthRoadId()]);
        }
        if (cross->GetEasthRoadId() != -1)
        {
            ASSERT(m_roads.find(cross->GetEasthRoadId()) != m_roads.end());
            cross->SetEasthRoad(m_roads[cross->GetEasthRoadId()]);
        }
        if (cross->GetSouthhRoadId() != -1)
        {
            ASSERT(m_roads.find(cross->GetSouthhRoadId()) != m_roads.end());
            cross->SetSouthhRoad(m_roads[cross->GetSouthhRoadId()]);
        }
        if (cross->GetWestRoadId() != -1)
        {
            ASSERT(m_roads.find(cross->GetWestRoadId()) != m_roads.end());
            cross->SetWestRoad(m_roads[cross->GetWestRoadId()]);
        }
        //indexer
        if (index != 0)
            for ( ; ite->first != index + m_crossIndexer.GetDiffer(); ++index)
                m_crossArray->ReplaceDataByIndex(index, 0);
        m_crossIndexer.Input(ite->first, index);
        m_crossArray->ReplaceDataByIndex(index, cross);
    }
    index = 0;
    for (auto ite = m_roads.begin(); ite != m_roads.end(); ++ite, ++index)
    {
        Road* road = ite->second;
        ASSERT(m_crosses.find(road->GetStartCrossId()) != m_crosses.end());
        ASSERT(m_crosses.find(road->GetEndCrossId()) != m_crosses.end());
        road->SetStartCross(m_crosses[road->GetStartCrossId()]);
        road->SetEndCross(m_crosses[road->GetEndCrossId()]);
        //indexer
        if (index != 0)
            for ( ; ite->first != index + m_roadIndexer.GetDiffer(); ++index)
                m_roadArray->ReplaceDataByIndex(index, 0);
        m_roadIndexer.Input(ite->first, index);
        m_roadArray->ReplaceDataByIndex(index, road);
    }
}

void Scenario::Initialize()
{
    Tactics::Instance.Initialize();
    Instance.DoInitialize();
}

const std::map<int, Car*>& Scenario::Cars()
{
    return Instance.m_cars;
}

const std::map<int, Cross*>& Scenario::Crosses()
{
    return Instance.m_crosses;
}

const std::map<int, Road*>& Scenario::Roads()
{
    return Instance.m_roads;
}

/*
Car* Scenario::GetCar(const int& id)
{
    auto find = Instance.m_cars.find(id);
    if (find == Instance.m_cars.end())
        return 0;
    return find->second;
}

Cross* Scenario::GetCross(const int& id)
{
    auto find = Instance.m_crosses.find(id);
    if (find == Instance.m_crosses.end())
        return 0;
    return find->second;
}

Road* Scenario::GetRoad(const int& id)
{
    auto find = Instance.m_roads.find(id);
    if (find == Instance.m_roads.end())
        return 0;
    return find->second;
}
*/

Car* Scenario::GetCar(const int& id)
{
    return Instance.m_carArray->Find(id);
}

Cross* Scenario::GetCross(const int& id)
{
    return Instance.m_crossArray->Find(id);
}

Road* Scenario::GetRoad(const int& id)
{
    return Instance.m_roadArray->Find(id);
}

const IndexerEnhanced<int>& Scenario::GetCarIndexer()
{
    return Instance.m_carIndexer;
}

const IndexerEnhanced<int>& Scenario::GetCrossIndexer()
{
    return Instance.m_crossIndexer;
}

const IndexerEnhanced<int>& Scenario::GetRoadIndexer()
{
    return Instance.m_roadIndexer;
}

const IndexerEnhanced<Cross::DirectionType>& Scenario::GetDirectionIndexer()
{
    return Instance.m_directionIndexer;
}

const int& Scenario::GetVipCarsN()
{
    static int vipCarsN = -1;
    if (vipCarsN < 0)
    {
        vipCarsN = 0;
        for (auto ite = Cars().begin(); ite != Cars().end(); ++ite)
            if (ite->second->GetIsVip())
                ++vipCarsN;
    }
    return vipCarsN;
}
#ifndef SCENARIO_H
#define SCENARIO_H

#include <map>
#include "car.h"
#include "cross.h"
#include "road.h"
#include <iostream>
#include "map-array.h"
#include "memory-pool.h"

class Scenario
{
private:
    std::map<int, Car*> m_cars;
    std::map<int, Cross*> m_crosses;
    std::map<int, Road*> m_roads;
    MemoryPool m_memoryPool;
    MapArray<int, Car>* m_carArray;
    MapArray<int, Cross>* m_crossArray;
    MapArray<int, Road>* m_roadArray;
    IndexerEnhanced<int> m_carIndexer; //for indexing Car ID to index
    IndexerEnhanced<int> m_crossIndexer; //for indexing Cross ID to index
    IndexerEnhanced<int> m_roadIndexer; //for indexing Road ID to index
    IndexerEnhanced<Cross::DirectionType> m_directionIndexer; //for indexing Direction to index
    
    Scenario();
    static Scenario Instance;
    
    bool HandleCar(std::istream& is);
    bool HandleCross(std::istream& is);
    bool HandleRoad(std::istream& is);
    void DoInitialize();
    
public:
    ~Scenario();
    
    static void Initialize();
    static const std::map<int, Car*>& Cars();
    static const std::map<int, Cross*>& Crosses();
    static const std::map<int, Road*>& Roads();
    static Car* GetCar(const int& id);
    static Cross* GetCross(const int& id);
    static Road* GetRoad(const int& id);
    static const IndexerEnhanced<int>& GetCarIndexer();
    static const IndexerEnhanced<int>& GetCrossIndexer();
    static const IndexerEnhanced<int>& GetRoadIndexer();
    static const IndexerEnhanced<Cross::DirectionType>& GetDirectionIndexer();
    
};//class Scenario

#define _a(id) Scenario::GetCarIndexer().Output(id)
#define _c(id) Scenario::GetCrossIndexer().Output(id)
#define _r(id) Scenario::GetRoadIndexer().Output(id)
#define _d(id) Scenario::GetDirectionIndexer().Output(id)

#endif

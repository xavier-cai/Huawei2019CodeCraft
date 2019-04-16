#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>
#include <map>
#include "car.h"
#include "cross.h"
#include "road.h"
#include <iostream>
#include "map-array.h"
#include "memory-pool.h"

typedef unsigned int uint;

class Scenario
{
private:
    std::vector<Car*> m_cars;
    std::vector<Cross*> m_crosses;
    std::vector<Road*> m_roads;
    std::vector<int> m_garageSize;
    std::vector<int> m_garageInnerIndex;

    std::map<int, int> m_carsIndexMap;
    std::map<int, int> m_crossesIndexMap;
    std::map<int, int> m_roadsIndexMap;

    int m_vipCarsN;
    int m_presetCarsN;
    
    Scenario();
    static Scenario Instance;
    
    bool HandleCar(std::istream& is);
    bool HandleCross(std::istream& is);
    bool HandleRoad(std::istream& is);
    bool HandleAnswer(std::istream& is);
    void DoInitialize();
    void DoMoreInitialize();
    
public:
    ~Scenario();
    
    static void Initialize();
    static const std::vector<Car*>& Cars();
    static const std::vector<Cross*>& Crosses();
    static const std::vector<Road*>& Roads();
    static const int& GetGarageSize(const int& id);
    static const int& GetGarageInnerIndex(const int& carId);
    static const int& MapCarOriginToIndex(const int& origin);
    static const int& MapCrossOriginToIndex(const int& origin);
    static const int& MapRoadOriginToIndex(const int& origin);
    static const int& GetVipCarsN();
    static const int& GetPresetCarsN();
    
};//class Scenario

#endif

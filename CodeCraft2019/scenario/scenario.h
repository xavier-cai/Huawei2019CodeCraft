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
    inline static const std::vector<Car*>& Cars();
    inline static const std::vector<Cross*>& Crosses();
    inline static const std::vector<Road*>& Roads();
    inline static const int& GetVipCarsN();
    inline static const int& GetPresetCarsN();

    static const int& GetGarageSize(const int& id);
    static const int& GetGarageInnerIndex(const int& carId);
    static const int& MapCarOriginToIndex(const int& origin);
    static const int& MapCrossOriginToIndex(const int& origin);
    static const int& MapRoadOriginToIndex(const int& origin);
    
};//class Scenario





/* 
 * [inline functions]
 *   it's not good to write code here, but we really need inline!
 */

inline const std::vector<Car*>& Scenario::Cars()
{
    return Instance.m_cars;
}

inline const std::vector<Cross*>& Scenario::Crosses()
{
    return Instance.m_crosses;
}

inline const std::vector<Road*>& Scenario::Roads()
{
    return Instance.m_roads;
}

inline const int& Scenario::GetVipCarsN()
{
    return Instance.m_vipCarsN;
}

inline const int& Scenario::GetPresetCarsN()
{
    return Instance.m_presetCarsN;
}

#endif

#ifndef SIM_SCENARIO_H
#define SIM_SCENARIO_H

#include "sim-car.h"
#include "sim-road.h"
#include <map>
#include <vector>

class SimScenario
{
protected:
    std::map< int, std::list<SimCar*> > m_simGarages;
    std::map<int, SimRoad> m_simRoads;
    std::map<int, SimCar> m_simCars;
    unsigned int m_reachCarsN;
    unsigned int m_carInGarageN;
    
public:
    SimScenario();
    virtual ~SimScenario();
    SimScenario(const SimScenario& o);
    SimScenario& operator = (const SimScenario& o);
    
    std::map< int, std::list<SimCar*> >& Garages();
    std::map<int, SimRoad>& Roads();
    std::map<int, SimCar>& Cars();
    void ReachGoal(unsigned int n = 1);
    void GetoutOnRoad(unsigned int n = 1);
    bool IsComplete() const;
    int GetOnRoadCarsN() const;
    
    void SaveToFile() const;
    void SaveToFile(const char* file) const;

};//class SimScenario

#endif

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
    /* for calculating score */
    int m_scheduledTime;
    int m_totalCompleteTime;
    int m_vipFirstPlanTime;
    int m_vipLastReachTime;
    int m_vipTotalCompleteTime;
    
public:
    SimScenario();
    virtual ~SimScenario();
    SimScenario(const SimScenario& o);
    SimScenario& operator = (const SimScenario& o);

    const int& GetScheduledTime() const;
    const int& GetTotalCompleteTime() const;
    int GetVipScheduledTime() const;
    const int& GetVipTotalCompleteTime() const;
    
    std::map< int, std::list<SimCar*> >& Garages();
    std::map<int, SimRoad>& Roads();
    std::map<int, SimCar>& Cars();
    void NotifyCarGetoutOnRoad(const int& time, const SimCar* car);
    void NotifyCarReachGoal(const int& time, const SimCar* car);
    bool IsComplete() const;
    int GetOnRoadCarsN() const;
    
    void SaveToFile() const;
    void SaveToFile(const char* file) const;

};//class SimScenario

#endif

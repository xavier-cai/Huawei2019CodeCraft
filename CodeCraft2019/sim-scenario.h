#ifndef SIM_SCENARIO_H
#define SIM_SCENARIO_H

#include "sim-car.h"
#include "sim-road.h"
#include <map>
#include <vector>
#include "quick-map.h"

class SimScenario
{
protected:
    QuickMap< int, std::list<SimCar*> > m_simGarages;
    QuickMap<int, SimRoad> m_simRoads;
    QuickMap<int, SimCar> m_simCars;
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
    
    QuickMap< int, std::list<SimCar*> >& Garages();
    QuickMap<int, SimRoad>& Roads();
    QuickMap<int, SimCar>& Cars();
    void NotifyCarGetoutOnRoad(const int& time, const SimCar* car);
    void NotifyCarReachGoal(const int& time, const SimCar* car);
    bool IsComplete() const;
    int GetOnRoadCarsN() const;
    
    void SaveToFile() const;
    void SaveToFile(const char* file) const;

};//class SimScenario

#endif

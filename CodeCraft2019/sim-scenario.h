#ifndef SIM_SCENARIO_H
#define SIM_SCENARIO_H

#include "sim-car.h"
#include "sim-road.h"
#include <map>
#include <vector>
#include "scenario.h"

class SimScenario
{
protected:
    std::vector< std::vector<SimCar*> > m_simGarages; //indexed by cross id
    std::vector<SimRoad*> m_simRoads;
    std::vector<SimCar*> m_simCars;
    unsigned int m_reachCarsN;
    unsigned int m_carOnRoadN;
    unsigned int m_carInGarageN;
    /* for calculating score */
    int m_scheduledTime;
    int m_totalCompleteTime;
    int m_vipFirstPlanTime;
    int m_vipLastReachTime;
    int m_vipTotalCompleteTime;
    
public:
    SimScenario(bool onlyPreset = false);
    virtual ~SimScenario();
    SimScenario(const SimScenario& o);
    SimScenario& operator = (const SimScenario& o);

    const int& GetScheduledTime() const;
    const int& GetTotalCompleteTime() const;
    int GetVipScheduledTime() const;
    const int& GetVipTotalCompleteTime() const;
    
    const std::vector< std::vector<SimCar*> >& Garages() const;
    const std::vector<SimRoad*>& Roads() const;
    const std::vector<SimCar*>& Cars() const;
    void NotifyCarGetoutOnRoad(const int& time, const SimCar* car);
    void NotifyCarReachGoal(const int& time, const SimCar* car);
    bool IsComplete() const;
    const unsigned int& GetCarInGarageN() const;
    const unsigned int& GetReachCarsN() const;
    int GetOnRoadCarsN() const;

    SimCar* AddCar(Car* car);
    void RemoveCar(SimCar* car);
    //void ReplaceGarage(const std::vector< std::vector<SimCar*> >& garage); 
    void ResetScenario();
    
    void SaveToFile() const;
    void SaveToFile(const char* file) const;

private:
    void Clear();

};//class SimScenario

#endif

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "sim-scenario.h"
#include <utility>

class Scheduler;

class Simulator
{  
public:
    struct UpdateResult
    {
        bool Conflict;

    };//struct UpdateResult

private:
    Simulator();

    Scheduler* m_scheduler;

    int m_scheduledCarsN; //counter
    int m_reachedCarsN; //counter
    bool m_conflictFlag; //for checking conflict, reset in each schedule cycle
    std::vector< std::vector<SimCar*> > m_vipCarsInGarage; //cross id -> vector of garage cars

    /* for handle callback */
    void HandleUpdateState(const SimCar::SimState& state);

    /* for notify scheduler */
    void NotifyFirstPriority(const int& time, SimScenario& scenario, SimCar* car) const;

    /* for notify simulator itself */
    void NotifyScheduleStart();
    void NotifyScheduleCycleStart();
    bool GetIsDeadLock(SimScenario& scenario) const;
    bool GetIsCompleted(SimScenario& scenario) const;

    /* internal functions */
    int GetPositionInNextRoad(const int& time, SimScenario& scenario, SimCar* car) const;
    SimCar* PeekFirstPriorityCarOnRoad(const int& time, SimScenario& scenario, const SimRoad* road, const int& crossId) const;
    bool PassCrossOrJustForward(const int& time, SimScenario& scenario, SimCar* car);
    bool GetCarOutFromGarage(const int& time, SimScenario& scenario, SimCar* car) const;
    void GetOutFromGarage(const int& time, SimScenario& scenario) const;
    void InitializeVipCarsInGarage(const int& time, SimScenario& scenario);
    void GetVipOutFromGarage(const int& time, SimScenario& scenario, const int& crossId = -1, const int& roadId = -1);

    /* for logging */
    void PrintCrossState(const int& time, SimScenario& scenario, Cross* cross) const;
    void PrintDeadLock(const int& time, SimScenario& scenario) const;

    /* cheater */
    bool m_isEnableCheater;

public:
    static Simulator Instance;

    void SetScheduler(Scheduler* scheduler);
    UpdateResult Update(const int& time, SimScenario& scenario);
    std::pair<int, int> CanCarGetOutFromGarage(const int& time, SimScenario& scenario, SimCar* car) const;
    void GetDeadLockCars(const int& time, SimScenario& scenario, std::list<SimCar*>& result, int n = -1) const; //n <= 0 means get all dead lock cars

    /* static functions */
    static void SetEnableCheater(const bool& enable);

};//class Simulator

#endif

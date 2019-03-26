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
    SimCar* PeekFirstPriorityCarOnRoad(const int& time, SimScenario& scenario, SimRoad* road, const int& crossId) const;
    SimCar* CheckFirstPriorityCarOnRoad(const int& time, SimScenario& scenario, SimRoad* road, const int& crossId) const;
    bool PassCrossOrJustForward(const int& time, SimScenario& scenario, SimCar* car);
    void GetOutFromGarage(const int& time, SimScenario& scenario) const;

    /* for logging */
    void PrintCrossState(const int& time, SimScenario& scenario, Cross* cross) const;
    void PrintDeadLock(const int& time, SimScenario& scenario) const;

public:
    static Simulator Instance;

    void SetScheduler(Scheduler* scheduler);
    UpdateResult Update(const int& time, SimScenario& scenario);
    void GetDeadLockCars(const int& time, SimScenario& scenario, std::list<SimCar*>& result, int n = -1) const; //n <= 0 means get all dead lock cars

};//class Simulator

#endif

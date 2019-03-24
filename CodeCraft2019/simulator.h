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
    
    enum TrunType
    {
        LEFT,
        DIRECT,
        RIGHT
    };

private:
    int m_time;
    SimScenario& m_scenario;

    static void NotifyFirstPriority(const int& time, SimScenario& scenario, SimCar* car);

    Simulator();
    Simulator(const int& time, SimScenario& scenario);
    UpdateResult UpdateSelf() const;

public:
    static void Initialize();
    static Simulator InstanceAgent;

    static Scheduler* SchedulerPtr;
    static UpdateResult UpdateSelf(const int& time, SimScenario& scenario);
    static bool ConflictFlag;
    static int ScheduledCarsN;
    static int ReachedCarsN;

    static Simulator::TrunType GetDirection(int from, int to);
    static int GetRoadPeer(int to, Simulator::TrunType dir, bool opposite);
    static int GetPositionInNextRoad(const int& time, SimScenario& scenario, SimCar* car);
    static SimCar* PeekFirstPriorityCarOnRoad(const int& time, SimScenario& scenario, SimRoad* road, const int& crossId);
    static SimCar* CheckFirstPriorityCarOnRoad(const int& time, SimScenario& scenario, SimRoad* road, const int& crossId);
    static bool PassCrossOrJustForward(const int& time, SimScenario& scenario, SimCar* car);
    void GetOutFromGarage(const int& time, SimScenario& scenario) const;
    void PrintCrossState(const int& time, SimScenario& scenario, Cross* cross) const;
    void PrintDeadLock(const int& time, SimScenario& scenario) const;
    
};//class Simulator

std::ostream& operator << (std::ostream& os, const Simulator::TrunType& turn);

#endif

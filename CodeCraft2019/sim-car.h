#ifndef SIM_CAR_H
#define SIM_CAR_H

#include "car.h"
#include "road.h"
#include "cross.h"
#include "trace.h"
#include "callback.h"

class SimScenario;

class SimCar
{
public:
    enum SimState
    {
        UNSCHEDULED,
        WAITING,
        SCHEDULED
    };
    
private:
    Car* m_car;
    SimScenario* m_scenario;
    
    int* m_realTime;
    Trace* m_trace; //diffirent simulation cars (SimCar) that sharing same ID are also sharing the same trace
    bool m_isInGarage;
    bool m_isReachGoal;
    bool m_isLockOnNextRoad; //if the car beacame the first priority, it can not changes its next road
    int m_lockOnNextRoadTime;
    bool m_isIgnored; //special state flag for debugging, which means it will not be updated, then cause simulation run into a dead loop
    int m_startTime; //the time go on the first road
    bool m_isForceOutput;
    int m_calculateTimeCache;
    
    /* indicate update state of car in simulation */
    int m_lastUpdateTime;
    SimState m_simState;
    SimCar* m_waitingCar;
    
    /* indicate position of car in simulation */
    int m_currentTraceIndex;
    //Trace::Node m_currentTraceNode;
    Road* m_currentRoad;
    int m_currentLane; //[1~number of lanes]
    bool m_currentDirection; //[true]: current road start->end, [false]: current road end->start
    int m_currentPosition; //[1~road length]
    
    void SetSimState(int time, SimState state);
    /* invoked when state changed by above function */
    static Callback::Handle1<void, const SimState&> m_updateStateNotifier;
    void NotifyUpdateState(const SimState& state) const;
    /* notify load changed */
    static Callback::Handle2<void, const SimCar*, Road*> m_updateGoOnNewRoad;
    static Callback::Handle1<void, const SimCar*> m_updateCarScheduled;
    
public:
    SimCar();
    SimCar(Car* car);

    void Reset();
    void SetScenario(SimScenario* scenario);
    void SetIsIgnored(const bool& ignored);
    void SetIsForceOutput(const bool& forceOutput);

    Car* GetCar() const;
    void SetRealTime(int realTime);
    int GetRealTime() const;
    Trace& GetTrace();
    const Trace& GetTrace() const;
    const bool& GetIsReachedGoal() const;
    const bool& GetIsInGarage() const;
    void LockOnNextRoad(const int& time);
    const bool& GetIsLockOnNextRoad() const;
    const int& GetLockOnNextRoadTime() const;
    const bool& GetIsIgnored() const;
    const int& GetStartTime() const;
    const bool& GetIsForceOutput() const;
    
    int GetNextRoadId() const; //-1 means reaching end cross
    int GetLastUpdateTime() const;
    SimState GetSimState(int time);
    SimCar* GetWaitingCar(int time);
    
    const int& GetCurrentTraceIndex() const;
    //Trace::Node GetCurrentTraceNode(); //iterator of the next road ID
    //Trace::NodeConst GetCurrentTraceNode() const;
    Road* GetCurrentRoad() const;
    int GetCurrentLane() const;
    bool GetCurrentDirection() const;
    int GetCurrentPosition() const;
    Cross* GetCurrentCross() const;
    Cross::TurnType GetCurrentTurnType() const;
    
    /* functions for updating state */
    void UpdateOnRoad(int time, Road* road, int lane, bool direction, int position); //go on the new road
    void UpdatePosition(int time, int position);
    void UpdateWaiting(int time, SimCar* waitingCar);
    void UpdateReachGoal(int time);
    void UpdateStayInGarage(int time);

    //[CAUTION : this callback is used by simulator]
    static void SetUpdateStateNotifier(const Callback::Handle1<void, const SimState&>& notifier);
    /* callbacks below can be used in scheduler */
    static void SetUpdateGoOnNewRoadNotifier(const Callback::Handle2<void, const SimCar*, Road*>& notifier);
    static void SetUpdateCarScheduledNotifier(const Callback::Handle1<void, const SimCar*>& notifier);

    int CalculateArriveTime(bool useCache);
    
};//class SimCar

#endif

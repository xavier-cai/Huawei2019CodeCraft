#ifndef SIM_CAR_H
#define SIM_CAR_H

#include "car.h"
#include "road.h"
#include "trace.h"
#include "callback.h"

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
    
    int* m_realTime;
    Trace* m_trace; //diffirent simulation cars (SimCar) that sharing same ID are also sharing the same trace
    bool m_isInGarage;
    bool m_isReachGoal;
    bool m_isLockOnNextRoad; //if the car beacame the first priority, it can not changes its next road
    bool m_isIgnored; //special state flag for debugging, which means it will not be updated, then cause simulation run into a dead loop
    
    /* indicate update state of car in simulation */
    int m_lastUpdateTime;
    SimState m_simState;
    SimCar* m_waitingCar;
    
    /* indicate position of car in simulation */
    int m_currentTraceIndex;
    Trace::Node m_currentTraceNode;
    Road* m_currentRoad;
    int m_currentLane; //[1~number of lanes]
    bool m_currentDirection; //[true]: current road start->end, [false]: current road end->start
    int m_currentPosition; //[1~road length]
    
    void SetSimState(int time, SimState state);
    /* invoked when state changed by above function */
    static Callback::Handle1<void, const SimState&> m_updateStateNotifier;
    //static void (*m_updateStateNotifier)(const SimState&);
    static void NotifyUpdateState(const SimState& state);
    
public:
    SimCar();
    SimCar(Car* car);

    void SetIsIgnored(const bool& ignored);

    Car* GetCar() const;
    void SetRealTime(int realTime);
    int GetRealTime() const;
    Trace& GetTrace();
    const Trace& GetTrace() const;
    const bool& GetIsReachedGoal() const;
    const bool& GetIsInGarage() const;
    void LockOnNextRoad();
    const bool& GetIsLockOnNextRoad() const;
    const bool& GetIsIgnored() const;
    
    int GetNextRoadId() const; //-1 means reaching end cross
    SimState GetSimState(int time);
    SimCar* GetWaitingCar(int time);
    
    const int& GetCurrentTraceIndex() const;
    Trace::Node GetCurrentTraceNode(); //iterator of the next road ID
    Trace::NodeConst GetCurrentTraceNode() const;
    Road* GetCurrentRoad() const;
    int GetCurrentLane() const;
    bool GetCurrentDirection() const;
    int GetCurrentPosition() const;
    Cross* GetCurrentCross() const;
    
    /* functions for updating state */
    void UpdateOnRoad(int time, Road* road, int lane, bool direction, int position); //go on the new road
    void UpdatePosition(int time, int position);
    void UpdateWaiting(int time, SimCar* waitingCar);
    void UpdateReachGoal(int time);
    void UpdateStayInGarage(int time);

    static void SetUpdateStateNotifier(const Callback::Handle1<void, const SimState&>& notifier);
    
};//class SimCar

#endif

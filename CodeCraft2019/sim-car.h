#ifndef SIM_CAR_H
#define SIM_CAR_H

#include "car.h"
#include "road.h"
#include "trace.h"

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
    
    int m_realTime;
    Trace m_trace;
    bool m_isInGarage;
    bool m_isReachGoal;
    bool m_isLockOnNextRoad;
    
    int m_lastUpdateTime;
    SimState m_simState;
    SimCar* m_waitingCar;
    
    Road* m_currentRoad;
    int m_currentLane; //[1~number of lanes]
    bool m_currentDirection; //[true]: current road start->end, [false]: current road end->start
    int m_currentPosition; //[1~road length]
    
    void SetSimState(int time, SimState state);
    
public:
    SimCar();
    SimCar(Car* car);

    Car* GetCar() const;
    void SetRealTime(int realTime);
    int GetRealTime() const;
    Trace& GetTrace();
    const Trace& GetTrace() const;
    const bool& GetIsReachedGoal() const;
    const bool& GetIsInGarage() const;
    void LockOnNextRoad();
    const bool& GetIsLockOnNextRoad() const;
    
    int GetNextRoadId() const; //-1 means reaching end cross
    SimState GetSimState(int time);
    SimCar* GetWaitingCar(int time);
    
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

    static void SetUpdateStateNotifier(void (*notifier)(const SimState&));
    
};//class SimCar

#endif

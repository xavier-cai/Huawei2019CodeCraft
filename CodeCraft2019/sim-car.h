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
    int m_calculateTimeToken;
    
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

    inline Car* GetCar() const;
    inline void SetRealTime(int realTime);
    inline int GetRealTime() const;
    inline Trace& GetTrace();
    inline const Trace& GetTrace() const;
    inline const bool& GetIsReachedGoal() const;
    inline const bool& GetIsInGarage() const;
    inline void LockOnNextRoad(const int& time);
    inline const bool& GetIsLockOnNextRoad() const;
    inline const int& GetLockOnNextRoadTime() const;
    inline const bool& GetIsIgnored() const;
    inline const int& GetStartTime() const;
    inline const bool& GetIsForceOutput() const;
    
    inline int GetNextRoadId() const; //-1 means reaching end cross
    inline int GetLastUpdateTime() const;
    inline SimState GetSimState(int time);
    inline SimCar* GetWaitingCar(int time);
    
    inline const int& GetCurrentTraceIndex() const;
    inline Road* GetCurrentRoad() const;
    inline int GetCurrentLane() const;
    inline bool GetCurrentDirection() const;
    inline int GetCurrentPosition() const;
    inline Cross* GetCurrentCross() const;
    inline Cross::TurnType GetCurrentTurnType() const;
    
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

    int CalculateSpendTime(bool useCache);
    int CalculateSpendTime(const int& token);
    
};//class SimCar





/* 
 * [inline functions]
 *   it's not good to write code here, but we really need inline!
 */

inline Car* SimCar::GetCar() const
{
    return m_car;
}

inline void SimCar::SetRealTime(int realTime)
{
    ASSERT(!m_car->GetIsPreset());
    ASSERT(realTime >= m_car->GetPlanTime());
    *m_realTime = realTime;
}

inline int SimCar::GetRealTime() const
{
    return *m_realTime;
}

inline Trace& SimCar::GetTrace()
{
    return *m_trace;
}

inline const Trace& SimCar::GetTrace() const
{
    return *m_trace;
}

inline const bool& SimCar::GetIsReachedGoal() const
{
    return m_isReachGoal;
}

inline const bool& SimCar::GetIsInGarage() const
{
    return m_isInGarage;
}

inline void SimCar::LockOnNextRoad(const int& time)
{
    m_isLockOnNextRoad = true;
    m_lockOnNextRoadTime = time;
}

inline const bool& SimCar::GetIsLockOnNextRoad() const
{
    return m_isLockOnNextRoad;
}

inline const int& SimCar::GetLockOnNextRoadTime() const
{
    return m_lockOnNextRoadTime >= 0 ? m_lockOnNextRoadTime : m_startTime;
}

inline const bool& SimCar::GetIsIgnored() const
{
    return m_isIgnored;
}

inline const int& SimCar::GetStartTime() const
{
    return m_startTime;
}

inline const bool& SimCar::GetIsForceOutput() const
{
    return m_isForceOutput;
}

inline int SimCar::GetNextRoadId() const
{
    return (*m_trace)[m_currentTraceIndex];
}

inline void SimCar::SetSimState(int time, SimState state)
{
    m_lastUpdateTime = time;
    m_simState = state;
    m_waitingCar = 0;
    
    //notify state changed
    if (!m_updateStateNotifier.IsNull())
        m_updateStateNotifier.Invoke(state);
    if (!m_updateCarScheduled.IsNull() && state == SCHEDULED)
        m_updateCarScheduled.Invoke(this);
}

inline int SimCar::GetLastUpdateTime() const
{
    return m_lastUpdateTime;
}

inline SimCar::SimState SimCar::GetSimState(int time)
{
    if (m_lastUpdateTime < 0) //inside the garage
        return UNSCHEDULED;
    if (time != m_lastUpdateTime)
    {
        ASSERT_MSG(m_simState == SCHEDULED, "the car must be scheduled in last time chip");
        SetSimState(time, UNSCHEDULED);
    }
    return m_simState;
}

inline SimCar* SimCar::GetWaitingCar(int time)
{
    ASSERT(GetSimState(time) == WAITING);
    ASSERT(m_waitingCar != 0);
    return m_waitingCar;
}

inline const int& SimCar::GetCurrentTraceIndex() const
{
    return m_currentTraceIndex;
}

inline Road* SimCar::GetCurrentRoad() const
{
    return m_currentRoad;
}

inline int SimCar::GetCurrentLane() const
{
    ASSERT(m_currentRoad != 0);
    return m_currentLane;
}

inline bool SimCar::GetCurrentDirection() const
{
    ASSERT(m_currentRoad != 0);
    return m_currentDirection;
}

inline int SimCar::GetCurrentPosition() const
{
    ASSERT(m_currentRoad != 0);
    return m_currentPosition;
}

inline Cross* SimCar::GetCurrentCross() const
{
    //ASSERT(m_currentRoad != 0);
    if (m_currentRoad == 0) //still in garage
    {
        ASSERT(m_isInGarage);
        return m_car->GetFromCross();
    }
    return m_currentDirection ? m_currentRoad->GetEndCross() : m_currentRoad->GetStartCross();
}

inline Cross::TurnType SimCar::GetCurrentTurnType() const
{
    int nextRoadId = GetNextRoadId();
    ASSERT(nextRoadId >= 0 || GetCurrentCross() == m_car->GetToCross());
    ASSERT(m_currentRoad != 0);
    return nextRoadId < 0 ? Cross::DIRECT : GetCurrentCross()->GetTurnDirection(m_currentRoad->GetId(), nextRoadId);
}

#endif

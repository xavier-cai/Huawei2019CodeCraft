#include "sim-car.h"
#include "assert.h"
#include "log.h"
#include "tactics.h"

Callback::Handle<void, const SimCar::SimState&> SimCar::m_updateStateNotifier(0);
void SimCar::NotifyUpdateState(const SimCar::SimState& state)
{
    if(!m_updateStateNotifier.IsNull())
        m_updateStateNotifier.Invoke(state);
}

SimCar::SimCar()
{
    ASSERT(false);
}

SimCar::SimCar(Car* car)
    : m_car(car), m_realTime(car->GetPlanTime()), m_trace(&Tactics::Instance.GetTraces()[car->GetId()])
    , m_isInGarage(true), m_isReachGoal(false), m_isLockOnNextRoad(false), m_isIgnored(false)
    , m_lastUpdateTime(-1), m_simState(SCHEDULED), m_waitingCar(0)
    , m_currentRoad(0), m_currentLane(0), m_currentDirection(true), m_currentPosition(0)
{
    ASSERT(car != 0);
    m_currentTraceNode = m_trace->Head();
}

void SimCar::SetIsIgnored(const bool& ignored)
{
    m_isIgnored = ignored;
}

Car* SimCar::GetCar() const
{
    return m_car;
}

void SimCar::SetRealTime(int realTime)
{
    ASSERT(realTime >= m_car->GetPlanTime());
    m_realTime = realTime;
}

int SimCar::GetRealTime() const
{
    return m_realTime;
}

Trace& SimCar::GetTrace()
{
    return *m_trace;
}

const Trace& SimCar::GetTrace() const
{
    return *m_trace;
}

const bool& SimCar::GetIsReachedGoal() const
{
    return m_isReachGoal;
}

const bool& SimCar::GetIsInGarage() const
{
    return m_isInGarage;
}

void SimCar::LockOnNextRoad()
{
    m_isLockOnNextRoad = true;
}

const bool& SimCar::GetIsLockOnNextRoad() const
{
    return m_isLockOnNextRoad;
}

const bool& SimCar::GetIsIgnored() const
{
    return m_isIgnored;
}

int SimCar::GetNextRoadId() const
{
    return *m_currentTraceNode;
}

void SimCar::SetSimState(int time, SimState state)
{
    m_lastUpdateTime = time;
    m_simState = state;
    m_waitingCar = 0;
    NotifyUpdateState(state);
}

SimCar::SimState SimCar::GetSimState(int time)
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

SimCar* SimCar::GetWaitingCar(int time)
{
    ASSERT(GetSimState(time) == WAITING);
    ASSERT(m_waitingCar != 0);
    return m_waitingCar;
}

Trace::Node& SimCar::GetCurrentTraceNode()
{
    return m_currentTraceNode;
}

Road* SimCar::GetCurrentRoad() const
{
    return m_currentRoad;
}

int SimCar::GetCurrentLane() const
{
    ASSERT(m_currentRoad != 0);
    return m_currentLane;
}

bool SimCar::GetCurrentDirection() const
{
    ASSERT(m_currentRoad != 0);
    return m_currentDirection;
}

int SimCar::GetCurrentPosition() const
{
    ASSERT(m_currentRoad != 0);
    return m_currentPosition;
}

Cross* SimCar::GetCurrentCross() const
{
    ASSERT(m_currentRoad != 0);
    return m_currentDirection ? m_currentRoad->GetEndCross() : m_currentRoad->GetStartCross();
}

void SimCar::UpdateOnRoad(int time, Road* road, int lane, bool direction, int position)
{
    auto state = GetSimState(time);
    ASSERT_MSG(state != SCHEDULED, "the car is already be scheduled");
    //if (state == WAITING)
    //    ASSERT_MSG(GetWaitingCar(time)->GetSimState(time) == SCHEDULED, "the waiting car need be scheduled first");
    m_isLockOnNextRoad = false;
    SetSimState(time, SCHEDULED); //update state
    auto nextId= GetNextRoadId(); //for checking road id
    if (nextId == -1) //reaching goal
    {
        LOG("@" << time << " the car [" << m_car->GetId() << "] reach the goal");
        ASSERT(road == 0);
        if (m_car->GetFromCrossId() != m_car->GetToCrossId())
        {
            ASSERT(m_currentRoad != 0);
            auto currentCrossId = m_currentDirection ? m_currentRoad->GetEndCrossId() : m_currentRoad->GetStartCrossId();
            ASSERT_MSG(currentCrossId == m_car->GetToCrossId(), "not reach the goal cross yet, now:" << currentCrossId << " expect:" << m_car->GetToCrossId());
        }
        m_isReachGoal = true;
        return;
    }
    LOG("@" << time << " the car [" << m_car->GetId() << "] go on the road " << road->GetId()
        << " lane " << lane
        << " from " << (direction ? road->GetStartCrossId() : road->GetEndCrossId())
        << " to " << (direction ? road->GetEndCrossId() : road->GetStartCrossId())
        << " position " << position);
    ASSERT(road != 0);
    ASSERT(road->GetId() == nextId);
    ASSERT(lane > 0 && lane <= road->GetLanes());
    ASSERT(direction || road->GetIsTwoWay());
    ASSERT(position > 0 && position <= road->GetLength());
    if (m_currentRoad == 0) //go out the garage
    {
        auto startCrossId = direction ? road->GetStartCrossId() : road->GetEndCrossId();
        ASSERT_MSG(startCrossId == m_car->GetFromCrossId(), "not start form the correct cross, now:" << startCrossId << " expect:" << m_car->GetFromCrossId());
        m_isInGarage = false;
    }
    else
    {
        ASSERT(m_currentTraceNode != m_trace->Head());
        ASSERT(*(m_currentTraceNode - 1) == m_currentRoad->GetId());
    }
    ASSERT_MSG(road->GetId() == GetNextRoadId(), "not on the correct road, now:" << road->GetId() << " expect:" << GetNextRoadId());
    ++m_currentTraceNode;
    m_currentRoad = road;
    m_currentLane = lane;
    m_currentDirection = direction;
    m_currentPosition = position;
}

void SimCar::UpdatePosition(int time, int position)
{
    LOG("@" << time << " the car [" << m_car->GetId() << "] move from " << m_currentPosition << " to " << position
        << " on road " << m_currentRoad->GetId()
        << " lane " << m_currentLane
        << " dir " << m_currentDirection);
    auto state = GetSimState(time);
    ASSERT_MSG(state != SCHEDULED, "the car is already be scheduled");
    ASSERT_MSG(m_currentRoad != 0, "the car is still in garage");
    SetSimState(time, SCHEDULED); //update state
    ASSERT(position > 0 && position <= m_currentRoad->GetLength());
    m_currentPosition = position;
}

void SimCar::UpdateWaiting(int time, SimCar* waitingCar)
{
    auto state = GetSimState(time);
    ASSERT(waitingCar != this);
    ASSERT_MSG(state != SCHEDULED, "the car is already be scheduled");
    ASSERT_MSG(m_currentRoad != 0, "the car is still in garage");
    if(state != WAITING || waitingCar != m_waitingCar)
    {
        SetSimState(time, WAITING); //update state
        ASSERT(waitingCar != 0);
        ASSERT_MSG(waitingCar->GetSimState(time) != SCHEDULED, "the waiting car is already be scheduled");
        m_waitingCar = waitingCar;
    }
}

void SimCar::UpdateReachGoal(int time)
{
    UpdateOnRoad(time, 0, 0, true, 0);
}

void SimCar::SetUpdateStateNotifier(const Callback::Handle<void, const SimState&>& notifier)
{
    m_updateStateNotifier = notifier;
}

#include "car.h"
#include "cross.h"
#include "assert.h"

Car::Car()
{
    ASSERT(false);
}

Car::Car(int id, int fromCrossId, int toCrossId, int maxSpeed, int planTime)
    : m_id(id), m_fromCrossId(fromCrossId), m_toCrossId(toCrossId), m_maxSpeed(maxSpeed), m_planTime(planTime)
    , m_fromCross(0), m_toCross(0)
{ }

Car::~Car()
{ }

int Car::GetId() const
{
    return m_id;
}

int Car::GetFromCrossId() const
{
    return m_fromCrossId;
}

int Car::GetToCrossId() const
{
    return m_toCrossId;
}

int Car::GetMaxSpeed() const
{
    return m_maxSpeed;
}

int Car::GetPlanTime() const
{
    return m_planTime;
}

void Car::SetFromCrossId(int id)
{
    m_fromCrossId = id;

}
void Car::SetToCrossId(int id)
{
    m_toCrossId = id;
}

void Car::SetMaxSpeed(int speed)
{
    m_maxSpeed = speed;
}

void Car::SetFromCross(Cross* cross)
{
    ASSERT(cross->GetId() == m_fromCrossId);
    m_fromCross = cross;
}

void Car::SetToCross(Cross* cross)
{
    ASSERT(cross->GetId() == m_toCrossId);
    m_toCross = cross;
}

Cross* Car::GetFromCross() const
{
    ASSERT(m_fromCross != 0);
    return m_fromCross;
}

Cross* Car::GetToCross() const
{
    ASSERT(m_toCross != 0);
    return m_toCross;
}


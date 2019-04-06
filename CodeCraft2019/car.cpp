#include "car.h"
#include "cross.h"
#include "assert.h"

Car::Car()
{
    ASSERT(false);
}

Car::Car(const int& id, const int& fromCrossId, const int& toCrossId, const int& maxSpeed, const int& planTime, const bool& isVip, const bool& isPreset)
    : m_id(id), m_fromCrossId(fromCrossId), m_toCrossId(toCrossId), m_maxSpeed(maxSpeed), m_planTime(planTime)
    , m_isVip(isVip), m_isPreset(isPreset)
    , m_fromCross(0), m_toCross(0)
{ }

Car::~Car()
{ }

const int& Car::GetId() const
{
    return m_id;
}

const int& Car::GetFromCrossId() const
{
    return m_fromCrossId;
}

const int& Car::GetToCrossId() const
{
    return m_toCrossId;
}

const int& Car::GetMaxSpeed() const
{
    return m_maxSpeed;
}

const int& Car::GetPlanTime() const
{
    return m_planTime;
}

const bool& Car::GetIsVip() const
{
    return m_isVip;
}

const bool& Car::GetIsPreset() const
{
    return m_isPreset;
}

void Car::SetFromCrossId(const int& id)
{
    m_fromCrossId = id;

}
void Car::SetToCrossId(const int& id)
{
    m_toCrossId = id;
}

void Car::SetMaxSpeed(const int& speed)
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
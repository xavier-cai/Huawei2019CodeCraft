#include "car.h"
#include "assert.h"
#include "cross.h"

Car::Car()
{
    ASSERT(false);
}

Car::Car(const int& origin, const int& id, const int& fromCrossId, const int& toCrossId, const int& maxSpeed, const int& planTime, const bool& isVip, const bool& isPreset)
    : m_originId(origin), m_id(id), m_fromCrossId(fromCrossId), m_toCrossId(toCrossId), m_maxSpeed(maxSpeed), m_planTime(planTime)
    , m_isVip(isVip), m_isPreset(isPreset)
    , m_fromCross(0), m_toCross(0)
{ }

Car::~Car()
{ }

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
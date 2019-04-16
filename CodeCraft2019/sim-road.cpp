#include "sim-road.h"
#include "assert.h"

SimRoad::SimRoad()
{
    ASSERT(false);
}

SimRoad::SimRoad(Road* road)
    : m_road(road)
{
    ASSERT(road != 0);
    m_carSize = road->GetLanes() * (road->GetIsTwoWay() ? 2 : 1);
    m_cars.resize(m_carSize);
    for (int i = 0; i < m_carSize; ++i)
        m_cars[i].reserve(m_road->GetLength());
}

void SimRoad::Reset()
{
    for (int i = 0; i < m_carSize; ++i)
        m_cars[i].clear();
}

Road* SimRoad::GetRoad() const
{
    return m_road;
}

const std::vector<Car*>& SimRoad::GetCarsImpl(const int& lane) const
{
    ASSERT(lane > 0 && lane <= m_road->GetLanes());
    ASSERT(lane <= m_carSize);
    return m_cars[lane - 1];
}

const std::vector<Car*>& SimRoad::GetCarsOppositeImpl(const int& lane) const
{
    ASSERT(lane > 0 && lane <= m_road->GetLanes());
    ASSERT(m_road->GetLanes() + lane <= m_carSize);
    return m_cars[m_road->GetLanes() + lane - 1];
}

const std::vector<Car*>& SimRoad::GetCarsImpl(const int& lane, bool opposite) const
{
    return opposite ? GetCarsOppositeImpl(lane) : GetCarsImpl(lane);
}

std::vector<Car*>& SimRoad::GetCarsImpl(const int& lane, bool opposite)
{
    if (opposite)
    {
        ASSERT(lane > 0 && lane <= m_road->GetLanes());
        ASSERT(m_road->GetLanes() + lane <= m_carSize);
        return m_cars[m_road->GetLanes() + lane - 1];
    }
    ASSERT(lane > 0 && lane <= m_road->GetLanes());
    ASSERT(lane <= m_carSize);
    return m_cars[lane - 1];
}

const std::vector<Car*>& SimRoad::GetCars(const int& lane) const
{
    return GetCarsImpl(lane);
}

const std::vector<Car*>& SimRoad::GetCarsOpposite(const int& lane) const
{
    return GetCarsOppositeImpl(lane);
}

const std::vector<Car*>& SimRoad::GetCars(const int& lane, bool opposite) const
{
    return opposite ? GetCarsOppositeImpl(lane) : GetCarsImpl(lane);
}

const std::vector<Car*>& SimRoad::GetCarsTo(const int& lane, const int& crossId) const
{
    return GetCars(lane, IsFromOrTo(crossId));
}

const std::vector<Car*>& SimRoad::GetCarsFrom(const int& lane, const int& crossId) const
{
    return GetCars(lane, !IsFromOrTo(crossId));
}

bool SimRoad::IsFromOrTo(const int& crossId) const
{
    return m_road->IsFromOrTo(crossId);
}

void SimRoad::RunIn(Car* car, const int& lane, const bool& opposite)
{
    GetCarsImpl(lane, opposite).push_back(car);
}

Car* SimRoad::RunOut(const int& lane, const bool& opposite)
{
    auto& cars = GetCarsImpl(lane, opposite);
    ASSERT(cars.size() > 0);
    Car* ret = *cars.begin();
    cars.erase(cars.begin());
    return ret;
}
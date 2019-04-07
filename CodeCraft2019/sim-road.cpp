#include "sim-road.h"
#include "assert.h"

SimRoad::SimRoad()
{
    ASSERT(false);
}

SimRoad::SimRoad(Road* road)
    : m_road(road), m_cars(0)
{
    ASSERT(road != 0);
    m_carSize = road->GetLanes() * (road->GetIsTwoWay() ? 2 : 1);
    m_cars = new std::list<Car*>[m_carSize];
    for (int i = 0; i < m_carSize; ++i)
        m_lists.push_back(&m_cars[i]);
}

SimRoad::SimRoad(const SimRoad& o)
    : m_road(o.m_road), m_cars(0)
{
    *this = o;
}

SimRoad& SimRoad::operator = (const SimRoad& o)
{
    m_road = o.m_road;
    m_carSize = o.m_carSize;
    if (m_cars != 0)
        delete[] m_cars;
    int size = m_carSize;
    m_cars = new std::list<Car*>[size];
    for (int i = 0; i < size; ++i)
        m_cars[i] = o.m_cars[i];
    m_lists.clear();
    for (int i = 0; i < m_carSize; ++i)
        m_lists.push_back(&m_cars[i]);
    return *this;
}

SimRoad::~SimRoad()
{
    delete[] m_cars;
}

Road* SimRoad::GetRoad() const
{
    return m_road;
}

std::list<Car*>& SimRoad::GetCarsImpl(const int& lane) const
{
    ASSERT(lane > 0 && lane <= m_road->GetLanes());
    ASSERT(lane <= m_carSize);
    return m_cars[lane - 1];
}

std::list<Car*>& SimRoad::GetCarsOppositeImpl(const int& lane) const
{
    ASSERT(lane > 0 && lane <= m_road->GetLanes());
    ASSERT(m_road->GetLanes() + lane <= m_carSize);
    return m_cars[m_road->GetLanes() + lane - 1];
}

std::list<Car*>& SimRoad::GetCarsImpl(const int& lane, bool opposite) const
{
    return opposite ? GetCarsOppositeImpl(lane) : GetCarsImpl(lane);
}

const std::list<Car*>& SimRoad::GetCars(const int& lane) const
{
    return GetCarsImpl(lane);
}

const std::list<Car*>& SimRoad::GetCarsOpposite(const int& lane) const
{
    return GetCarsOppositeImpl(lane);
}

const std::list<Car*>& SimRoad::GetCars(const int& lane, bool opposite) const
{
    return opposite ? GetCarsOppositeImpl(lane) : GetCarsImpl(lane);
}

const std::list<Car*>& SimRoad::GetCarsTo(const int& lane, const int& crossId) const
{
    return GetCars(lane, IsFromOrTo(crossId));
}

const std::list<Car*>& SimRoad::GetCarsFrom(const int& lane, const int& crossId) const
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
    cars.pop_front();
    return ret;
}
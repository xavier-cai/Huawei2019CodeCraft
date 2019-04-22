#include "sim-road.h"
#include "assert.h"
#include "sim-scenario.h"
#include <algorithm>

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
#ifndef SIM_ROAD_H
#define SIM_ROAD_H

#include "road.h"
#include "car.h"
#include <vector>

class SimScenario;

class SimRoad
{
private:
    Road* m_road;
    int m_carSize;
    int m_carN;
    std::vector< std::vector<Car*> > m_cars; //lane -> car list

    /* implements */
    inline const std::vector<Car*>& GetCarsImpl(const int& lane) const;
    inline const std::vector<Car*>& GetCarsOppositeImpl(const int& lane) const;
    inline const std::vector<Car*>& GetCarsImpl(const int& lane, bool opposite) const;
    inline std::vector<Car*>& GetCarsImpl(const int& lane, bool opposite);
    
public:
    SimRoad();
    SimRoad(Road* road);
    
    void Reset();
    inline Road* GetRoad() const;

    /* const interfaces */
    inline const int& GetCarN() const;
    inline const std::vector<Car*>& GetCars(const int& lane) const; //lane : [1~number of lanes]
    inline const std::vector<Car*>& GetCarsOpposite(const int& lane) const;
    inline const std::vector<Car*>& GetCars(const int& lane, bool opposite) const; //opposite : [true] means end->start; [false] means start->end
    inline const std::vector<Car*>& GetCarsTo(const int& lane, const int& crossId) const;
    inline const std::vector<Car*>& GetCarsFrom(const int& lane, const int& crossId) const;

    /* functions for running a car & changing the list */
    void RunIn(Car* car, const int& lane, const bool& opposite);
    Car* RunOut(const int& lane, const bool& opposite);
    
};//class SimRoad





/* 
 * [inline functions]
 *   it's not good to write code here, but we really need inline!
 */

#include "assert.h"

inline Road* SimRoad::GetRoad() const
{
    return m_road;
}

inline const int& SimRoad::GetCarN() const
{
    return m_carN;
}

inline const std::vector<Car*>& SimRoad::GetCarsImpl(const int& lane) const
{
    ASSERT(lane > 0 && lane <= m_road->GetLanes());
    ASSERT(lane <= m_carSize);
    return m_cars[lane - 1];
}

inline const std::vector<Car*>& SimRoad::GetCarsOppositeImpl(const int& lane) const
{
    ASSERT(lane > 0 && lane <= m_road->GetLanes());
    ASSERT(m_road->GetLanes() + lane <= m_carSize);
    return m_cars[m_road->GetLanes() + lane - 1];
}

inline const std::vector<Car*>& SimRoad::GetCarsImpl(const int& lane, bool opposite) const
{
    return opposite ? GetCarsOppositeImpl(lane) : GetCarsImpl(lane);
}

inline std::vector<Car*>& SimRoad::GetCarsImpl(const int& lane, bool opposite)
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

inline const std::vector<Car*>& SimRoad::GetCars(const int& lane) const
{
    return GetCarsImpl(lane);
}

inline const std::vector<Car*>& SimRoad::GetCarsOpposite(const int& lane) const
{
    return GetCarsOppositeImpl(lane);
}

inline const std::vector<Car*>& SimRoad::GetCars(const int& lane, bool opposite) const
{
    return opposite ? GetCarsOppositeImpl(lane) : GetCarsImpl(lane);
}

inline const std::vector<Car*>& SimRoad::GetCarsTo(const int& lane, const int& crossId) const
{
    return GetCars(lane, m_road->IsFromOrTo(crossId));
}

inline const std::vector<Car*>& SimRoad::GetCarsFrom(const int& lane, const int& crossId) const
{
    return GetCars(lane, !m_road->IsFromOrTo(crossId));
}


#endif

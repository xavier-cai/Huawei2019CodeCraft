#ifndef SIM_ROAD_H
#define SIM_ROAD_H

#include "road.h"
#include <vector>

class SimRoad
{
private:
    Road* m_road;
    int m_carSize;
    std::list<Car*>* m_cars;
    std::vector<std::list<Car*>*> m_lists; //just for visualization in VS

    /* implements */
    std::list<Car*>& GetCarsImpl(const int& lane) const;
    std::list<Car*>& GetCarsOppositeImpl(const int& lane) const;
    std::list<Car*>& GetCarsImpl(const int& lane, bool opposite) const;
    
public:
    SimRoad();
    SimRoad(Road* road);
    SimRoad(const SimRoad& o);
    SimRoad& operator = (const SimRoad& o);
    ~SimRoad();
    
    void Reset();
    Road* GetRoad() const;

    /* const interfaces */
    const std::list<Car*>& GetCars(const int& lane) const; //lane : [1~number of lanes]
    const std::list<Car*>& GetCarsOpposite(const int& lane) const;
    const std::list<Car*>& GetCars(const int& lane, bool opposite) const; //opposite : [true] means end->start; [false] means start->end
    const std::list<Car*>& GetCarsTo(const int& lane, const int& crossId) const;
    const std::list<Car*>& GetCarsFrom(const int& lane, const int& crossId) const;
    bool IsFromOrTo(const int& crossId) const;

    /* functions for running a car & changing the list */
    void RunIn(Car* car, const int& lane, const bool& opposite);
    Car* RunOut(const int& lane, const bool& opposite);
    
};//class SimRoad

#endif

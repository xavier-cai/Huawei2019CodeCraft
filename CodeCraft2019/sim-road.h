#ifndef SIM_ROAD_H
#define SIM_ROAD_H

#include "road.h"
#include <vector>

class SimRoad
{
private:
    Road* m_road;
    int m_carSize;
    std::vector< std::vector<Car*> > m_cars; //lane -> car list

    /* implements */
    const std::vector<Car*>& GetCarsImpl(const int& lane) const;
    const std::vector<Car*>& GetCarsOppositeImpl(const int& lane) const;
    const std::vector<Car*>& GetCarsImpl(const int& lane, bool opposite) const;
    std::vector<Car*>& GetCarsImpl(const int& lane, bool opposite);
    
public:
    SimRoad();
    SimRoad(Road* road);
    
    void Reset();
    Road* GetRoad() const;

    /* const interfaces */
    const std::vector<Car*>& GetCars(const int& lane) const; //lane : [1~number of lanes]
    const std::vector<Car*>& GetCarsOpposite(const int& lane) const;
    const std::vector<Car*>& GetCars(const int& lane, bool opposite) const; //opposite : [true] means end->start; [false] means start->end
    const std::vector<Car*>& GetCarsTo(const int& lane, const int& crossId) const;
    const std::vector<Car*>& GetCarsFrom(const int& lane, const int& crossId) const;
    bool IsFromOrTo(const int& crossId) const;

    /* functions for running a car & changing the list */
    void RunIn(Car* car, const int& lane, const bool& opposite);
    Car* RunOut(const int& lane, const bool& opposite);
    
};//class SimRoad

#endif

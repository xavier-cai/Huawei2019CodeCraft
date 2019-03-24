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
    std::vector<std::list<Car*>*> m_lists;
    
public:
    SimRoad();
    SimRoad(Road* road);
    SimRoad(const SimRoad& o);
    SimRoad& operator = (const SimRoad& o);
    ~SimRoad();
    
    Road* GetRoad() const;

    std::list<Car*>& GetCars(const int& lane) const; //lane : [1~number of lanes]
    std::list<Car*>& GetCarsOpposite(const int& lane) const;
    std::list<Car*>& GetCars(const int& lane, bool opposite) const; //opposite : [true] means end->start; [false] means start->end
    std::list<Car*>& GetCarsTo(const int& lane, const int& crossId) const;
    std::list<Car*>& GetCarsFrom(const int& lane, const int& crossId) const;
    bool IsFromOrTo(const int& crossId) const;
    
};//class SimRoad

#endif

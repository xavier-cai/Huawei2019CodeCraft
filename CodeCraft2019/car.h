#ifndef CAR_H
#define CAR_H

class Cross;

class Car
{
private:
    int m_originId;
    int m_id;
    int m_fromCrossId;
    int m_toCrossId;
    int m_maxSpeed;
    int m_planTime;
    bool m_isVip;
    bool m_isPreset;
    
    Cross* m_fromCross;
    Cross* m_toCross;
    
public:
    Car();
    Car(const int& origin, const int& id, const int& fromCrossId, const int& toCrossId, const int& maxSpeed, const int& planTime, const bool& isVip, const bool& isPreset);
    ~Car();
    
    inline const int& GetOriginId() const;
    inline const int& GetId() const;
    inline const int& GetFromCrossId() const;
    inline const int& GetToCrossId() const;
    inline const int& GetMaxSpeed() const;
    inline const int& GetPlanTime() const;
    inline const bool& GetIsVip() const;
    inline const bool& GetIsPreset() const;

    void SetFromCrossId(const int& id);
    void SetToCrossId(const int& id);
    void SetMaxSpeed(const int& speed);
    void SetFromCross(Cross* cross);
    void SetToCross(Cross* cross);
    inline Cross* GetFromCross() const;
    inline Cross* GetToCross() const;
    
};//class Car

#include <iostream>
inline std::ostream& operator << (std::ostream& os, const Car& car);





/* 
 * [inline functions]
 *   it's not good to write code here, but we really need inline!
 */

#include "assert.h"

inline const int& Car::GetOriginId() const
{
    return m_originId;
}

inline const int& Car::GetId() const
{
    return m_id;
}

inline const int& Car::GetFromCrossId() const
{
    return m_fromCrossId;
}

inline const int& Car::GetToCrossId() const
{
    return m_toCrossId;
}

inline const int& Car::GetMaxSpeed() const
{
    return m_maxSpeed;
}

inline const int& Car::GetPlanTime() const
{
    return m_planTime;
}

inline const bool& Car::GetIsVip() const
{
    return m_isVip;
}

inline const bool& Car::GetIsPreset() const
{
    return m_isPreset;
}

inline Cross* Car::GetFromCross() const
{
    ASSERT(m_fromCross != 0);
    return m_fromCross;
}

inline Cross* Car::GetToCross() const
{
    ASSERT(m_toCross != 0);
    return m_toCross;
}

inline std::ostream& operator << (std::ostream& os, const Car& car)
{
    return (os << (car.GetIsPreset() ? "preset " : "")
        << (car.GetIsVip() ? "VIP " : "")
        << "car [" << car.GetOriginId() << "(" << car.GetId() << ")" << "]");
}

#endif

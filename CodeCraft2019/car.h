#ifndef CAR_H
#define CAR_H

class Road;
class Cross;

class Car
{
private:
    int m_id;
    int m_fromCrossId;
    int m_toCrossId;
    int m_maxSpeed;
    int m_planTime;
    
    Cross* m_fromCross;
    Cross* m_toCross;
    
public:
    Car();
    Car(const int& id, const int& fromCrossId, const int& toCrossId, const int& maxSpeed, const int& planTime);
    ~Car();
    
    const int& GetId() const;
    const int& GetFromCrossId() const;
    const int& GetToCrossId() const;
    const int& GetMaxSpeed() const;
    const int& GetPlanTime() const;

    void SetFromCrossId(const int& id);
    void SetToCrossId(const int& id);
    void SetMaxSpeed(const int& speed);
    void SetFromCross(Cross* cross);
    void SetToCross(Cross* cross);
    Cross* GetFromCross() const;
    Cross* GetToCross() const;
    
};//class Car

#endif

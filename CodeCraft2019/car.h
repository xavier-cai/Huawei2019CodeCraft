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
    Car(int id, int fromCrossId, int toCrossId, int maxSpeed, int planTime);
    ~Car();
    
    int GetId() const;
    int GetFromCrossId() const;
    int GetToCrossId() const;
    int GetMaxSpeed() const;
    int GetPlanTime() const;

    void SetFromCrossId(int id);
    void SetToCrossId(int id);
    void SetMaxSpeed(int speed);
    void SetFromCross(Cross* cross);
    void SetToCross(Cross* cross);
    Cross* GetFromCross() const;
    Cross* GetToCross() const;
    
};//class Car

#endif

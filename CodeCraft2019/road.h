#ifndef ROAD_H
#define ROAD_H

#include <list>

class Car;
class Cross;

class Road
{
private:
    int m_id;
    int m_length;
    int m_limit;
    int m_lanes;
    int m_startCrossId;
    int m_endCrossId;
    bool m_isTwoWay;

    Cross* m_startCross;
    Cross* m_endCross;
    
public:
    Road();
    Road(const int& id, const int& length, const int& limit, const int& lanes, const int& startCrossId, const int& endCrossId, const bool& isTwoWay);
    ~Road();
    
    const int& GetId() const;
    const int& GetLength() const;
    const int& GetLimit() const;
    const int& GetLanes() const;
    const int& GetStartCrossId() const;
    const int& GetEndCrossId() const;
    const bool& GetIsTwoWay() const;

    void SetLimit(const int& limit);
    void SetLength(const int& length);
    void SetStartCross(Cross* cross);
    void SetEndCross(Cross* cross);
    Cross* GetStartCross() const;
    Cross* GetEndCross() const;
    Cross* GetPeerCross(const Cross* peer) const;
    bool CanStartFrom(const int& crossId) const;
    bool CanReachTo(const int& crossId) const;
    bool IsFromOrTo(const int& crossId) const;

};//class Road

#endif

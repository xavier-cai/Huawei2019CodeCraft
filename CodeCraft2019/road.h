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
    Road(int id, int length, int limit, int lanes, int startCrossId, int endCrossId, bool isTwoWay);
    ~Road();
    
    int GetId() const;
    int GetLength() const;
    int GetLimit() const;
    int GetLanes() const;
    int GetStartCrossId() const;
    int GetEndCrossId() const;
    bool GetIsTwoWay() const;

    void SetLimit(int limit);
    void SetLength(int length);
    void SetStartCross(Cross* cross);
    void SetEndCross(Cross* cross);
    Cross* GetStartCross() const;
    Cross* GetEndCross() const;
    Cross* GetPeerCross(Cross* peer) const;

};//class Road

#endif

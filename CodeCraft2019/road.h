#ifndef ROAD_H
#define ROAD_H

#include <list>

class Cross;

class Road
{
private:
    int m_originId;
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
    Road(const int& origin, const int& id, const int& length, const int& limit, const int& lanes, const int& startCrossId, const int& endCrossId, const bool& isTwoWay);
    ~Road();
    
    inline const int& GetOriginId() const;
    inline const int& GetId() const;
    inline const int& GetLength() const;
    inline const int& GetLimit() const;
    inline const int& GetLanes() const;
    inline const int& GetStartCrossId() const;
    inline const int& GetEndCrossId() const;
    inline const bool& GetIsTwoWay() const;

    void SetLimit(const int& limit);
    void SetLength(const int& length);
    void SetStartCrossId(const int& id);
    void SetEndCrossId(const int& id);
    void SetStartCross(Cross* cross);
    void SetEndCross(Cross* cross);
    Cross* GetStartCross() const;
    Cross* GetEndCross() const;
    Cross* GetPeerCross(const Cross* peer) const;
    bool CanStartFrom(const int& crossId) const;
    bool CanReachTo(const int& crossId) const;
    bool IsFromOrTo(const int& crossId) const;

};//class Road





/* 
 * [inline functions]
 *   it's not good to write code here, but we really need inline!
 */

#include "assert.h"

inline const int& Road::GetOriginId() const
{
    return m_originId;
}

inline const int& Road::GetId() const
{
    return m_id;
}

inline const int& Road::GetLength() const
{
    return m_length;
}

inline const int& Road::GetLimit() const
{
    return m_limit;
}

inline const int& Road::GetLanes() const
{
    return m_lanes;
}

inline const int& Road::GetStartCrossId() const
{
    return m_startCrossId;
}

inline const int& Road::GetEndCrossId() const
{
    return m_endCrossId;
}

inline const bool& Road::GetIsTwoWay() const
{
    return m_isTwoWay;
}

inline Cross* Road::GetStartCross() const
{
    ASSERT(m_startCross != 0);
    return m_startCross;
}

inline Cross* Road::GetEndCross() const
{
    ASSERT(m_endCross != 0);
    return m_endCross;
}

inline Cross* Road::GetPeerCross(const Cross* peer) const
{
    ASSERT(peer != 0);
    bool opposite = m_endCross == peer;
    ASSERT(opposite || m_startCross == peer);
    return opposite ? m_startCross : m_endCross;
}

inline bool Road::CanStartFrom(const int& crossId) const
{
    return crossId == m_startCrossId || (m_isTwoWay && crossId == m_endCrossId);
}

inline bool Road::CanReachTo(const int& crossId) const
{
    return crossId == m_endCrossId || (m_isTwoWay && crossId == m_startCrossId);
}

inline bool Road::IsFromOrTo(const int& crossId) const
{
    bool ret = m_startCrossId == crossId;
    ASSERT(ret || m_endCrossId == crossId);
    return ret;
}

#endif

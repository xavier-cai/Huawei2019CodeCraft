#include "road.h"
#include "car.h"
#include "cross.h"
#include "assert.h"

Road::Road()
{
    ASSERT(false);
}

Road::Road(const int& id, const int& length, const int& limit, const int& lanes, const int& startCrossId, const int& endCrossId, const bool& isTwoWay)
    : m_id(id), m_length(length), m_limit(limit), m_lanes(lanes), m_startCrossId(startCrossId), m_endCrossId(endCrossId), m_isTwoWay(isTwoWay)
    , m_startCross(0), m_endCross(0)
{ }

Road::~Road()
{ }

const int& Road::GetId() const
{
    return m_id;
}

const int& Road::GetLength() const
{
    return m_length;
}

const int& Road::GetLimit() const
{
    return m_limit;
}

const int& Road::GetLanes() const
{
    return m_lanes;
}

const int& Road::GetStartCrossId() const
{
    return m_startCrossId;
}

const int& Road::GetEndCrossId() const
{
    return m_endCrossId;
}

const bool& Road::GetIsTwoWay() const
{
    return m_isTwoWay;
}

void Road::SetLimit(const int& limit)
{
    m_limit = limit;
}

void Road::SetLength(const int& length)
{
    m_length = length;
}

void Road::SetStartCross(Cross* cross)
{
    ASSERT(cross->GetId() == m_startCrossId);
    m_startCross = cross;
}

void Road::SetEndCross(Cross* cross)
{
    ASSERT(cross->GetId() == m_endCrossId);
    m_endCross = cross;
}

Cross* Road::GetStartCross() const
{
    ASSERT(m_startCross != 0);
    return m_startCross;
}

Cross* Road::GetEndCross() const
{
    ASSERT(m_endCross != 0);
    return m_endCross;
}

Cross* Road::GetPeerCross(Cross* peer) const
{
    ASSERT(peer != 0);
    bool opposite = m_endCross == peer;
    ASSERT(opposite || m_startCross == peer);
    return opposite ? m_startCross : m_endCross;
}

bool Road::CanStartFrom(const int& crossId) const
{
    bool isStart = crossId == m_startCrossId;
    ASSERT(isStart || crossId == m_endCrossId);
    return isStart || m_isTwoWay;
}

bool Road::CanReachTo(const int& crossId) const
{
    bool isEnd = crossId == m_endCrossId;
    ASSERT(isEnd || crossId == m_startCrossId);
    return isEnd || m_isTwoWay;
}

#include "road.h"
#include "car.h"
#include "cross.h"
#include "assert.h"

Road::Road()
{
    ASSERT(false);
}

Road::Road(int id, int length, int limit, int lanes, int startCrossId, int endCrossId, bool isTwoWay)
    : m_id(id), m_length(length), m_limit(limit), m_lanes(lanes), m_startCrossId(startCrossId), m_endCrossId(endCrossId), m_isTwoWay(isTwoWay)
    , m_startCross(0), m_endCross(0)
{ }

Road::~Road()
{ }

int Road::GetId() const
{
    return m_id;
}

int Road::GetLength() const
{
    return m_length;
}

int Road::GetLimit() const
{
    return m_limit;
}

int Road::GetLanes() const
{
    return m_lanes;
}

int Road::GetStartCrossId() const
{
    return m_startCrossId;
}

int Road::GetEndCrossId() const
{
    return m_endCrossId;
}

bool Road::GetIsTwoWay() const
{
    return m_isTwoWay;
}

void Road::SetLimit(int limit)
{
    m_limit = limit;
}

void Road::SetLength(int length)
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

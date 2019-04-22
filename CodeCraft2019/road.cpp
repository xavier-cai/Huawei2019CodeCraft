#include "road.h"
#include "cross.h"

Road::Road()
{
    ASSERT(false);
}

Road::Road(const int& origin, const int& id, const int& length, const int& limit, const int& lanes, const int& startCrossId, const int& endCrossId, const bool& isTwoWay)
    : m_originId(origin), m_id(id), m_length(length), m_limit(limit), m_lanes(lanes), m_startCrossId(startCrossId), m_endCrossId(endCrossId), m_isTwoWay(isTwoWay)
    , m_startCross(0), m_endCross(0)
{ }

Road::~Road()
{ }

void Road::SetLimit(const int& limit)
{
    m_limit = limit;
}

void Road::SetLength(const int& length)
{
    m_length = length;
}

void Road::SetStartCrossId(const int& id)
{
    m_startCrossId = id;
}

void Road::SetEndCrossId(const int& id)
{
    m_endCrossId = id;
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

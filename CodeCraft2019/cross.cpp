#include "cross.h"
#include "road.h"
#include "assert.h"

Cross::Cross()
{
    ASSERT(false);
}

Cross::Cross(const int& id, const int& northRoadId, const int& eastRoadId, const int& southRoadId, const int& westRoadId)
    : m_id(id), m_northRoadId(northRoadId), m_eastRoadId(eastRoadId), m_southRoadId(southRoadId), m_westRoadId(westRoadId)
    , m_north(0), m_east(0), m_south(0), m_west(0)
{
    if (m_northRoadId >= 0) m_directions[m_northRoadId] = NORTH;
    if (m_eastRoadId >= 0) m_directions[m_eastRoadId] = EAST;
    if (m_southRoadId >= 0) m_directions[m_southRoadId] = SOUTH;
    if (m_westRoadId >= 0) m_directions[m_westRoadId] = WEST;
}

Cross::~Cross()
{ }

const int& Cross::GetId() const
{
    return m_id;
}

const int& Cross::GetNorthRoadId() const
{
    return m_northRoadId;
}

const int& Cross::GetEasthRoadId() const
{
    return m_eastRoadId;
}

const int& Cross::GetSouthhRoadId() const
{
    return m_southRoadId;
}

const int& Cross::GetWestRoadId() const
{
    return m_westRoadId;
}

int Cross::GetRoadId(const DirectionType& dir) const
{
    switch(dir)
    {
        case NORTH: return m_northRoadId;
        case EAST: return m_eastRoadId;
        case SOUTH: return m_southRoadId;
        case WEST: return m_westRoadId;
    }
    ASSERT(false);
    return -1;
}

const Cross::DirectionType& Cross::GetDirection(const int& id) const
{
    auto find = m_directions.find(id);
    ASSERT(find != m_directions.end());
    return find->second;
}

Cross::TurnType Cross::GetTurnDirection(const int& from, const int& to) const
{
    switch ((int)GetDirection(to) - (int)GetDirection(from))
    {
    case 1:
    case -3:
        return Cross::LEFT;
    case -1:
    case 3:
        return Cross::RIGHT;
    case 2:
    case -2:
        return Cross::DIRECT;
    default:
        break;
    }
    ASSERT(false);
    return Cross::DIRECT;
}

Cross::DirectionType Cross::GetTurnDestinationDirection(const int& from, const Cross::TurnType& turn) const
{
    int dir = (int)GetDirection(from);
    switch (turn)
    {
    case LEFT: dir += 1; break;
    case DIRECT: dir += 2; break;
    case RIGHT: dir -=1; break;
    default:
        ASSERT(false);
    }
    if (dir < 0)
        dir += DirectionType_Size;
    if (dir >= DirectionType_Size)
        dir %= DirectionType_Size;
    return (DirectionType)dir;
}

Road* Cross::GetTurnDestination(const int& from, const Cross::TurnType& turn) const
{
    return GetRoad(GetTurnDestinationDirection(from, turn));
}

int Cross::GetTurnDestinationId(const int& from, const Cross::TurnType& turn) const
{
    return GetRoadId(GetTurnDestinationDirection(from, turn));
}

void Cross::SetNorthRoad(Road* road)
{
    ASSERT(road->GetId() == m_northRoadId);
    m_north = road;
}

void Cross::SetEasthRoad(Road* road)
{
    ASSERT(road->GetId() == m_eastRoadId);
    m_east = road;
}

void Cross::SetSouthhRoad(Road* road)
{
    ASSERT(road->GetId() == m_southRoadId);
    m_south = road;
}

void Cross::SetWestRoad(Road* road)
{
    ASSERT(road->GetId() == m_westRoadId);
    m_west = road;
}

Road* Cross::GetNorthRoad() const
{
    return m_north;
}

Road* Cross::GetEasthRoad() const
{
    return m_east;
}

Road* Cross::GetSouthhRoad() const
{
    return m_south;
}

Road* Cross::GetWestRoad() const
{
    return m_west;
}

Road* Cross::GetRoad(const Cross::DirectionType& dir) const
{
    switch(dir)
    {
        case NORTH: return m_north;
        case EAST: return m_east;
        case SOUTH: return m_south;
        case WEST: return m_west;
    }
    ASSERT(false);
    return 0;
}

Cross::TurnType operator ! (const Cross::TurnType& t)
{
    switch (t)
    {
    case Cross::LEFT: return Cross::RIGHT;
    case Cross::DIRECT: return Cross::DIRECT;
    case Cross::RIGHT: return Cross::LEFT;
    default:
        break;
    }
    ASSERT(false);
    return Cross::DIRECT;
}

std::ostream& operator << (std::ostream& os, const Cross::TurnType& turn)
{
    switch (turn)
    {
    case Cross::LEFT: os << "Left"; break;
    case Cross::DIRECT: os << "Direct"; break;
    case Cross::RIGHT: os << "Right"; break;
    default: ASSERT(false); break;
    }
    return os;
}

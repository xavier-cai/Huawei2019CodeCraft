#include "cross.h"
#include "road.h"
#include "assert.h"

Cross::Cross()
{
    ASSERT(false);
}

Cross::Cross(int id, int northRoadId, int eastRoadId, int southRoadId, int westRoadId)
    : m_id(id), m_northRoadId(northRoadId), m_eastRoadId(eastRoadId), m_southRoadId(southRoadId), m_westRoadId(westRoadId)
    , m_north(0), m_east(0), m_south(0), m_west(0)
{ }

Cross::~Cross()
{ }

int Cross::GetId() const
{
    return m_id;
}

int Cross::GetNorthRoadId() const
{
    return m_northRoadId;
}

int Cross::GetEasthRoadId() const
{
    return m_eastRoadId;
}

int Cross::GetSouthhRoadId() const
{
    return m_southRoadId;
}

int Cross::GetWestRoadId() const
{
    return m_westRoadId;
}

int Cross::GetRoadId(const Cross::DirectionType& dir) const
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

#ifndef CROSS_H
#define CROSS_H

#include "define.h"
#include <map>

class Road;

class Cross
{
public:
    enum DirectionType
    {
        NORTH,
        EAST,
        SOUTH,
        WEST
    };

#define DirectionType_Size 4
#define DirectionType_Foreach(var, content) \
    FOREACH(Cross::DirectionType, Cross::NORTH, Cross::WEST, var, content)

    enum TurnType
    {
        LEFT,
        DIRECT,
        RIGHT
    };

private:
    int m_originId;
    int m_id;
    int m_northRoadId;
    int m_eastRoadId;
    int m_southRoadId;
    int m_westRoadId;
    
    Road* m_north;
    Road* m_east;
    Road* m_south;
    Road* m_west;

    std::map<int, DirectionType> m_directions;
    
public:
    Cross();
    Cross(const int& origin, const int& id, const int& northRoadId, const int& eastRoadId, const int& southRoadId, const int& westRoadId);
    ~Cross();
    
    inline const int& GetOriginId() const;
    inline const int& GetId() const;
    inline const int& GetNorthRoadId() const;
    inline const int& GetEasthRoadId() const;
    inline const int& GetSouthRoadId() const;
    inline const int& GetWestRoadId() const;
    inline int GetRoadId(const DirectionType& dir) const;

    const DirectionType& GetDirection(const int& id) const;
    TurnType GetTurnDirection(const int& from, const int& to) const;
    DirectionType GetTurnDestinationDirection(const int& from, const TurnType& turn) const;
    Road* GetTurnDestination(const int& from, const TurnType& turn) const;
    int GetTurnDestinationId(const int& from, const TurnType& turn) const;
    
    void SetNorthRoadId(const int& id);
    void SetEasthRoadId(const int& id);
    void SetSouthRoadId(const int& id);
    void SetWestRoadId(const int& id);
    void SetNorthRoad(Road* road);
    void SetEasthRoad(Road* road);
    void SetSouthRoad(Road* road);
    void SetWestRoad(Road* road);
    inline Road* GetNorthRoad() const;
    inline Road* GetEasthRoad() const;
    inline Road* GetSouthRoad() const;
    inline Road* GetWestRoad() const;
    inline Road* GetRoad(const DirectionType& dir) const;
    
};//class Cross

Cross::TurnType operator ! (const Cross::TurnType& t);
std::ostream& operator << (std::ostream& os, const Cross::TurnType& turn);





/* 
 * [inline functions]
 *   it's not good to write code here, but we really need inline!
 */

#include "assert.h"

inline const int& Cross::GetOriginId() const
{
    return m_originId;
}

inline const int& Cross::GetId() const
{
    return m_id;
}

inline const int& Cross::GetNorthRoadId() const
{
    return m_northRoadId;
}

inline const int& Cross::GetEasthRoadId() const
{
    return m_eastRoadId;
}

inline const int& Cross::GetSouthRoadId() const
{
    return m_southRoadId;
}

inline const int& Cross::GetWestRoadId() const
{
    return m_westRoadId;
}

inline int Cross::GetRoadId(const DirectionType& dir) const
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

inline Road* Cross::GetNorthRoad() const
{
    return m_north;
}

inline Road* Cross::GetEasthRoad() const
{
    return m_east;
}

inline Road* Cross::GetSouthRoad() const
{
    return m_south;
}

inline Road* Cross::GetWestRoad() const
{
    return m_west;
}

inline Road* Cross::GetRoad(const Cross::DirectionType& dir) const
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

inline Cross::TurnType operator ! (const Cross::TurnType& t)
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

inline std::ostream& operator << (std::ostream& os, const Cross::TurnType& turn)
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

#endif

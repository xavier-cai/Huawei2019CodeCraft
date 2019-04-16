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
    
    const int& GetOriginId() const;
    const int& GetId() const;
    const int& GetNorthRoadId() const;
    const int& GetEasthRoadId() const;
    const int& GetSouthRoadId() const;
    const int& GetWestRoadId() const;
    int GetRoadId(const DirectionType& dir) const;
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
    Road* GetNorthRoad() const;
    Road* GetEasthRoad() const;
    Road* GetSouthRoad() const;
    Road* GetWestRoad() const;
    Road* GetRoad(const DirectionType& dir) const;
    
};//class Cross

Cross::TurnType operator ! (const Cross::TurnType& t);
std::ostream& operator << (std::ostream& os, const Cross::TurnType& turn);

#endif

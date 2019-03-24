#ifndef CROSS_H
#define CROSS_H

#include "define.h"

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
    
private:
    int m_id;
    int m_northRoadId;
    int m_eastRoadId;
    int m_southRoadId;
    int m_westRoadId;
    
    Road* m_north;
    Road* m_east;
    Road* m_south;
    Road* m_west;
    
public:
    Cross();
    Cross(int id, int northRoadId, int eastRoadId, int southRoadId, int westRoadId);
    ~Cross();
    
    int GetId() const;
    int GetNorthRoadId() const;
    int GetEasthRoadId() const;
    int GetSouthhRoadId() const;
    int GetWestRoadId() const;
    int GetRoadId(const DirectionType& dir) const;
    
    void SetNorthRoad(Road* road);
    void SetEasthRoad(Road* road);
    void SetSouthhRoad(Road* road);
    void SetWestRoad(Road* road);
    Road* GetNorthRoad() const;
    Road* GetEasthRoad() const;
    Road* GetSouthhRoad() const;
    Road* GetWestRoad() const;
    Road* GetRoad(const DirectionType& dir) const;
    
};//class Cross

#endif

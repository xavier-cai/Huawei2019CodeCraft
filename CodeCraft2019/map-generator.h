#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H

#include <map>
#include "scenario.h"

class SimCar;

class MapGenerator
{
public:
    MapGenerator();
    void Generate(int argc, char *argv[]);

    void SetWidth(int width);
    void SetHeight(int height);
    void SetCarsN(int n);
    void SetSingleWayProbability(double prob);
    void SetNoWayProbability(double prob);
    void SetVipProbability(double prob);
    void SetPresetProbability(double prob);
    void SetPresetMaxRealTime(int time);

private:
    void GenerateMap();
    bool CheckPathValid() const;
    void SaveToFile() const;

    int m_width;
    int m_height;
    int m_carsN;
    double m_singleWayProb;
    double m_noWayProb;
    double m_vipProb;
    double m_presetProb;
    int m_presetMaxRealTime;

    std::map<int, Cross> m_crosses;
    std::map<int, Road> m_roads;
    std::map<int, Car> m_cars;
    std::map<int, SimCar*> m_preset;

};//class MapGenerator

#endif
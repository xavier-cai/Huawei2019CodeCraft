#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config
{
private:
    Config();
    virtual ~Config() = 0;
    
public:
    static void Initialize(int argc, char* argv[]);
    static std::string PathCar;
    static std::string PathCross;
    static std::string PathRoad;
    static std::string PathPreset;
    static std::string PathResult;
    static const double TimeLimit;
    
};//class Config

#endif

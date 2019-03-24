#include "config.h"
#include "assert.h"

std::string Config::PathCar;
std::string Config::PathCross;
std::string Config::PathRoad;
std::string Config::PathResult;
const double Config::TimeLimit(300);

Config::Config()
{
    ASSERT(false);
}

void Config::Initialize(int argc, char* argv[])
{
    ASSERT_MSG(argc >= 5, "argc=" << argc);
    PathCar = argv[1];
    PathRoad = argv[2];
    PathCross = argv[3];
    PathResult = argv[4];
}


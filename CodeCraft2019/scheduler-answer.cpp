#include "scheduler-answer.h"
#include "file-reader.h"
#include "assert.h"
#include "config.h"

void SchedulerAnswer::DoInitialize(SimScenario& scenario)
{
    m_scenario = &scenario;
    FileReader reader;
    ASSERT(reader.Read(Config::PathResult.c_str(), &SchedulerAnswer::HandleAnswer, this));
}

bool SchedulerAnswer::HandleAnswer(std::istream& is)
{
    char c;
    is >> c;
    if (c == '#')
        return true;
    if (c != '(')
        ASSERT(false);
    int argv[2];
    for (int i = 0; i < 2; i++)
    {
        argv[i] = -1;
        is >> argv[i] >> c;
        ASSERT_MSG(c == ',', "char=" << c << " position=" << i);
    }
    ASSERT(argv[0] >= 0 && argv[1] >= 0);
    SimCar* car = &m_scenario->Cars()[argv[0]];
    ASSERT(car != 0);
    car->SetRealTime(argv[1]);
    int path;
    while (true)
    {
        path = -1;
        is >> path >> c;
        ASSERT(c == ',' || c == ')');
        ASSERT(path >= 0);
        car->GetTrace().AddToTail(path);
        if (c == ')')
            break;
    }
    return true;
}
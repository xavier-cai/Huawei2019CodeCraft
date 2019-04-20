#include "scheduler-answer.h"
#include "file-reader.h"
#include "assert.h"
#include "config.h"
#include "log.h"
#include "simulator.h"

void SchedulerAnswer::DoInitialize(SimScenario& scenario)
{
    Simulator::SetEnableCheater(false);
    m_scenario = &scenario;
    LOG("read information of answer from " << Config::PathResult);
    FileReader reader;
    bool result = reader.Read(Config::PathResult.c_str(), Callback::Create(&SchedulerAnswer::HandleAnswer, this));
    ASSERT(result);
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
    for (int i = 0; i < 2; ++i)
    {
        argv[i] = -1;
        is >> argv[i] >> c;
        ASSERT_MSG(c == ',', "char=" << c << " position=" << i);
    }
    ASSERT(argv[0] >= 0 && argv[1] >= 0);
    int id = Scenario::MapCarOriginToIndex(argv[0]);
    SimCar* car = m_scenario->Cars()[id];

    ASSERT(car != 0);
    if (car->GetCar()->GetIsPreset())
    {
        car->SetIsForceOutput(true);
        car->GetTrace().Clear();
    }
    else
    {
        car->SetRealTime(argv[1]);
    }
    int path;
    while (true)
    {
        path = -1;
        is >> path >> c;
        ASSERT(c == ',' || c == ')');
        ASSERT(path >= 0);
        car->GetTrace().AddToTail(Scenario::MapRoadOriginToIndex(path));
        if (c == ')')
            break;
    }

    return true;
}

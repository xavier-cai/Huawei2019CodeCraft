#include "scheduler-answer.h"
#include "file-reader.h"
#include "assert.h"
#include "config.h"

void SchedulerAnswer::DoInitialize(SimScenario& scenario)
{
    m_scenario = &scenario;
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
    SimCar* car = &m_scenario->Cars()[argv[0]];
    ASSERT(car != 0);
    if (!car->GetCar()->GetIsPreset())
    {
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
    }
    return true;
}

SchedulerAnswer::SchedulerAnswer(const std::string& traceFile)
    : m_traceFile(0)
{
    if (traceFile.length() > 0)
        m_traceFile = new std::ofstream(traceFile.c_str());
}

SchedulerAnswer::~SchedulerAnswer()
{
    if (m_traceFile != 0)
    {
        m_traceFile->flush();
        m_traceFile->close();
        delete m_traceFile;
    }
}

void SchedulerAnswer::DoHandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result)
{
    if (m_traceFile != 0)
    {
        std::ostream& os = *m_traceFile;
        os << "time:" << time << "\n";
        for (auto roadIte = scenario.Roads().begin(); roadIte != scenario.Roads().end(); ++roadIte)
        {
            os << "(" << roadIte->first << ",";
            os << "forward,[";
            int length = roadIte->second.GetRoad()->GetLength();
            for (int i = 1; i <= roadIte->second.GetRoad()->GetLanes(); ++i)
            {
                os << (i == 1 ? "" : ",") << "[";
                auto& cars = roadIte->second.GetCars(i);
                int index = length;
                for (auto carIte = cars.begin(); carIte != cars.end(); ++carIte)
                {
                    int pos = scenario.Cars()[(*carIte)->GetId()].GetCurrentPosition();
                    for (; index > pos; --index)
                        os << (index == length ? "" : ",") << "-1";
                    os << (index == length ? "" : ",") << (*carIte)->GetId();
                    --index;
                }
                for (; index > 0; --index)
                    os << (index == length ? "" : ",") << "-1";
                os << "]";
            }
            os << "])";
            if (roadIte->second.GetRoad()->GetIsTwoWay())
            {
                os << "\n(" << roadIte->first << ",";
                os << "backward,[";
                for (int i = 1; i <= roadIte->second.GetRoad()->GetLanes(); ++i)
                {
                    os << (i == 1 ? "" : ",") << "[";
                    auto& cars = roadIte->second.GetCarsOpposite(i);
                    int index = length;
                    for (auto carIte = cars.begin(); carIte != cars.end(); ++carIte)
                    {
                        int pos = scenario.Cars()[(*carIte)->GetId()].GetCurrentPosition();
                        for (; index > pos; --index)
                            os << (index == length ? "" : ",") << "-1";
                        os << (index == length ? "" : ",") << (*carIte)->GetId();
                        --index;
                    }
                    for (; index > 0; --index)
                        os << (index == length ? "" : ",") << "-1";
                    os << "]";
                }
                os << "])";
            }
            os << std::endl;
        }
    }
}
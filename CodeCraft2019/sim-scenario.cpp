#include "sim-scenario.h"
#include "scenario.h"
#include "assert.h"
#include "config.h"
#include <fstream>

SimScenario::SimScenario()
    : m_reachCarsN(0), m_carInGarageN(Scenario::Cars().size())
    , m_scheduledTime(-1), m_totalCompleteTime(0), m_vipFirstPlanTime(-1), m_vipLastReachTime(-1), m_vipTotalCompleteTime(0)
{
    //create roads
    for (auto ite = Scenario::Roads().begin(); ite != Scenario::Roads().end(); ++ite)
    {
        bool result = m_simRoads.insert(std::make_pair(ite->first, SimRoad(ite->second))).second;
        ASSERT(result);
    }
    //create garages & cars
    for (auto ite = Scenario::Cars().begin(); ite != Scenario::Cars().end(); ++ite)
    {
        auto result = m_simCars.insert(std::make_pair(ite->first, SimCar(ite->second)));
        result.first->second.SetScenario(this);
        ASSERT(result.second);
        m_simGarages[ite->second->GetFromCrossId()].push_back(&m_simCars[ite->first]);
    }
}

SimScenario::~SimScenario()
{ }

SimScenario::SimScenario(const SimScenario& o)
{
    *this = o;
}

SimScenario& SimScenario::operator = (const SimScenario& o)
{
    m_simGarages = o.m_simGarages;
    m_simRoads = o.m_simRoads;
    m_simCars = o.m_simCars;
    for (auto ite = m_simGarages.begin(); ite != m_simGarages.end(); ++ite)
    {
        for (auto ii = ite->second.begin(); ii != ite->second.end(); ++ii)
        {
            *ii = &m_simCars[(*ii)->GetCar()->GetId()];
            (*ii)->SetScenario(this);
        }
    }
    m_reachCarsN = o.m_reachCarsN;
    m_carInGarageN = o.m_carInGarageN;
    m_scheduledTime = o.m_scheduledTime;
    m_totalCompleteTime = o.m_totalCompleteTime;
    m_vipFirstPlanTime = o.m_vipFirstPlanTime;
    m_vipLastReachTime = o.m_vipLastReachTime;
    m_vipTotalCompleteTime = o.m_vipTotalCompleteTime;
    return *this;
}

const int& SimScenario::GetScheduledTime() const
{
    return m_scheduledTime;
}

const int& SimScenario::GetTotalCompleteTime() const
{
    return m_totalCompleteTime;
}

int SimScenario::GetVipScheduledTime() const
{
    return m_vipLastReachTime - m_vipFirstPlanTime;
}

const int& SimScenario::GetVipTotalCompleteTime() const
{
    return m_vipTotalCompleteTime;
}

std::map< int, std::list<SimCar*> >& SimScenario::Garages()
{
    return m_simGarages;
}

std::map<int, SimRoad>& SimScenario::Roads()
{
    return m_simRoads;
}

std::map<int, SimCar>& SimScenario::Cars()
{
    return m_simCars;
}

void SimScenario::NotifyCarGetoutOnRoad(const int& time, const SimCar* car)
{
    --m_carInGarageN;
}

void SimScenario::NotifyCarReachGoal(const int& time, const SimCar* car)
{
    ++m_reachCarsN;
    int cost = time - car->GetCar()->GetPlanTime();
    ASSERT(cost >= 0);
    if (car->GetCar()->GetIsVip())
    {
        m_vipLastReachTime = time;
        if (m_vipFirstPlanTime < 0 || m_vipFirstPlanTime > car->GetCar()->GetPlanTime())
            m_vipFirstPlanTime = car->GetCar()->GetPlanTime();
        m_vipTotalCompleteTime += cost;
    }
    m_scheduledTime = time;
    m_totalCompleteTime += cost;
}

void SimScenario::SaveToFile() const
{
    SaveToFile(Config::PathResult.c_str());
}

void SimScenario::SaveToFile(const char* file) const
{
    std::ofstream ofs(file);
    ASSERT(ofs.is_open());
    for (auto ite = m_simCars.begin(); ite != m_simCars.end(); ++ite)
    {
        const SimCar* car = &ite->second;
        if (!car->GetCar()->GetIsPreset())
        {
            ofs << '(' << ite->first << ", " << car->GetRealTime();
            ASSERT(car->GetTrace().Head() != car->GetTrace().Tail());
            for (auto traceIte = car->GetTrace().Head(); traceIte != car->GetTrace().Tail(); ++traceIte)
            {
                ofs << ", " << *traceIte;
            }
            ofs << ")\n";
        }
    }
    ofs.clear();
    ofs.flush();
}

bool SimScenario::IsComplete() const
{
    return m_reachCarsN == Scenario::Cars().size();
}

int SimScenario::GetOnRoadCarsN() const
{
    return Scenario::Cars().size() - (m_reachCarsN + m_carInGarageN);
}

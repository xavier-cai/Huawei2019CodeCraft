#include "sim-scenario.h"
#include "scenario.h"
#include "assert.h"
#include "config.h"
#include <fstream>

SimScenario::SimScenario(bool onlyPreset)
    : m_simGarages(Scenario::CalculateIndexArraySize(Scenario::Crosses()), &Scenario::GetCrossIndexer())
    , m_simRoads(Scenario::CalculateIndexArraySize(Scenario::Roads()), Scenario::Roads(), &Scenario::GetRoadIndexer())
    , m_simCars(Scenario::CalculateIndexArraySize(Scenario::Cars()), Scenario::Cars(), &Scenario::GetCarIndexer())
    , m_reachCarsN(0), m_carInGarageN(0)
    , m_scheduledTime(-1), m_totalCompleteTime(0), m_vipFirstPlanTime(-1), m_vipLastReachTime(-1), m_vipTotalCompleteTime(0)
{
    for (auto ite = Scenario::Crosses().begin(); ite != Scenario::Crosses().end(); ++ite)
    {
        m_simGarages.insert(ite->first, std::map<int, SimCar*>());
    }
    for (auto ite = m_simCars.begin(); ite != m_simCars.end(); )
    {
        if (!onlyPreset || ite->second.GetCar()->GetIsPreset())
        {
            ite->second.SetScenario(this);
            m_simGarages[ite->second.GetCar()->GetFromCrossId()].insert(std::make_pair(ite->second.GetCar()->GetId(), &ite->second));
            ++ite;
        }
        else
        {
            ite = m_simCars.erase(ite);
        }
    }
    m_carInGarageN = m_simCars.size();
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
            ii->second = &m_simCars[ii->first];
            ii->second->SetScenario(this);
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

QuickMap< int, std::map<int, SimCar*> >& SimScenario::Garages()
{
    return m_simGarages;
}

QuickMap<int, SimRoad>& SimScenario::Roads()
{
    return m_simRoads;
}

QuickMap<int, SimCar>& SimScenario::Cars()
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
        if (!car->GetCar()->GetIsPreset() || car->GetIsForceOutput())
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
    ofs.flush();
    ofs.close();
}

bool SimScenario::IsComplete() const
{
    return m_reachCarsN == m_simCars.size();
}

const unsigned int& SimScenario::GetCarInGarageN() const
{
    return m_carInGarageN;
}

const unsigned int& SimScenario::GetReachCarsN() const
{
    return m_reachCarsN;
}

int SimScenario::GetOnRoadCarsN() const
{
    return m_simCars.size() - (m_reachCarsN + m_carInGarageN);
}

void SimScenario::ResetScenario()
{
    m_simGarages.clear();
    for (auto ite = m_simCars.begin(); ite != m_simCars.end(); ++ite)
    {
        ite->second.Reset();
        m_simGarages[ite->second.GetCar()->GetFromCrossId()].insert(std::make_pair(ite->second.GetCar()->GetId(), &ite->second));
    }
    for (auto ite = m_simRoads.begin(); ite != m_simRoads.end(); ++ite)
    {
        ite->second.Reset();
    }
    m_reachCarsN = 0;
    m_carInGarageN = m_simCars.size();
    m_scheduledTime = -1;
    m_totalCompleteTime = 0;
    m_vipFirstPlanTime = -1;
    m_vipLastReachTime = -1;
    m_vipTotalCompleteTime = 0;
}

SimCar* SimScenario::AddCar(Car* car)
{
    ASSERT(m_simCars.find(car->GetId()) == m_simCars.end());
    auto result = m_simCars.insert(car->GetId(), SimCar(car));
    ASSERT(result.second);
    if (m_simGarages.find(car->GetFromCrossId()) == m_simGarages.end())
        m_simGarages.insert(car->GetFromCrossId(), std::map<int, SimCar*>());
    auto& garage = m_simGarages[car->GetFromCrossId()];
    ASSERT(garage.find(car->GetId()) == garage.end());
    auto gResult = garage.insert(std::make_pair(car->GetId(), &(result.first->second)));
    ASSERT(gResult.second);
    return gResult.first->second;
}

void SimScenario::RemoveCar(const int& id)
{
    auto find = m_simCars.find(id);
    ASSERT(find != m_simCars.end());
    ASSERT(find->second.GetIsInGarage());
    m_simGarages[find->second.GetCar()->GetFromCrossId()].erase(id);
    m_simCars.erase(find);
}

#include "sim-scenario.h"
#include "scenario.h"
#include "assert.h"
#include "config.h"
#include <fstream>

SimScenario::SimScenario()
    : m_reachCarsN(0), m_carInGarageN(Scenario::Cars().size())
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
        bool result = m_simCars.insert(std::make_pair(ite->first, SimCar(ite->second))).second;
        ASSERT(result);
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
        for (auto ii = ite->second.begin(); ii != ite->second.end(); ++ii)
            *ii = &m_simCars[(*ii)->GetCar()->GetId()];
    m_reachCarsN = o.m_reachCarsN;
    m_carInGarageN = o.m_carInGarageN;
    return *this;
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

void SimScenario::ReachGoal(unsigned int n)
{
    m_reachCarsN += n;
}

void SimScenario::GetoutOnRoad(unsigned int n)
{
    m_carInGarageN -= n;
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
        ofs << '(' << ite->first << ", " << car->GetRealTime();
        ASSERT(car->GetTrace().Head() != car->GetTrace().Tail());
        for (auto traceIte = car->GetTrace().Head(); traceIte != car->GetTrace().Tail(); ++traceIte)
        {
            ofs << ", " << *traceIte;
        }
        ofs << ")\n";
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

#include "sim-scenario.h"
#include "assert.h"
#include "config.h"
#include <fstream>

SimScenario::SimScenario(bool onlyPreset)
    : m_reachCarsN(0), m_carOnRoadN(0), m_carInGarageN(0)
    , m_scheduledTime(-1), m_totalCompleteTime(0), m_vipFirstPlanTime(-1), m_vipLastReachTime(-1), m_vipTotalCompleteTime(0)
{
    m_simGarages.resize(Scenario::Crosses().size());
    for (uint i = 0; i < m_simGarages.size(); ++i)
    {
        m_simGarages[i].resize(Scenario::GetGarageSize(i), 0);
    }
    m_simRoads.resize(Scenario::Roads().size(), 0);
    m_simCars.resize(Scenario::Cars().size(), 0);
    for (uint i = 0; i < m_simCars.size(); ++i)
    {
        Car* car = Scenario::Cars()[i];
        if (!onlyPreset || car->GetIsPreset())
        {
            SimCar* simCar = new SimCar(car);
            simCar->SetScenario(this);
            m_simCars[i] = simCar;
            m_simGarages[simCar->GetCar()->GetFromCrossId()][Scenario::GetGarageInnerIndex(i)] = simCar;
            ++m_carInGarageN;
        }
    }
    for (uint i = 0; i < m_simRoads.size(); ++i)
    {
        m_simRoads[i] = new SimRoad(Scenario::Roads()[i]);
    }
}

void SimScenario::Clear()
{
    for (uint i = 0; i < m_simCars.size(); ++i)
    {
        if (m_simCars[i] != 0)
            delete m_simCars[i];
    }
    m_simCars.clear();
    for (uint i = 0; i < m_simRoads.size(); ++i)
    {
        if (m_simRoads[i] != 0)
            delete m_simRoads[i];
    }
}

SimScenario::~SimScenario()
{
    Clear();
}

SimScenario::SimScenario(const SimScenario& o)
{
    *this = o;
}

SimScenario& SimScenario::operator = (const SimScenario& o)
{
    Clear();
    m_simGarages.resize(o.m_simGarages.size());
    m_simRoads.resize(o.m_simRoads.size(), 0);
    m_simCars.resize(o.m_simCars.size(), 0);
    for (uint i = 0; i < m_simGarages.size(); ++i)
    {
        m_simGarages[i].resize(Scenario::GetGarageSize(i), 0);
    }
    for (uint i = 0; i < m_simCars.size(); ++i)
    {
        if (o.m_simCars[i] != 0)
        {
            m_simCars[i] = new SimCar(*o.m_simCars[i]);
            m_simCars[i]->SetScenario(this);
            m_simGarages[m_simCars[i]->GetCar()->GetFromCrossId()][Scenario::GetGarageInnerIndex(i)] = m_simCars[i];
        }
    }
    for (uint i = 0; i < m_simRoads.size(); ++i)
    {
        if (o.m_simRoads[i] != 0)
        {
            m_simRoads[i] = new SimRoad(*o.m_simRoads[i]);
        }
    }
    m_reachCarsN = o.m_reachCarsN;
    m_carOnRoadN = o.m_carOnRoadN;
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

void SimScenario::NotifyCarGetoutOnRoad(const int& time, const SimCar* car)
{
    ASSERT(m_carInGarageN > 0);
    --m_carInGarageN;
    ++m_carOnRoadN;
}

void SimScenario::NotifyCarReachGoal(const int& time, const SimCar* car)
{
    --m_carOnRoadN;
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
    for (uint i = 0; i < m_simCars.size(); ++i)
    {
        const SimCar* car = m_simCars[i];
        if (!car->GetCar()->GetIsPreset() || car->GetIsForceOutput())
        {
            ofs << '(' << car->GetCar()->GetOriginId() << ", " << car->GetRealTime();
            ASSERT(car->GetTrace().Head() != car->GetTrace().Tail());
            for (uint i = 0; i < car->GetTrace().Size(); ++i)
            {
                ofs << ", " << Scenario::Roads()[car->GetTrace()[i]]->GetOriginId();
            }
            ofs << ")\n";
        }
    }
    ofs.flush();
    ofs.close();
}

bool SimScenario::IsComplete() const
{
    return m_carInGarageN == 0 && m_carOnRoadN == 0;
}

SimCar* SimScenario::AddCar(Car* car)
{
    ASSERT(m_reachCarsN == 0 && m_carOnRoadN == 0);
    ASSERT(m_simCars[car->GetId()] == 0);
    SimCar* simCar = new SimCar(car);
    m_simCars[car->GetId()] = simCar;
    SimCar*& carInGarage = m_simGarages[car->GetFromCrossId()][Scenario::GetGarageInnerIndex(car->GetId())];
    ASSERT(carInGarage == 0);
    carInGarage = simCar;
    return simCar;
}

void SimScenario::RemoveCar(SimCar* car)
{
    ASSERT(m_reachCarsN == 0 && m_carOnRoadN == 0);
    SimCar*& carInGarage = m_simGarages[car->GetCar()->GetFromCrossId()][Scenario::GetGarageInnerIndex(car->GetCar()->GetId())];
    SimCar*& carInVector = m_simCars[car->GetCar()->GetId()];
    ASSERT(carInGarage != 0);
    ASSERT(carInVector != 0);
    ASSERT(carInGarage == carInVector);
    delete carInGarage;
    carInGarage = 0;
    carInVector = 0;
}

/*
void SimScenario::ReplaceGarage(const std::vector< std::vector<SimCar*> >& garage)
{
    ASSERT(m_reachCarsN == 0 && m_carOnRoadN == 0);
    m_simGarages = garage;
}
*/

void SimScenario::ResetScenario()
{
    for (uint i = 0; i < m_simGarages.size(); ++i)
    {
        m_simGarages[i].clear();
    }
    m_carInGarageN = 0;
    for (uint i = 0; i < m_simCars.size(); ++i)
    {
        SimCar* car = m_simCars[i];
        if (car != 0)
        {
            car->Reset();
            m_simGarages[car->GetCar()->GetFromCrossId()].push_back(car);
            ++m_carInGarageN;
        }
    }
    for (uint i = 0; i < m_simRoads.size(); ++i)
    {
        if (m_simRoads[i] != 0)
            m_simRoads[i]->Reset();
    }
    m_reachCarsN = 0;
    m_scheduledTime = -1;
    m_totalCompleteTime = 0;
    m_vipFirstPlanTime = -1;
    m_vipLastReachTime = -1;
    m_vipTotalCompleteTime = 0;
}
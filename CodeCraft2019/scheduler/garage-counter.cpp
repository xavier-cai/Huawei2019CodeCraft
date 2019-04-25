#include "garage-counter.h"
#include <algorithm>
#include "simulator.h"

void GarageCounter::Initialize(SimScenario& scenario)
{
    m_dispatchCarsNInEachTime.resize(scenario.Garages().size(), 0);
    m_leftCarsN.resize(scenario.Garages().size(), 0);
    m_carsN.resize(scenario.Garages().size(), 0);
    int totalCarsN = scenario.GetCarInGarageN();
    for (uint i = 0; i < scenario.Garages().size(); ++i)
    {
        m_leftCarsN[i] = 0;
        for (uint j = 0; j < scenario.Garages()[i].size(); ++j)
        {
            SimCar* car = scenario.Garages()[i][j];
            if (car != 0 && car->GetIsInGarage())
            {
                ++m_leftCarsN[i];
            }
        }
        m_dispatchCarsNInEachTime[i] = m_leftCarsN[i] * scenario.Garages().size() * 1.0 / totalCarsN;
        m_carsN[i] = m_leftCarsN[i];
    }

    m_bestCars.resize(scenario.Garages().size());
}

int StaticTime = 0;

bool CompareCar(SimCar* a, SimCar* b)
{
    if (a->GetCar()->GetIsVip() != b->GetCar()->GetIsVip())
        return a->GetCar()->GetIsVip();
    if (a->CalculateSpendTime(StaticTime) != b->CalculateSpendTime(StaticTime))
        return (a->CalculateSpendTime(StaticTime) > b->CalculateSpendTime(StaticTime));
    if (a->GetCar()->GetMaxSpeed() != b->GetCar()->GetMaxSpeed())
        return a->GetCar()->GetMaxSpeed() < b->GetCar()->GetMaxSpeed();
    return a->GetCar()->GetOriginId() < b->GetCar()->GetOriginId();
}

void GarageCounter::Update(const int& time, SimScenario& scenario)
{
    m_isScheduling = true;
    StaticTime = time;
    for (uint i = 0; i < scenario.Garages().size(); ++i)
    {
        m_leftCarsN[i] = 0;
        m_bestCars[i].clear();
        for (uint j = 0; j < scenario.Garages()[i].size(); ++j)
        {
            SimCar* car = scenario.Garages()[i][j];
            if (car != 0 && car->GetIsInGarage())
            {
                ++m_leftCarsN[i];
                if (car->GetRealTime() <= time)
                    m_bestCars[i].push_back(car);
            }
        }
        m_bestCars[i].sort(&CompareCar);
    }
}

void GarageCounter::HandleBeforeGarageDispatch(const int& time, SimScenario& scenario)
{
    m_isScheduling = false;
    return;
    for (uint i = 0; i < scenario.Garages().size(); ++i)
    {
        m_bestCars[i].clear();
        int countLimit = (time * m_dispatchCarsNInEachTime[i] + m_leftCarsN[i]) - m_carsN[i];
        if (countLimit > 0)
        {
            for (uint j = 0; j < scenario.Garages()[i].size(); ++j)
            {
                SimCar* car = scenario.Garages()[i][j];
                if (car != 0 && car->GetIsInGarage() && car->GetRealTime() <= time && Simulator::Instance.CanCarGetOutFromGarage(time, scenario, car).first > 0)
                {
                    auto value = car->CalculateSpendTime(time);
                    if (m_bestCars[i].size() == 0)
                        m_bestCars[i].push_back(car);
                    else
                    {
                        for (auto ite = --m_bestCars[i].end(); ; --ite)
                        {
                            //if (CompareCar(time, *ite, car))
                            {
                                m_bestCars[i].insert(++ite, car);
                                break;
                            }
                            if (ite == m_bestCars[i].begin())
                            {
                                m_bestCars[i].insert(ite, car);
                                break;
                            }
                        }
                        while (m_bestCars[i].size() > countLimit)
                            m_bestCars[i].pop_back();
                    }
                }
            }
        }
    }
}

bool GarageCounter::CanDispatch(const int& time, SimScenario& scenario, SimCar* car) const
{
    int garageId = car->GetCar()->GetFromCrossId();
    int cangoCarsN = GetCangoCarsN(time, garageId);
    if (cangoCarsN <= 0)
    {
        return false;
    }

    ASSERT(m_bestCars[garageId].size() > 0);
    SimCar* front = m_bestCars[garageId].front();
    if (m_isScheduling)
    {
        ASSERT(car->GetCar()->GetIsVip());
        //return car->GetCar()->GetMaxSpeed() == m_smallestSpeed[garageId] && car->CalculateSpendTime(time) >= m_longestSpendTime[garageId] * 0.75;
        return car->GetCar()->GetIsVip() == front->GetCar()->GetIsVip()
            && car->GetCar()->GetMaxSpeed() <= front->GetCar()->GetMaxSpeed() *1.25
            && car->CalculateSpendTime(time) >= front->CalculateSpendTime(time) * 0.75;
    }

    int index = 0;
    for (auto ite = m_bestCars[garageId].begin(); index < cangoCarsN && ite != m_bestCars[garageId].end(); ++ite, ++index)
        if (*ite == car)
            return true;
    return false;
}

void GarageCounter::NotifyDispatch(const int& garageId)
{
    --m_leftCarsN[garageId];
}

int GarageCounter::GetCangoCarsN(const int& time, const int& garageId) const
{
    return (time * m_dispatchCarsNInEachTime[garageId] + m_leftCarsN[garageId]) - m_carsN[garageId];
}

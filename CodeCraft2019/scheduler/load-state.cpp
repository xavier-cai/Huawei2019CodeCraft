#include "load-state.h"
#include "assert.h"
#include "log.h"
#include "scenario.h"

LoadState::TimeSlice::TimeSlice()
    : Time(-1), Load(0)
{ }

LoadState::TimeSlice::TimeSlice(const int& time, const int& load)
    : Time(time), Load(load)
{ }

void LoadState::Update(const int& time, SimScenario& scenario)
{
    if (time <= 0)
        return ;
    if (m_timeLoad.size() > 0)
        ASSERT(m_timeLoad.back().Time == time - 1);
    m_timeLoad.push_back(TimeSlice(time, scenario.GetOnRoadCarsN()));
    for (uint i = 0; i < scenario.Roads().size(); ++i)
    {
        SimRoad* road = scenario.Roads()[i];
        int lanes = road->GetRoad()->GetLanes();
        int load = 0;
        int loadOpposite = 0;
        for (int i = 0; i < lanes * 2; ++i)
        {
            bool opposite = i >= lanes;
            if (opposite && !road->GetRoad()->GetIsTwoWay())
                break;
            (opposite ? loadOpposite : load) += road->GetCars((i % lanes) + 1, opposite).size();
        }
        m_spaceLoad[std::make_pair(road->GetRoad()->GetId(), false)] += load;
        m_spaceLoad[std::make_pair(road->GetRoad()->GetId(), true)] += loadOpposite;
    }
}

void LoadState::Print(SimScenario& scenario) const
{
    ASSERT(m_timeLoad.size() > 0);
    const int& time = m_timeLoad.back().Time;
    LOG ("Space load : ");
    int capacityTotal = 0;
    for (uint i = 0; i < scenario.Roads().size(); ++i)
    {
        SimRoad* road = scenario.Roads()[i];
        int lanes = road->GetRoad()->GetLanes();
        int capacity = lanes * road->GetRoad()->GetLength();
        capacityTotal += road->GetRoad()->GetIsTwoWay() ? capacity * 2 : capacity;
        { //print
            auto find0 = m_spaceLoad.find(std::make_pair(road->GetRoad()->GetId(), false));
            ASSERT(find0 != m_spaceLoad.end());
            LOG ("road " << road->GetRoad()->GetId()
                << " from " << road->GetRoad()->GetStartCrossId()
                << " to " << road->GetRoad()->GetEndCrossId()
                << " lanes " << lanes
                << " capacity " << capacity
                << " load " << (find0->second * 1.0 / time / capacity)
                );
        }
        if (!road->GetRoad()->GetIsTwoWay())
            continue;
        { //print opposite
            auto find1 = m_spaceLoad.find(std::make_pair(road->GetRoad()->GetId(), true));
            ASSERT(find1 != m_spaceLoad.end());
            LOG ("road " << road->GetRoad()->GetId()
                << " from " << road->GetRoad()->GetEndCrossId()
                << " to " << road->GetRoad()->GetStartCrossId()
                << " lanes " << lanes
                << " capacity " << capacity
                << " load " << (find1->second * 1.0 / time / capacity)
                );
        }
    }
    LOG ("Time load (" << time << ") : ");
    for (auto ite = m_timeLoad.begin(); ite != m_timeLoad.end(); ite++)
    {
        LOG ("time " << ite->Time
            << " load " << (ite->Load * 1.0 / capacityTotal)
            );
    }
}
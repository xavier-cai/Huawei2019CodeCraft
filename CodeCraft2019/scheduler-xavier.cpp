#include "scheduler-xavier.h"
#include "scenario.h"

void SchedulerXavier::DoInitialize(SimScenario& scenario)
{
    int size = Scenario::Crosses().size();
    m_state = m_memoryPool.NewArray<bool**>(size);
    m_cost = m_memoryPool.NewArray<double**>(size);
    m_path = m_memoryPool.NewArray<std::list<int>**>(size);
    for (int i = 0; i < size; ++i)
    {
        m_state[i] = m_memoryPool.NewArray<bool*>(size);
        m_cost[i] = m_memoryPool.NewArray<double*>(size);
        m_path[i] = m_memoryPool.NewArray< std::list<int>* >(size);
        for (int j = 0; j < size; ++j)
        {
            m_state[i][j] = m_memoryPool.NewArray<bool>(DirectionType_Size);
            m_cost[i][j] = m_memoryPool.NewArray<double>(DirectionType_Size);
            m_path[i][j] = m_memoryPool.NewArray< std::list<int> >(DirectionType_Size);
            for(int k = 0; k < DirectionType_Size; ++k)
            {
                m_state[i][j][k] = false;
                m_cost[i][j][k] = -1;
            }
        }
    }
    
    for (auto iteFrom = Scenario::Crosses().begin(); iteFrom != Scenario::Crosses().end(); ++iteFrom)
    {
        for (auto iteTo = Scenario::Crosses().begin(); iteTo != Scenario::Crosses().end(); ++iteTo)
        {
            if (iteFrom->first != iteTo->first)
            {
                std::set<int> bans;
                std::list<int> path;
                UpdatePathAndCost(iteFrom->first, iteTo->first, bans, path);
            }
        }
    }
}

int SchedulerXavier::UpdatePathAndCost(int current, int target, std::set<int>& bans, std::list<int>& list)
{
    if (current == target)
        return 0;
    Cross* cross = Scenario::GetCross(current);
    ASSERT(cross != 0);
    int bestCost = -1;
    DirectionType_Foreach(dir,
        Road* road = cross->GetRoad(dir);
        if (road == 0)
            continue;
        if (!road->GetIsTwoWay() && road->GetStartCross() != cross)
            continue;
        Cross* peer = road->GetPeerCross(cross);
        ASSERT(peer != 0);
        if (bans.find(peer->GetId()) != bans.end())
            continue;
        int cost = -1;
        std::list<int>& storage = m_path[_c(cross->GetId())][_c(target)][_d(dir)];
        if (m_state[_c(cross->GetId())][_c(target)][_d(dir)]) //already handled
        {
            cost = m_cost[_c(cross->GetId())][_c(target)][_d(dir)];
        }
        else
        {
            bans.insert(current);
            cost = UpdatePathAndCost(peer->GetId(), target, bans, storage);
            if (cost >= 0)
            {
                storage.push_back(peer->GetId());
                cost += road->GetLength();
            }
            std::cout << cross->GetId() << " to " << target << " dir " << dir << " : " << cost << std::endl;
            if(cost >= 0)
            {
                for(auto pathIte = storage.begin(); pathIte != storage.end(); ++pathIte)
                    std::cout << *pathIte << " ";
                std::cout << std::endl;
            }
            m_cost[_c(cross->GetId())][_c(target)][_d(dir)] = cost;
            m_state[_c(cross->GetId())][_c(target)][_d(dir)] = true;
            bans.erase(current);
        }
        if (cost >= 0 && (bestCost < 0 || cost < bestCost))
        {
            bestCost = cost;
            list = storage;
        }
    );
    return bestCost;
}
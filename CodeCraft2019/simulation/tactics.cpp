#include "tactics.h"
#include "scenario.h"

Tactics Tactics::Instance;

Tactics::Tactics()
{ }

void Tactics::Initialize()
{
    m_traces.clear();
    m_realTimes.clear();
    m_traces.resize(Scenario::Cars().size());
    m_realTimes.resize(Scenario::Cars().size(), -1);
}

std::vector<Trace>& Tactics::GetTraces()
{
    return m_traces;
}

std::vector<int>& Tactics::GetRealTimes()
{
    return m_realTimes;
}
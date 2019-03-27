#include "tactics.h"

Tactics Tactics::Instance;

Tactics::Tactics()
{ }

void Tactics::Initialize()
{
    m_traces.clear();
    m_realTimes.clear();
}

std::map<int, Trace>& Tactics::GetTraces()
{
    return m_traces;
}

std::map<int, int>& Tactics::GetRealTimes()
{
    return m_realTimes;
}
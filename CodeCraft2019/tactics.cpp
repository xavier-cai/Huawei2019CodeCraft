#include "tactics.h"

Tactics Tactics::Instance;

Tactics::Tactics()
{ }

std::map<int, Trace>& Tactics::GetTraces()
{
    return m_traces;
}
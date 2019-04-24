#ifndef TACTICS_H
#define TACTICS_H

#include "trace.h"
#include <vector>

class Tactics
{
private:
    Tactics();

    std::vector<Trace> m_traces;
    std::vector<int> m_realTimes;

public:
    static Tactics Instance; //singlton

    void Initialize();
    std::vector<Trace>& GetTraces();
    std::vector<int>& GetRealTimes();

};//class Tactic

#endif
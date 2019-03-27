#ifndef TACTICS_H
#define TACTICS_H

#include "trace.h"
#include <map>

class Tactics
{
private:
    Tactics();

    std::map<int, Trace> m_traces;
    std::map<int, int> m_realTimes;

public:
    static Tactics Instance; //singlton

    void Initialize();
    std::map<int, Trace>& GetTraces();
    std::map<int, int>& GetRealTimes();

};//class Tactic

#endif
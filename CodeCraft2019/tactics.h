#ifndef TACTICS_H
#define TACTICS_H

#include "trace.h"
#include <map>

class Tactics
{
private:
    Tactics();

    std::map<int, Trace> m_traces;

public:
    static Tactics Instance; //singlton

    std::map<int, Trace>& GetTraces();

};//class Tactic

#endif
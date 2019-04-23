#ifndef LOAD_STATE_H
#define LOAD_STATE_H

#include "sim-scenario.h"
#include <list>
#include "pmap.h"

class LoadState
{
public:
    struct TimeSlice
    {
        TimeSlice();
        TimeSlice(const int& time, const int& load);
        int Time;
        int Load;
    };//struct TimeSlice

    void Update(const int& time, SimScenario& scenario); //can not drop back or jump skip any time
    void Print(SimScenario& scenario) const;

private:
    std::list<TimeSlice> m_timeLoad;
    std::pmap<int, int, bool> m_spaceLoad; //road id & is opposite -> load state

};//class LoadState

#endif
#ifndef SIMULATOR_IMPL_H
#define SIMULATOR_IMPL_H

#include "simulator.h"

class SimulatorImpl
{
public:
    static Simulator::UpdateResult UpdateSelf(const int& time, SimScenario& scenario);

};//class SimulatorImpl

#endif
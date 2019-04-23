#ifndef SIM_SCENARIO_TESTER_H
#define SIM_SCENARIO_TESTER_H

#include "sim-scenario.h"

class SimScenariotTester : public SimScenario
{
public:
    SimScenariotTester();

    static double GeneProb, BlockProb, WaitingProb;

};//class SimScenariotTester

#endif
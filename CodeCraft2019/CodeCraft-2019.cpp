#include "config.h"
#include "scenario.h"
#include "log.h"

#include "assert.h"
#include "timer.h"
#include "simulator.h"
#include "sim-scenario.h"

#include <list>
#include <set>
#include <map>
#include "random.h"

#include "sim-scenario-tester.h"
#include "scheduler-floyd.h"
#include "scheduler-answer.h"

#include "simulator-impl.h"

class Program
{
private:
    int RunImpl(int argc, char *argv[], Scheduler* scheduler, bool save)
    {
        Config::Initialize(argc, argv);
        Scenario::Initialize();
        Simulator::Initialize();

	    SimScenario scenario;
        Simulator::SchedulerPtr = scheduler;

        scheduler->Initialize(scenario);
        int time = 0;
	    for(; true; time++) //forever until complete!
	    {
            scheduler->Update(time, scenario);
	        auto result = SimulatorImpl::UpdateSelf(time, scenario);
            scheduler->HandleResult(time, result);
	        if (result.Conflict)
                return -1;
	        if (scenario.IsComplete())
	            break;
	    }
        if(save)
            scenario.SaveToFile();
        LOG("Program execute time : " << Timer::GetSpendTime() << "s");
	    return time;
    }

public:
    int Run(int argc, char *argv[])
    {
        //Log::Default(Log::ENABLE);
        Log::Enable<Program>();
        Random::SetSeedAuto();
        //Random::SetSeed(0);
        LOG("Random seed : " << Random::GetSeed());

        //char* set1[] = { "", "./config/car.txt", "./config/road.txt", "./config/cross.txt", "./config/answer.txt" };
        char* set1[] = { "", "./config-1/car.txt", "./config-1/road.txt", "./config-1/cross.txt", "./config-1/answer.txt" };
        char* set2[] = { "", "./config-2/car.txt", "./config-2/road.txt", "./config-2/cross.txt", "./config-2/answer.txt" };
        argv = set1;
        if (true)
        {
            int bestTime = -1;
            int bestArg1, bestArg2;
            for (int arg1 = 10; arg1 <= 300; arg1 += 10)
            {
                for (int arg2 = 15; arg2 <= 30; arg2 += 1)
                {
                    //if (arg1 != 130 || arg2 != 22)
                    //    continue;
                    SchedulerFloyd::lenthWeight = arg1 / 100.0;
                    SchedulerFloyd::carLimit = arg2 / 10.0;
                    bool success = true;
                    int total = 0;
                    for (int i = 1; i <= 2; i++)
                    {
                        SchedulerFloyd scheduler;
                        //SchedulerAnswer scheduler;
                        int cost = RunImpl(5, i == 1 ? set1 : set2, &scheduler, true);
                        if (cost <= 0)
                            success = false;
                        else
                            total += cost;
                        LOG(arg1 << " " << arg2 << " : " << cost);
                    }
                    if (success)
                    {
                        if (bestTime < 0 || total < bestTime)
                        {
                            bestTime = total;
                            bestArg1 = arg1;
                            bestArg2 = arg2;
                        }
                    }
                }
            }
            LOG("best : " << bestArg1 << " " << bestArg2 << " : " << bestTime);
        }
        else
        {
            SchedulerFloyd::lenthWeight = int(10) / 100.0;
            //SchedulerFloyd::lanesWeight = int(20) / 100.0;
            SchedulerFloyd::carLimit = int(25) / 10.0;
            SchedulerFloyd scheduler;
            int ret = RunImpl(argc, argv, &scheduler, false);
            LOG(ret);
        }

        //SchedulerAnswer schedulerAnswer;
        //ASSERT(RunImpl(argc, argv, &schedulerAnswer, false) >= 0);
        return 0;
    }

    int Test(int argc, char* argv[])
    {
        Log::Default(Log::ENABLE);
        Log::Enable<Program>();
        Random::SetSeedAuto();
        //Random::SetSeed(0);
        LOG("Random seed : " << Random::GetSeed());
        char* argset[] = { "", "./config-2/car.txt", "./config-2/road.txt", "./config-2/cross.txt", "./config-2/answer.txt" };
        Config::Initialize(5, argset);
        Scenario::Initialize();
        Simulator::Initialize();
        std::cout << "input [Generator Prob], [Block Prob], [Waiting Prob] (format input%, -1 means default & end)" << std::endl;
        double input = -1;
        double* sink[3] = { &SimScenariotTester::GeneProb, &SimScenariotTester::BlockProb, &SimScenariotTester::WaitingProb };
        for (int i = 0; i < 3; i++)
        {
            std::cin >> input;
            if(input < 0)
                break;
            *(sink[i]) = input / 100.0;
        }
        LOG("Generator Prob : " << SimScenariotTester::GeneProb);
        LOG("Block Prob : " << SimScenariotTester::BlockProb);
        LOG("Waiting Prob : " << SimScenariotTester::WaitingProb);
        SimScenariotTester tester;
        int time = 2;
        LOG("Simulation start from " << time);
        std::cout << "Close this after 1 second plz" << std::endl;
	    for(; true; time++) //forever until complete!
	    {
	        auto result = SimulatorImpl::UpdateSelf(time, tester);
	        if (result.Conflict)
                return -1;
	        if (tester.IsComplete())
	            break;
	    }
        LOG("Program execute time : " << Timer::GetSpendTime() << "s");
        return 0;
    }

};//class Program

int main(int argc, char *argv[])
{
    Program program;
    return program.Run(argc, argv);
    //return program.Test(argc, argv);
}
;
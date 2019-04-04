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
#include "score-calculator.h"

#include "sim-scenario-tester.h"
#include "scheduler-floyd.h"
#include "scheduler-answer.h"

class Program
{
private:
    int RunImpl(int argc, char *argv[], Scheduler* scheduler, bool save)
    {
        //Random::SetSeedAuto();
        Random::SetSeed(0);
        LOG("Random seed : " << Random::GetSeed());

        Config::Initialize(argc, argv);
        Scenario::Initialize();

	    SimScenario scenario;
        Simulator::Instance.SetScheduler(scheduler);

        scheduler->Initialize(scenario);
        int time = 0;
	    for(; true; ++time) //forever until complete!
	    {
            scheduler->Update(time, scenario);
            auto result = Simulator::Instance.Update(time, scenario);
            int oldTime = time;
            scheduler->HandleResult(time, scenario, result);
            if (time != oldTime)
                LOG ("time back to " << time << " from " << oldTime << " " << Timer::GetSpendTime());
	        if (result.Conflict)
                return -1;
	        if (scenario.IsComplete())
	            break;
	    }
        LOG("Program execute time : " << Timer::GetSpendTime() << "s");
        if(save)
            scenario.SaveToFile();
        
        /* calculate score */
        if (time >= 0 && Scenario::GetVipCarsN() > 0)
        {
            time = ScoreCalculator::Calculate(scenario).Score;
        }
	    return time;
    }

public:
    int Run(int argc, char *argv[])
    {
        Random rngJitter;
        rngJitter.SetSeedAutoImpl();

        Log::Default(Log::ENABLE);
        Log::Enable<Program>();
        //Log::Enable<DeadLockSolver>();
        //Log::Enable<Simulator>();
        Log::Disable<LoadState>();

        //char* set1[] = { "", "./config/car.txt", "./config/road.txt", "./config/cross.txt", "./config/answer.txt" };
        //char* set1[] = { "", "./config2-check-1/car.txt", "./config2-check-1/road.txt", "./config2-check-1/cross.txt", "./config2-check-1/presetAnswer.txt", "./config2-check-1/answer-my.txt" };
        //char* set2[] = { "", "./config2-check-2/car.txt", "./config2-check-2/road.txt", "./config2-check-2/cross.txt", "./config2-check-2/presetAnswer.txt", "./config2-check-2/answer-my.txt" };
        char* set1[] = { "", "./config2-1/car.txt", "./config2-1/road.txt", "./config2-1/cross.txt", "./config2-1/presetAnswer.txt", "./config2-1/answer.txt" };
        char* set2[] = { "", "./config2-2/car.txt", "./config2-2/road.txt", "./config2-2/cross.txt", "./config2-2/presetAnswer.txt", "./config2-2/answer-laziji-147.txt" };
        argv = set1;
        if (false)
        {
            int bestTime = -1;
            int bestArg1 = -1, bestArg2 = -1;
            for (int iArg1 = 140; iArg1 <= 230; iArg1 += 10)
            {
                for (int iArg2 = 270; iArg2 <= 320; iArg2 += 10)
                {
                    for (int sample = 0; sample < 1; sample++)
                    {
                        //if (iArg1 != 140 || iArg2 != 270)
                        //    continue;
                        int arg1 = iArg1;
                        int arg2 = iArg2;
                        if(sample != 0)
                        {
                            arg1 += rngJitter.NextUniform(0, 5) - 2;
                            arg2 += rngJitter.NextUniform(0, 5) - 2;
                        }
                        SchedulerFloyd::lenthWeight = arg1 / 100.0;
                        SchedulerFloyd::carLimit = arg2 / 100.0;
                        bool success = true;
                        int total = 0;
                        for (int i = 2; i <= 2; ++i)
                        {
                            SchedulerFloyd scheduler;
                            //SchedulerAnswer scheduler;
                            int cost = RunImpl(6, i == 1 ? set1 : set2, &scheduler, true);
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
            }
            LOG("best : " << bestArg1 << " " << bestArg2 << " : " << bestTime);
        }
        else
        {
            for (int i = 2; i <= 2; ++i)
            {
                //SchedulerFloyd::lenthWeight = int(6) / 10.0;
                //SchedulerFloyd::lanesWeight = int(20) / 100.0;
                //SchedulerFloyd::carLimit = int(28) / 10.0;
                //SchedulerFloyd scheduler;
                SchedulerAnswer scheduler;
                int ret = RunImpl(6, i == 1 ? set1 : set2, &scheduler, false);
                LOG(ret);
            }
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
        std::cout << "input [Generator Prob], [Block Prob], [Waiting Prob] (format input%, -1 means default & end)" << std::endl;
        double input = -1;
        double* sink[3] = { &SimScenariotTester::GeneProb, &SimScenariotTester::BlockProb, &SimScenariotTester::WaitingProb };
        for (int i = 0; i < 3; ++i)
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
	    for(; true; ++time) //forever until complete!
	    {
            auto result = Simulator::Instance.Update(time, tester);
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

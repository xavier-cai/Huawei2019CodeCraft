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
#include "map-generator.h"

#include "sim-scenario-tester.h"
#include "scheduler-floyd.h"
#include "scheduler-answer.h"
#include "scheduler-time-weight.h"

#include "run-framework.h"

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
            Simulator::UpdateResult result;
            if (false)
            {
                SimScenario copy(scenario);
                result = Simulator::Instance.Update(time, copy);
                scenario = copy;
            }
            else
            {
                result = Simulator::Instance.Update(time, scenario);
            }
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
        //Log::Default(Log::ENABLE);
        Log::Enable<Scenario>();
        Log::Enable<SchedulerAnswer>();
        Log::Enable<Program>();
        Log::Enable<ScoreCalculator>();
        Log::Enable<DeadLockSolver>();
        Log::Enable<Simulator>();
        Log::Enable<Timer>();
        Log::Enable<SchedulerTimeWeight>();
        //Log::Disable<LoadState>();

        char* set1[] = { "", "./config2-1/car.txt", "./config2-1/road.txt", "./config2-1/cross.txt", "./config2-1/presetAnswer.txt", "./config2-1/answer.txt" };
        char* set2[] = { "", "./config2-2/car.txt", "./config2-2/road.txt", "./config2-2/cross.txt", "./config2-2/presetAnswer.txt", "./config2-2/answer.txt" };
        char* set3[] = { "", "./config3-1/car.txt", "./config3-1/road.txt", "./config3-1/cross.txt", "./config3-1/presetAnswer.txt", "./config3-1/answer.txt" };
        char* set4[] = { "", "./config3-2/car.txt", "./config3-2/road.txt", "./config3-2/cross.txt", "./config3-2/presetAnswer.txt", "./config3-2/answer.txt" };
        char* set5[] = { "", "./config3-exam-1/car.txt", "./config3-exam-1/road.txt", "./config3-exam-1/cross.txt", "./config3-exam-1/presetAnswer.txt", "./config3-exam-1/answer.txt" };
        char* set6[] = { "", "./config3-exam-2/car.txt", "./config3-exam-2/road.txt", "./config3-exam-2/cross.txt", "./config3-exam-2/presetAnswer.txt", "./config3-exam-2/answer.txt" };
        
        argc = 6;
        argv = set1;

        //Log::Enable<RunFramework>();
        //RunFramework framework;
        //framework.Run(argc, argv);

        SchedulerFloyd scheduler;
        scheduler.SetLengthWeight(1.6);
        scheduler.SetIsDropBackByDijkstra(false);
        scheduler.SetIsEnableVipWeight(true);
        scheduler.SetIsFasterAtEndStep(true);
        scheduler.SetIsLessCarAfterDeadLock(false);
        scheduler.SetIsLimitedByRoadSizeCount(true);
        scheduler.SetIsOptimalForLastVipCar(true);
        scheduler.SetIsVipCarDispatchFree(false);
        scheduler.SetPresetVipTracePreloadWeight(0.3);

        //SchedulerTimeWeight scheduler;
        //SchedulerAnswer scheduler;

        scheduler.EnableTrace("log.tr");
        int ret = RunImpl(argc, argv, &scheduler, true);
        LOG(ret);

        Timer::Print();
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

    int Generate(int argc, char* argv[])
    {
        Log::Enable<MapGenerator>();
        Log::Enable<Program>();
        Random::SetSeedAuto();
        LOG("random seed : " << Random::GetSeed());
        MapGenerator generator;
        generator.SetCarsN(100000);
        generator.SetWidth(13);
        generator.SetHeight(13);
        generator.SetNoWayProbability(0.01);
        generator.SetSingleWayProbability(0.03);
        generator.SetVipProbability(0.1);
        generator.SetPresetProbability(0.15);
        generator.SetPresetMaxRealTime(800);
        char* args[] = { "", "./config2-gene-1/car.txt", "./config2-gene-1/road.txt", "./config2-gene-1/cross.txt", "./config2-gene-1/presetAnswer.txt", "./config2-gene-1/answer.txt" };
        generator.Generate(6, args);
        return 0;
    }

};//class Program

int main(int argc, char *argv[])
{
    Program program;
    return program.Run(argc, argv);
    //return program.Test(argc, argv);
    //return program.Generate(argc, argv);
}

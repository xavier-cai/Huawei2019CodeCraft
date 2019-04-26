#include "run-framework.h"
#include "log.h"
#include "timer.h"
#include "random.h"
#include "config.h"
#include "scenario.h"
#include "simulator.h"
#include "sim-scenario.h"

RunFramework::RunFramework()
    : m_maxTime(900), m_safeInterval(100), m_bestAnswer(-1)
    , m_isStableOutputed(false)
{ }

bool RunFramework::IsNoMoreTime() const
{
    return Timer::GetLeftTime(m_maxTime) < m_safeInterval;
}

bool RunFramework::HandleTerminateAssert()
{
    LOG("assert detected programing running time : " << Timer::GetSpendTime());
    return false;
}

void RunFramework::Run(int argc, char* argv[])
{
    Random::SetSeed(0);
    LOG("Random seed : " << Random::GetSeed());

    Config::Initialize(argc, argv);
    Scenario::Initialize();

    while (!IsNoMoreTime())
    {
        try
        {
            //first step : run the stable version
            if (!m_isStableOutputed)
                RunStableVersion();
            //second step : find a more good answer
            RunFindABetterAnswer();
        }
        catch(...)
        {
            //handle exception
            if (!HandleTerminateAssert())
                break;
        }
    }
}

#include "score-calculator.h"

void RunFramework::RunImpl(Scheduler* scheduler)
{
    ASSERT(false);
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
            return;
	    if (scenario.IsComplete())
	        break;
        if (IsNoMoreTime())
            return;
	}
    LOG("Program execute time : " << Timer::GetSpendTime() << "s");
    int answer = ScoreCalculator::Calculate(scenario).Score;
    if(m_bestAnswer < 0 || answer < m_bestAnswer)
    {
        scenario.SaveToFile();
        m_bestAnswer = answer;
    }
}

#include "scheduler-floyd.h"

void RunFramework::RunStableVersion()
{
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
    RunImpl(&scheduler);
    m_isStableOutputed = true;
}

void RunFramework::RunFindABetterAnswer()
{

}
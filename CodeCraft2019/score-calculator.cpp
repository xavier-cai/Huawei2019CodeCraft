#include "score-calculator.h"
#include "scenario.h"
#include <set>
#include "assert.h"
#include "log.h"

struct ScoreStatistic
{
    int FirstPlanTime, LastPlanTime;
    int MinSpeed, MaxSpeed;
    std::set<int> StartDistribution, EndDistribution;

    ScoreStatistic();
    void Update(const Car* car);
    bool IsValid() const;

};//struct ScoreStatistic

ScoreStatistic::ScoreStatistic()
    : FirstPlanTime(-1), LastPlanTime(-1)
    , MinSpeed(-1), MaxSpeed(-1)
{ }

void ScoreStatistic::Update(const Car* car)
{
    const int& planTime = car->GetPlanTime();
    const int& speed = car->GetMaxSpeed();
    if (FirstPlanTime < 0 || planTime < FirstPlanTime)
        FirstPlanTime = planTime;
    if (LastPlanTime < 0 || planTime > LastPlanTime)
        LastPlanTime = planTime;
    if (MinSpeed < 0 || speed < MinSpeed)
        MinSpeed = speed;
    if (MaxSpeed < 0 || speed > MaxSpeed)
        MaxSpeed = speed;
    StartDistribution.insert(car->GetFromCrossId());
    EndDistribution.insert(car->GetToCrossId());
}

bool ScoreStatistic::IsValid() const
{
    return FirstPlanTime >= 0 && LastPlanTime >= 0
        && MinSpeed > 0 && MaxSpeed > 0
        && StartDistribution.size() > 0 && EndDistribution.size() > 0;
}

ScoreCalculator::ScoreResult::ScoreResult()
    : Score(-1), Total(-1), Vip(false)
{ }

ScoreCalculator::ScoreCalculator()
{ }

ScoreCalculator ScoreCalculator::Instance;

ScoreCalculator::ScoreResult ScoreCalculator::DoCalculate(const SimScenario& scenario) const
{
    ScoreResult result;
    ScoreStatistic all, vip;
    for (auto ite = Scenario::Cars().begin(); ite != Scenario::Cars().end(); ++ite)
    {
        const Car* car = ite->second;
        if (car->GetIsVip())
        {
            result.Vip = true;
            vip.Update(car);
        }
        all.Update(car);
    }
    ASSERT(all.IsValid());
    ASSERT(!result.Vip || vip.IsValid());
    LOG("origin result : score " << scenario.GetScheduledTime()
        << " total " << scenario.GetTotalCompleteTime());
    LOG("vip result : score " << scenario.GetVipScheduledTime()
        << " total " << scenario.GetVipTotalCompleteTime());
    float factor = 0
        + (all.MaxSpeed * 1.0 / all.MinSpeed) / (vip.MaxSpeed * 1.0 / vip.MinSpeed)
        + (all.LastPlanTime * 1.0 / all.FirstPlanTime) / (vip.LastPlanTime * 1.0 / vip.FirstPlanTime)
        + all.StartDistribution.size() * 1.0 / vip.StartDistribution.size()
        + all.EndDistribution.size() * 1.0 / vip.EndDistribution.size();
    float factorA = Scenario::Cars().size() * 1.0 / Scenario::GetVipCarsN() * 0.05 + factor * 0.2375;
    float factorB = Scenario::Cars().size() * 1.0 / Scenario::GetVipCarsN() * 0.8 + factor * 0.05;
    result.Score = factorA * scenario.GetVipScheduledTime() + scenario.GetScheduledTime() + 0.5;
    result.Total = factorB * scenario.GetVipTotalCompleteTime() + scenario.GetTotalCompleteTime() + 0.5;
    LOG("final result : score " << result.Score
        << " total " << result.Total);
    return result;
}

ScoreCalculator::ScoreResult ScoreCalculator::Calculate(const SimScenario& scenario)
{
    return Instance.DoCalculate(scenario);
}
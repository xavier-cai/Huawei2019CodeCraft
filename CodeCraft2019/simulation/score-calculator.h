#ifndef SCORE_CALCULATOR_H
#define SCORE_CALCULATOR_H

#include "sim-scenario.h"

class ScoreCalculator
{
public:
    struct ScoreResult
    {
        int Score;
        int Total;
        bool Vip; //contain VIP car

        ScoreResult();

    };//struct Socre

private:
    ScoreCalculator();
    static ScoreCalculator Instance;

    ScoreResult DoCalculate(const SimScenario& scenario) const;

public:
    static ScoreResult Calculate(const SimScenario& scenario);

};//class ScoreCalculator

#endif
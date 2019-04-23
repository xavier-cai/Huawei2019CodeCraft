#ifndef SCHEDULER_TIMEWEIGHT_H
#define SCHEDULER_TIMEWEIGHT_H

#include "scheduler.h"
#include <list>
#include <vector>
#include "dead-lock-solver.h"

class SchedulerTimeWeight : public Scheduler
{
public:
    SchedulerTimeWeight();

protected:
    virtual void DoInitialize(SimScenario& scenario) override;
    virtual void DoUpdate(int& time, SimScenario& scenario) override;

private:
    /* factor array */
    static void InitilizeConfidence();
    //static std::vector< std::vector< std::vector<double> > > m_confidence; //confidence(deltaT, a, b) : binomial(N = a, k = b)
    static std::vector< std::vector< std::vector<double> > > m_collectionWeight; //interface for update w'(a, b, delataT( <= b))
    //static std::vector< std::pair<int, int> > m_significantRange; //deltaT -> start, length
    static int m_maxValidRange;

    int m_updateInterval;
    std::vector< std::vector<SimCar*> > m_carList;

    /* time weight */
    int m_carWeightStartTime;
    std::vector< std::vector< std::pair<double, double> > > m_carWeight;
    std::vector<int> m_roadWeight; //basic weight
    std::vector<int> m_roadCapacity; //capacity limit
    std::vector< std::pair<double, double> > m_threshold; //number of lane -> ignore threshold & ban threshold
    std::vector< std::vector<double> > m_expectedWeight; //decide if need change trace
    std::vector< std::vector< std::pair< std::list<int>, int> > > m_bestTrace; //start -> end : trace & length

    void InitilizeBestTraceByFloyd();
    bool IsAppropriateToDispatch(const int& time, SimCar* car) const;

    void UpdateTimeWeightByRoadAndTime(const int& time, const int& roadId, const bool& dir, int startTime, int leaveTime);
    void UpdateTimeWeightForEachCar(const int& time, SimCar* car);
    void UpdateCurrentWeightByScenario(const int& time, SimScenario& scenario);
    bool UpdateCarTraceByDijkstraWithTimeWeight(const int& time, SimScenario& scenario, const std::vector<int>& validFirstHop, SimCar* car) const;

};

#endif
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
    virtual void DoHandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result) override;
    virtual void DoHandleBecomeFirstPriority(const int& time, SimScenario& scenario, SimCar* car) override;

private:
    /* factor array */
    static void InitilizeConfidence();
    //static std::vector< std::vector< std::vector<double> > > m_confidence; //confidence(deltaT, a, b) : binomial(N = a, k = b)
    static std::vector< std::vector< std::vector<double> > > m_collectionWeight; //interface for update w'(a, b, delataT( <= b))
    //static std::vector< std::pair<int, int> > m_significantRange; //deltaT -> start, length
    static int m_maxValidRange;

    int m_updateInterval;
    std::vector< std::vector<SimCar*> > m_carList;
    DeadLockSolver m_deadLockSolver;
    std::pair<int, bool> SelectBestRoad(SimScenario& scenario, const std::vector<int>& list, SimCar* car);

    /* time weight */
    int m_carWeightStartTime;
    std::vector< std::vector< std::pair<double, double> > > m_carWeight;
    std::vector<int> m_roadWeight; //basic weight
    std::vector<int> m_roadCapacity; //capacity limit
    std::vector< std::pair<double, double> > m_threshold; //number of lane -> ignore threshold & ban threshold
    std::vector< std::vector<double> > m_expectedWeight; //decide if need change trace
    std::vector< std::vector< std::pair< std::list<int>, int> > > m_bestTrace; //start -> end : trace & length

    void InitializeBestTraceByFloyd();
    void InitializeCarTraceByBeastTrace(SimScenario& scenario);
    void InitializeCarTraceByDijkstra(SimScenario& scenario);
    bool IsAppropriateToDispatch(const int& time, SimCar* car) const;

    void UpdateTimeWeightByRoadAndTime(const int& time, const int& roadId, const bool& dir, int startTime, int leaveTime, const bool& isDecrease);
    void UpdateTimeWeightForEachCar(const int& time, SimCar* car, const bool& isDecrease = false);
    void UpdateCurrentWeightByScenario(const int& time, SimScenario& scenario);
    bool UpdateCarTraceByDijkstraWithTimeWeight(const int& time, SimScenario& scenario, SimCar* car, const std::vector<int>& banedFirstHop = std::vector<int>()) const;

};

#endif
#ifndef SCHEDULER_FLOYD_H
#define SCHEDULER_FLOYD_H

#include "scheduler.h"
#include <list>
#include "dead-lock-solver.h"

class SchedulerFloyd : public Scheduler
{
public:
    SchedulerFloyd();

    void SetLengthWeight(double v);
    void SetCarNumWeight(double v);
    void SetLanesNumWeight(double v);
    void SetCarLimit(double v);
    void SetPresetVipTracePreloadWeight(double v);
    void SetLastPresetVipCarEstimateArriveTime(int v);
    void SetIsEnableVipWeight(bool v);
    void SetIsOptimalForLastVipCar(bool v);
    void SetIsLimitedByRoadSizeCount(bool v);
    void SetIsFasterAtEndStep(bool v);
    void SetIsLessCarAfterDeadLock(bool v);
    void SetIsDropBackByDijkstra(bool v);
    void SetIsVipCarDispatchFree(bool v);
    void SetVipCarOptimalStartTime(int v);
    void HandleSimCarScheduled(const SimCar* car);

protected:
    virtual void DoInitialize(SimScenario& scenario) override;
    virtual void DoUpdate(int& time, SimScenario& scenario) override;
    virtual void DoHandleGetoutGarage(const int& time, SimScenario& scenario, SimCar* car) override;
    virtual void DoHandleBecomeFirstPriority(const int& time, SimScenario& scenario, SimCar* car) override;
    virtual void DoHandleResult(int& time, SimScenario& scenario, Simulator::UpdateResult& result) override;
    void HandlePresetCars(SimScenario& scenario);

private:
    /* algorithm floyde */
    std::vector< std::vector<double> > m_weightCrossToCross;
    std::vector< std::vector<int> > m_connectionCrossToCross;
    std::vector< std::vector< std::vector<int> > > m_minPathCrossToCross;

    /* appointment */
    std::vector< std::pair<int, int> > m_appointOnRoadCounter;
    
    /* for dispatching cars */
    std::set<int> m_notArrivedProtectedCars;
    //std::vector< std::pair<int, int> > aver;
    std::vector< std::pair<int, int> > m_garageMinSpeed;
    std::vector< std::pair<int, int> > m_garageTraceSizeLimit;
    std::vector<int> m_garagePlanCarNum;

    /* for solving dead lock */
    DeadLockSolver m_deadLockSolver;
    void HandleGoOnNewRoad(const SimCar* car, Road* oldRoad);
    std::pair<int, bool> SelectBestRoad(SimScenario& scenario, const std::vector<int>& list, SimCar* car);

    /* private interfaces */
    void CalculateWeight(SimScenario& scenario);
    void RefreshNotArrivedPresetCars(SimScenario& scenario);
    bool UpdateCarTraceByDijkstra(const int& time, const SimScenario& scenario, SimCar* car) const;
    bool UpdateCarTraceByDijkstra(const int& time, const SimScenario& scenario, const std::vector<int>& validFirstHop, SimCar* car) const;

    int m_updateInterval;

    /* weight */
    double m_lengthWeight;
    double m_carNumWeight;
    double m_lanesNumWeight;

    /* dispatch limit */
    double m_carLimit;
    double m_carLimitLooser;
    double m_carLimitTighter;
    double m_presetVipTracePreloadWeight;
    int m_lastVipCarRealTime;
    int m_lastPresetVipCarEstimateArriveTime;
    int m_vipCarOptimalStartTime;
    int m_vipCarTraceProtectedStartTime;

    /* switchers */
    bool m_isEnableVipWeight;
    bool m_isOptimalForLastVipCar;
    bool m_isLimitedByRoadSizeCount;
    bool m_isFasterAtEndStep;
    bool m_isLessCarAfterDeadLock;
    bool m_isDropBackByDijkstra;
    bool m_isVipCarDispatchFree;

    /* temporary variables */
    double m_roadCapacityAverage;
    int m_carsNumOnRoadLimit;

};//class SchedulerFloyd

#endif
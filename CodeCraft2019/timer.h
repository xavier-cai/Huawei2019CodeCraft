#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <map>
#include <string>

class TimerHandle
{
public:
    double GetSpendTime() const;

private:
    TimerHandle(const clock_t& record); //only Timer can create an instance
    friend class Timer;
    clock_t m_record;

};//class TimerHandle

class Timer
{
private:
    static Timer Instance;
    Timer();

    clock_t m_start;
    double m_max;

    double DoGetSpendTime(const clock_t& t) const;
    friend class TimerHandle;

    const TimerHandle DoRecord() const;
    double DoGetSpendTime() const;
    double DoGetLeftTime(const double& max) const;

    /* for calculating algorithm time cost */
    struct CostStatistic
    {
        CostStatistic();
        int UpdateCount;
        double UpdateAverage;
        double UpdateMax;
        double UpdateMin;
        TimerHandle Handle;
    };//struct CostStatistic
    std::map<std::string, CostStatistic> m_statistic;
    void DoUpdateTimeCostBegin(const std::string& id);
    void DoUpdateTimeCostEnd(const std::string& id);
    void DoPrint() const;

public:
    static const TimerHandle Record();
    static double GetSpendTime();
    static double GetLeftTime(const double& max);

     /* for calculating algorithm time cost */
    static void UpdateTimeCostBegin(const std::string& id);
    static void UpdateTimeCostEnd(const std::string& id);
    static void Print();

};//class Timer

#endif

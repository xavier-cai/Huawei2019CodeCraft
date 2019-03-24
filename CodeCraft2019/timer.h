#ifndef TIMER_H
#define TIMER_H

#include <time.h>

class TimerHandle
{
public:
    double GetSpendTime() const;

private:
    TimerHandle(clock_t record); //only Timer can create an instance
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

    double DoGetSpendTime(clock_t t) const;
    friend class TimerHandle;

    const TimerHandle DoRecord() const;
    double DoGetSpendTime() const;
    double DoGetLeftTime(double max) const;

public:
    static const TimerHandle Record();
    static double GetSpendTime();
    static double GetLeftTime(double max);

};//class Timer

#endif
